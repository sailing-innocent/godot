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
#include "state/battle_state.h"
#include "state/combat_entity.h"
#include "state/rule_set.h"

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
			if (def.is_valid()) {
				// Range check against the caster's position.
				if (transform.is_valid()) {
					Vector2i target = payload.get("target", transform->get_coord());
					int dist = QueryAPI::hex_distance(transform->get_coord(), target);
					if (dist > def->get_range() || dist < def->get_min_range()) {
						result->set_reason("Target out of range");
						return result;
					}
				}
				// Cost check.
				Ref<StatsComponent> stats = actor->get_component(StringName("Stats"));
				TypedArray<SkillCost> costs = def->get_costs();
				for (int i = 0; i < costs.size(); i++) {
					Ref<SkillCost> cost = costs[i];
					if (cost.is_null() || stats.is_null()) {
						continue;
					}
					switch (cost->get_cost_type()) {
						case 0:
							if (stats->get_ap() < cost->get_amount()) {
								result->set_reason("Not enough AP");
								return result;
							}
							break;
						case 1:
							if (stats->get_mp() < cost->get_amount()) {
								result->set_reason("Not enough MP");
								return result;
							}
							break;
						case 2:
							if (stats->get_hp() <= cost->get_amount()) {
								result->set_reason("Not enough HP");
								return result;
							}
							break;
						default:
							break;
					}
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
