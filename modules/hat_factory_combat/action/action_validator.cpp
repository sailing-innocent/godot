#include "action_validator.h"

#include "action_result.h"
#include "battle_action.h"
#include "components/movement_component.h"
#include "components/skill_component.h"
#include "components/stats_component.h"
#include "components/transform_component.h"
#include "components/turn_component.h"
#include "query/query_api.h"
#include "skill/skill_cost.h"
#include "skill/skill_def.h"
#include "skill/skill_effect.h"
#include "state/battle_state.h"
#include "state/combat_entity.h"
#include "state/rule_set.h"
#include "modules/hat_factory_hex_grid/hex_grid_map_data.h"

#include "core/object/class_db.h"

void ActionValidator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("validate", "state", "action"), &ActionValidator::validate);
	ClassDB::bind_method(D_METHOD("validate_move", "state", "action"), &ActionValidator::validate_move);
	ClassDB::bind_method(D_METHOD("validate_skill", "state", "action"), &ActionValidator::validate_skill);
}

Ref<ActionResult> ActionValidator::validate(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const {
	Ref<ActionResult> result;
	result.instantiate();
	result->set_action(p_action);
	result->set_next_state(p_state);

	if (p_state.is_null() || p_action.is_null()) {
		result->set_reason("Null state or action");
		return result;
	}

	CombatEntity *actor = p_state->get_entity(p_action->get_actor_id());
	if (actor == nullptr) {
		result->set_reason("Actor does not exist");
		return result;
	}

	// Only the current actor may act.
	if (p_state->get_current_actor() != 0 && p_state->get_current_actor() != p_action->get_actor_id()) {
		result->set_reason("Not the current actor");
		return result;
	}

	Ref<TurnComponent> turn = actor->get_component(StringName("Turn"));
	Ref<TransformComponent> transform = actor->get_component(StringName("Transform"));
	Dictionary payload = p_action->get_payload();

	int type = p_action->get_type();
	switch (type) {
		case BattleAction::MOVE: {
			if (turn.is_valid() && turn->get_moved()) {
				result->set_reason("Actor has already moved");
				return result;
			}
			if (transform.is_null()) {
				result->set_reason("Actor has no Transform component");
				return result;
			}
			Vector2i destination = payload.get("target", Vector2i());
			TypedArray<Vector2i> path;
			if (payload.has("path")) {
				path = TypedArray<Vector2i>(payload["path"]);
			}
			if (path.size() > 0) {
				destination = path[path.size() - 1];
			}
			int range = 0;
			if (actor->has_component(StringName("Movement"))) {
				Ref<MovementComponent> movement = actor->get_component(StringName("Movement"));
				if (movement.is_valid()) {
					range = movement->get_range();
				}
			}
			Ref<QueryAPI> query;
			query.instantiate();
			query->set_state(p_state);
			TypedArray<Vector2i> reachable = query->get_reachable_cells(p_action->get_actor_id(), range);
			bool found = false;
			for (int i = 0; i < reachable.size(); i++) {
				if (reachable[i] == destination) {
					found = true;
					break;
				}
			}
			if (!found) {
				result->set_reason("Destination not reachable");
				return result;
			}
			result->set_accepted(true);
			return result;
		}
		case BattleAction::SKILL: {
			if (turn.is_valid() && turn->get_acted()) {
				result->set_reason("Actor has already acted");
				return result;
			}
			StringName skill_id = payload.get("skill_id", StringName());
			if (skill_id == StringName()) {
				result->set_reason("Missing skill_id");
				return result;
			}
			if (actor->has_component(StringName("Skill"))) {
				Ref<SkillComponent> skills = actor->get_component(StringName("Skill"));
				if (skills.is_valid()) {
					TypedArray<StringName> known = skills->get_skill_ids();
					bool knows = false;
					for (int i = 0; i < known.size(); i++) {
						if (StringName(known[i]) == skill_id) {
							knows = true;
							break;
						}
					}
					if (!knows) {
						result->set_reason("Actor does not know this skill");
						return result;
					}
				}
			}
			Ref<SkillDef> def;
			Ref<RuleSet> rules = p_state->get_rules();
			if (rules.is_valid()) {
				def = rules->get_skill_def(skill_id);
			}
			if (def.is_null()) {
				result->set_reason("Skill definition unavailable");
				return result;
			}
			TypedArray<SkillEffect> effects = def->get_effects();
			if (def->get_range() < 0 || def->get_min_range() < 0 || def->get_min_range() > def->get_range() || def->get_aoe_radius() < 0) {
				result->set_reason("Invalid skill range");
				return result;
			}
			if (effects.size() == 0) {
				result->set_reason("Skill has no effects");
				return result;
			}
			for (int i = 0; i < effects.size(); i++) {
				Ref<SkillEffect> effect = effects[i];
				if (effect.is_null()) {
					result->set_reason("Missing skill effect");
					return result;
				}
			}
			{
				Vector2i target = payload.get("target", transform.is_valid() ? transform->get_coord() : Vector2i());
				Ref<HexGridMapData> grid = p_state->get_grid_data();
				if (grid.is_valid() && !grid->has_cell(target)) {
					result->set_reason("Target cell does not exist");
					return result;
				}
				int target_id = payload.get("target_entity", 0);
				Ref<CombatEntity> occupant = p_state->get_entity(target_id);
				Ref<TransformComponent> target_transform;
				if (occupant.is_valid()) {
					target_transform = occupant->get_component(StringName("Transform"));
				}
				if (def->get_target_type() == 0 && (target_id != actor->get_entity_id() || target != transform->get_coord())) {
					result->set_reason("Skill must target self");
					return result;
				}
				if (def->get_target_type() != 3 && def->get_target_type() != 0 && def->get_aoe_radius() == 0 &&
					(target_id == 0 || occupant.is_null() || target_transform.is_null() || target_transform->get_coord() != target ||
					((def->get_target_type() == 1) != (occupant->get_owner() == actor->get_owner())))) {
					result->set_reason("Invalid skill target");
					return result;
				}
				if (target_id != 0 && (occupant.is_null() || target_transform.is_null() || target_transform->get_coord() != target)) {
					result->set_reason("Target entity is not at target cell");
					return result;
				}
				// Range check against the caster's position.
				if (transform.is_valid()) {
					int dist = QueryAPI::hex_distance(transform->get_coord(), target);
					if (dist > def->get_range() || dist < def->get_min_range()) {
						result->set_reason("Target out of range");
						return result;
					}
				}
				// Cost check.
				Ref<StatsComponent> stats = actor->get_component(StringName("Stats"));
				TypedArray<SkillCost> costs = def->get_costs();
				int total_ap = 0;
				int total_mp = 0;
				int total_hp = 0;
				for (int i = 0; i < costs.size(); i++) {
					Ref<SkillCost> cost = costs[i];
					if (cost.is_null() || stats.is_null() || cost->get_amount() < 0) {
						result->set_reason("Invalid skill cost");
						return result;
					}
					switch (cost->get_cost_type()) {
						case 0:
							total_ap += cost->get_amount();
							break;
						case 1:
							total_mp += cost->get_amount();
							break;
						case 2:
							total_hp += cost->get_amount();
							break;
						default:
							result->set_reason("Unsupported skill cost");
							return result;
					}
				}
				if (stats->get_ap() < total_ap || stats->get_mp() < total_mp || stats->get_hp() <= total_hp) {
					result->set_reason("Not enough skill resources");
					return result;
				}
			}
			result->set_accepted(true);
			return result;
		}
		case BattleAction::ITEM: {
			if (turn.is_valid() && turn->get_acted()) {
				result->set_reason("Actor has already acted");
				return result;
			}
			StringName item_id = payload.get("item_id", StringName());
			if (item_id == StringName()) {
				result->set_reason("Missing item_id");
				return result;
			}
			result->set_accepted(true);
			return result;
		}
		case BattleAction::WAIT:
		case BattleAction::END_TURN:
		case BattleAction::ESCAPE:
			result->set_accepted(true);
			return result;
		default:
			result->set_reason("Action type not supported");
			return result;
	}
}

Ref<ActionResult> ActionValidator::validate_move(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const {
	Ref<BattleAction> copy = p_action.is_valid() ? p_action->clone() : Ref<BattleAction>();
	if (copy.is_valid()) {
		copy->set_type(BattleAction::MOVE);
	}
	return validate(p_state, copy);
}

Ref<ActionResult> ActionValidator::validate_skill(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const {
	Ref<BattleAction> copy = p_action.is_valid() ? p_action->clone() : Ref<BattleAction>();
	if (copy.is_valid()) {
		copy->set_type(BattleAction::SKILL);
	}
	return validate(p_state, copy);
}
