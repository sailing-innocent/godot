#include "end_turn_resolver.h"

#include "../action_result.h"
#include "../battle_action.h"
#include "components/skill_component.h"
#include "components/stats_component.h"
#include "components/status_component.h"
#include "components/turn_component.h"
#include "event/game_event.h"
#include "state/battle_state.h"
#include "state/combat_entity.h"
#include "state/rule_set.h"
#include "components/transform_component.h"
#include "modules/hat_factory_hex_grid/hex_cell_data.h"
#include "modules/hat_factory_hex_grid/hex_grid_map_data.h"

#include "core/object/class_db.h"

void EndTurnResolver::_bind_methods() {
	ClassDB::bind_method(D_METHOD("resolve", "state", "action"), &EndTurnResolver::resolve);
}

Ref<ActionResult> EndTurnResolver::resolve(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const {
	Ref<ActionResult> result;
	result.instantiate();
	result->set_action(p_action);

	if (p_state.is_null() || p_action.is_null()) {
		result->set_reason("Null state or action");
		return result;
	}

	Ref<BattleState> next = p_state->clone();
	CombatEntity *entity = next->get_entity(p_action->get_actor_id());
	if (entity == nullptr) {
		result->set_reason("Actor does not exist");
		return result;
	}

	// Mark the ending actor as done and tick its status durations / cooldowns.
	Ref<TurnComponent> turn = entity->get_component(StringName("Turn"));
	if (turn.is_valid()) {
		turn->set_moved(true);
		turn->set_acted(true);
	}
	Ref<StatusComponent> status = entity->get_component(StringName("Status"));
	if (status.is_valid()) {
		TypedArray<StatusEffectInstance> before = status->get_effects();
		status->tick_turns();
		TypedArray<StatusEffectInstance> after = status->get_effects();
		for (int i = 0; i < before.size(); i++) {
			Ref<StatusEffectInstance> inst = before[i];
			if (inst.is_null()) {
				continue;
			}
			bool still_present = false;
			for (int j = 0; j < after.size(); j++) {
				Ref<StatusEffectInstance> other = after[j];
				if (other.is_valid() && other->get_effect_id() == inst->get_effect_id()) {
					still_present = true;
					break;
				}
			}
			if (!still_present) {
				Ref<GameEvent> ev;
				ev.instantiate();
				ev->set_type(GameEvent::STATUS_REMOVED);
				ev->set_target_id(p_action->get_actor_id());
				Dictionary params;
				params["status_id"] = inst->get_effect_id();
				ev->set_params(params);
				ev->set_round(next->get_round());
				next->append_event(ev);
				result->append_event(ev);
			}
		}
	}
	Ref<SkillComponent> skills = entity->get_component(StringName("Skill"));
	if (skills.is_valid()) {
		skills->tick_cooldowns();
	}

	// Emit TURN_ENDED for the ending actor.
	Ref<GameEvent> end_ev;
	end_ev.instantiate();
	end_ev->set_type(GameEvent::TURN_ENDED);
	end_ev->set_actor_id(p_action->get_actor_id());
	end_ev->set_round(next->get_round());
	next->append_event(end_ev);
	result->append_event(end_ev);

	// Advance to the next living actor in the turn order.
	TypedArray<int> order = next->get_turn_order();
	if (order.size() > 0) {
		int idx = next->get_turn_index();
		int next_idx = idx;
		bool wrapped = false;
		for (int step = 1; step <= order.size(); step++) {
			int candidate = (idx + step) % order.size();
			int candidate_id = order[candidate];
			if (next->get_entity(candidate_id) != nullptr) {
				next_idx = candidate;
				wrapped = candidate <= idx;
				break;
			}
		}
		if (wrapped) {
			next->set_round(next->get_round() + 1);
			Ref<GameEvent> round_ev;
			round_ev.instantiate();
			round_ev->set_type(GameEvent::ROUND_ENDED);
			round_ev->set_round(next->get_round());
			next->append_event(round_ev);
			result->append_event(round_ev);
		}
		next->set_turn_index(next_idx);
		int next_actor = order[next_idx];
		next->set_current_actor(next_actor);

		CombatEntity *next_entity = next->get_entity(next_actor);
		if (next_entity != nullptr) {
			Ref<TurnComponent> next_turn = next_entity->get_component(StringName("Turn"));
			if (next_turn.is_valid()) {
				next_turn->set_moved(false);
				next_turn->set_acted(false);
				next_turn->set_skipped(false);
			}
			// Use the same terrain energy rule as BattleEngine::start_turn().
			Ref<StatsComponent> next_stats = next_entity->get_component(StringName("Stats"));
			if (next_stats.is_valid()) {
				Ref<RuleSet> rules = next->get_rules();
				if (rules.is_valid() && rules->get_use_terrain_energy_regen()) {
					StringName terrain_id;
					int height = 0;
					Ref<TransformComponent> transform = next_entity->get_component(StringName("Transform"));
					Ref<HexGridMapData> grid = next->get_grid_data();
					if (transform.is_valid() && grid.is_valid()) {
						Ref<HexCellData> cell = grid->get_cell(transform->get_coord());
						if (cell.is_valid()) {
							terrain_id = cell->get_terrain_id();
							height = cell->get_height();
						}
					}
					int regen = rules->get_energy_regen_for(terrain_id, height);
					next_stats->set_ap(CLAMP(next_stats->get_ap() + regen, 0, next_stats->get_max_ap()));
				} else {
					next_stats->set_ap(next_stats->get_max_ap());
				}
			}
		}

		Ref<GameEvent> start_ev = GameEvent::turn_started(next_actor);
		start_ev->set_round(next->get_round());
		next->append_event(start_ev);
		result->append_event(start_ev);
	}

	result->set_accepted(true);
	result->set_next_state(next);
	return result;
}
