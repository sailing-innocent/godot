#include "battle_engine.h"

#include "../action/action_result.h"
#include "../action/action_validator.h"
#include "../action/battle_action.h"
#include "../action/resolvers/end_turn_resolver.h"
#include "../action/resolvers/move_resolver.h"
#include "../action/resolvers/skill_resolver.h"
#include "../action/resolvers/wait_resolver.h"
#include "../components/movement_component.h"
#include "../components/skill_component.h"
#include "../components/stats_component.h"
#include "../components/transform_component.h"
#include "../components/turn_component.h"
#include "../event/game_event.h"
#include "../skill/skill_def.h"
#include "../state/rule_set.h"
#include "../query/query_api.h"
#include "../state/battle_state.h"
#include "../state/combat_entity.h"
#include "../serialization/deterministic_random.h"
#include "modules/hat_factory_hex_grid/hex_cell_data.h"
#include "modules/hat_factory_hex_grid/hex_grid_map_data.h"
#include "battle_config.h"

#include "core/object/class_db.h"

void BattleEngine::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_initial_state", "config"), &BattleEngine::create_initial_state);
	ClassDB::bind_method(D_METHOD("submit", "state", "action"), &BattleEngine::submit);
	ClassDB::bind_method(D_METHOD("get_legal_actions", "state", "actor_id"), &BattleEngine::get_legal_actions);
	ClassDB::bind_method(D_METHOD("check_win_condition", "state"), &BattleEngine::check_win_condition);
	ClassDB::bind_method(D_METHOD("start_turn", "state"), &BattleEngine::start_turn);
	ClassDB::bind_method(D_METHOD("query", "state"), &BattleEngine::query);

	ADD_SIGNAL(MethodInfo("action_resolved", PropertyInfo(Variant::OBJECT, "result", PROPERTY_HINT_RESOURCE_TYPE, "ActionResult")));
	ADD_SIGNAL(MethodInfo("phase_changed", PropertyInfo(Variant::STRING_NAME, "phase_name")));
}

Ref<BattleState> BattleEngine::create_initial_state(const Ref<BattleConfig> &p_config) const {
	Ref<BattleState> state;
	state.instantiate();
	if (p_config.is_null()) {
		return state;
	}
	state->set_session_id(p_config->get_session_id());
	state->set_random_seed(p_config->get_random_seed());
	state->set_grid_data(p_config->get_grid_data());
	state->set_terrain_library(p_config->get_terrain_library());
	state->set_rules(p_config->get_rules());
	state->set_round(1);
	state->set_phase(StringName("init"));
	state->set_turn_index(0);
	state->set_current_actor(0);

	Ref<DeterministicRandom> rng;
	rng.instantiate();
	rng->set_seed(p_config->get_random_seed());
	state->set_rng(rng);

	Array entities_data = p_config->get_entities_data();
	TypedArray<int> order;
	for (int i = 0; i < entities_data.size(); i++) {
		Dictionary edict = entities_data[i];
		Ref<CombatEntity> e = CombatEntity::from_dict(edict);
		if (e.is_valid()) {
			state->set_entity(e->get_entity_id(), e);
			order.push_back(e->get_entity_id());
		}
	}
	state->set_turn_order(order);
	return state;
}

Ref<ActionResult> BattleEngine::submit(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const {
	ActionValidator validator;
	Ref<ActionResult> validation = validator.validate(p_state, p_action);
	if (!validation->get_accepted()) {
		const_cast<BattleEngine *>(this)->emit_signal(SNAME("action_resolved"), validation);
		return validation;
	}

	Ref<ActionResult> result;
	int type = p_action->get_type();
	if (type == BattleAction::MOVE) {
		MoveResolver resolver;
		result = resolver.resolve(p_state, p_action);
	} else if (type == BattleAction::SKILL) {
		SkillResolver resolver;
		result = resolver.resolve(p_state, p_action);
	} else if (type == BattleAction::WAIT) {
		WaitResolver resolver;
		result = resolver.resolve(p_state, p_action);
	} else if (type == BattleAction::END_TURN) {
		EndTurnResolver resolver;
		result = resolver.resolve(p_state, p_action);
	} else if (type == BattleAction::ITEM) {
		// The item resolver lives in the hat_factory_rpg module so that the
		// combat module has no link-time dependency on RPG types.
		if (ClassDB::class_exists("ItemActionResolver")) {
			Object *obj = ClassDB::instantiate("ItemActionResolver");
			RefCounted *rc = Object::cast_to<RefCounted>(obj);
			if (rc != nullptr) {
				Ref<RefCounted> resolver;
				resolver.reference_ptr(rc);
				Variant ret = resolver->call("resolve", p_state, p_action);
				result = ret;
			}
		}
		if (result.is_null()) {
			result.instantiate();
			result->set_action(p_action);
			result->set_next_state(p_state);
			result->set_reason("Item resolver unavailable");
		}
	} else {
		result.instantiate();
		result->set_action(p_action);
		result->set_next_state(p_state);
		result->set_reason("Action type not implemented");
	}

	const_cast<BattleEngine *>(this)->emit_signal(SNAME("action_resolved"), result);
	return result;
}

TypedArray<BattleAction> BattleEngine::get_legal_actions(const Ref<BattleState> &p_state, int p_actor_id) const {
	TypedArray<BattleAction> actions;
	if (p_state.is_null()) {
		return actions;
	}
	CombatEntity *entity = p_state->get_entity(p_actor_id);
	if (entity == nullptr) {
		return actions;
	}

	bool moved = false;
	bool acted = false;
	Ref<TurnComponent> turn = entity->get_component(StringName("Turn"));
	if (turn.is_valid()) {
		moved = turn->get_moved();
		acted = turn->get_acted();
	}

	Ref<QueryAPI> query;
	query.instantiate();
	query->set_state(p_state);

	Ref<TransformComponent> transform = entity->get_component(StringName("Transform"));

	// Move actions: one per reachable cell (excluding the current cell).
	if (!moved && transform.is_valid()) {
		int range = 0;
		if (entity->has_component(StringName("Movement"))) {
			Ref<MovementComponent> movement = entity->get_component(StringName("Movement"));
			if (movement.is_valid()) {
				range = movement->get_range();
			}
		}
		if (range > 0) {
			TypedArray<Vector2i> cells = query->get_reachable_cells(p_actor_id, range);
			for (int i = 0; i < cells.size(); i++) {
				Vector2i cell = cells[i];
				if (cell == transform->get_coord()) {
					continue;
				}
				TypedArray<Vector2i> path;
				path.push_back(transform->get_coord());
				path.push_back(cell);
				actions.push_back(BattleAction::move(p_actor_id, path));
			}
		}
	}

	// Skill actions: one per (known skill, valid target entity) pair.
	if (!acted && transform.is_valid() && entity->has_component(StringName("Skill"))) {
		Ref<SkillComponent> skills = entity->get_component(StringName("Skill"));
		Ref<RuleSet> rules = p_state->get_rules();
		if (skills.is_valid()) {
			TypedArray<StringName> skill_ids = skills->get_skill_ids();
			for (int i = 0; i < skill_ids.size(); i++) {
				StringName skill_id = skill_ids[i];
				Ref<SkillDef> def;
				if (rules.is_valid()) {
					def = rules->get_skill_def(skill_id);
				}
				if (def.is_null()) {
					continue;
				}
				int target_type = def->get_target_type();
				TypedArray<CombatEntity> ents = p_state->get_entities();
				for (int e = 0; e < ents.size(); e++) {
					Ref<CombatEntity> target = ents[e];
					if (target.is_null() || !target->has_component(StringName("Transform"))) {
						continue;
					}
					if (target_type == 0 && target->get_entity_id() != p_actor_id) {
						continue;
					}
					if (target_type == 1 && target->get_owner() != entity->get_owner()) {
						continue;
					}
					if (target_type == 2 && target->get_owner() == entity->get_owner()) {
						continue;
					}
					Ref<TransformComponent> tt = target->get_component(StringName("Transform"));
					int dist = QueryAPI::hex_distance(transform->get_coord(), tt->get_coord());
					if (dist > def->get_range() || dist < def->get_min_range()) {
						continue;
					}
					actions.push_back(BattleAction::skill(p_actor_id, skill_id, tt->get_coord(), target->get_entity_id()));
				}
			}
		}
	}

	actions.push_back(BattleAction::wait(p_actor_id));
	actions.push_back(BattleAction::end_turn(p_actor_id));
	return actions;
}

Ref<ActionResult> BattleEngine::check_win_condition(const Ref<BattleState> &p_state) const {
	Ref<ActionResult> result;
	result.instantiate();
	result->set_next_state(p_state);
	if (p_state.is_null()) {
		result->set_reason("Null state");
		return result;
	}

	HashMap<int, bool> owners;
	TypedArray<CombatEntity> ents = p_state->get_entities();
	for (int i = 0; i < ents.size(); i++) {
		Ref<CombatEntity> e = ents[i];
		if (e.is_valid()) {
			owners[e->get_owner()] = true;
		}
	}

	if (owners.size() <= 1) {
		StringName winner;
		if (owners.size() == 1) {
			for (KeyValue<int, bool> kv : owners) {
				winner = StringName(String::num_int64(kv.key));
			}
		}
		Ref<GameEvent> ev = GameEvent::battle_ended(winner);
		ev->set_round(p_state->get_round());
		Ref<BattleState> next = p_state->clone();
		next->append_event(ev);
		result->set_accepted(true);
		result->set_next_state(next);
		result->append_event(ev);
	} else {
		result->set_reason("Battle continues");
	}
	return result;
}

Ref<BattleState> BattleEngine::start_turn(const Ref<BattleState> &p_state) const {
	if (p_state.is_null()) {
		return p_state;
	}
	Ref<BattleState> next = p_state->clone();
	TypedArray<int> order = next->get_turn_order();
	int idx = next->get_turn_index();
	int actor_id = 0;
	if (order.size() > 0) {
		idx = idx % order.size();
		next->set_turn_index(idx);
		actor_id = order[idx];
	}
	next->set_current_actor(actor_id);
	next->set_phase(StringName("turn"));

	CombatEntity *entity = next->get_entity(actor_id);
	if (entity != nullptr) {
		Ref<TurnComponent> turn = entity->get_component(StringName("Turn"));
		if (turn.is_valid()) {
			turn->set_moved(false);
			turn->set_acted(false);
			turn->set_skipped(false);
		}
		Ref<StatsComponent> stats = entity->get_component(StringName("Stats"));
		if (stats.is_valid()) {
			Ref<RuleSet> rules = next->get_rules();
			if (rules.is_valid() && rules->get_use_terrain_energy_regen()) {
				// Terrain-based action energy regen (MVP R6): the current actor
				// regen energy according to the terrain/height of the cell it
				// stands on, clamped to max AP, instead of a full refill.
				StringName terrain_id;
				int height = 0;
				Ref<TransformComponent> transform = entity->get_component(StringName("Transform"));
				if (transform.is_valid()) {
					Ref<HexGridMapData> grid = next->get_grid_data();
					if (grid.is_valid()) {
						Ref<HexCellData> cell = grid->get_cell(transform->get_coord());
						if (cell.is_valid()) {
							terrain_id = cell->get_terrain_id();
							height = cell->get_height();
						}
					}
				}
				int regen = rules->get_energy_regen_for(terrain_id, height);
				stats->set_ap(CLAMP(stats->get_ap() + regen, 0, stats->get_max_ap()));
			} else {
				stats->set_ap(stats->get_max_ap());
			}
		}
	}

	Ref<GameEvent> ev = GameEvent::turn_started(actor_id);
	ev->set_round(next->get_round());
	next->get_event_log().push_back(ev);
	const_cast<BattleEngine *>(this)->emit_signal(SNAME("phase_changed"), StringName("turn"));
	return next;
}

Ref<QueryAPI> BattleEngine::query(const Ref<BattleState> &p_state) const {
	Ref<QueryAPI> q;
	q.instantiate();
	q->set_state(p_state);
	return q;
}
