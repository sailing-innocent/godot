#include "skill_resolver.h"

#include "../action_result.h"
#include "../battle_action.h"
#include "components/stats_component.h"
#include "components/status_component.h"
#include "components/transform_component.h"
#include "components/turn_component.h"
#include "event/game_event.h"
#include "query/query_api.h"
#include "skill/hit_result.h"
#include "skill/skill_context.h"
#include "skill/skill_cost.h"
#include "skill/skill_def.h"
#include "skill/skill_executor.h"
#include "state/battle_state.h"
#include "state/combat_entity.h"
#include "state/rule_set.h"

#include "core/math/math_funcs.h"
#include "core/object/class_db.h"

void SkillResolver::_bind_methods() {
	ClassDB::bind_method(D_METHOD("resolve", "state", "action"), &SkillResolver::resolve);
}

TypedArray<int> SkillResolver::_gather_targets(const Ref<BattleState> &p_state, const Ref<CombatEntity> &p_caster, const Ref<SkillDef> &p_def, const Vector2i &p_target_coord, int p_target_entity) const {
	TypedArray<int> targets;
	if (p_state.is_null() || p_caster.is_null()) {
		return targets;
	}

	int target_type = p_def.is_valid() ? p_def->get_target_type() : 3;
	int aoe_radius = p_def.is_valid() ? p_def->get_aoe_radius() : 0;
	int caster_owner = p_caster->get_owner();

	if (target_type == 0) {
		// Self only.
		targets.push_back(p_caster->get_entity_id());
		return targets;
	}

	TypedArray<CombatEntity> ents = p_state->get_entities();
	for (int i = 0; i < ents.size(); i++) {
		Ref<CombatEntity> e = ents[i];
		if (e.is_null() || !e->has_component(StringName("Transform"))) {
			continue;
		}
		Ref<TransformComponent> et = e->get_component(StringName("Transform"));
		if (et.is_null()) {
			continue;
		}
		bool in_area = false;
		if (aoe_radius > 0) {
			in_area = QueryAPI::hex_distance(et->get_coord(), p_target_coord) <= aoe_radius;
		} else if (p_target_entity != 0) {
			in_area = e->get_entity_id() == p_target_entity;
		} else {
			in_area = et->get_coord() == p_target_coord;
		}
		if (!in_area) {
			continue;
		}
		// Filter by target type: 1 = ally, 2 = enemy, 3 = any.
		if (target_type == 1 && e->get_owner() != caster_owner) {
			continue;
		}
		if (target_type == 2 && e->get_owner() == caster_owner) {
			continue;
		}
		targets.push_back(e->get_entity_id());
	}
	return targets;
}

Ref<ActionResult> SkillResolver::resolve(const Ref<BattleState> &p_state, const Ref<BattleAction> &p_action) const {
	Ref<ActionResult> result;
	result.instantiate();
	result->set_action(p_action);

	if (p_state.is_null() || p_action.is_null()) {
		result->set_reason("Null state or action");
		return result;
	}

	Dictionary payload = p_action->get_payload();
	StringName skill_id = payload.get("skill_id", StringName());
	if (skill_id == StringName()) {
		result->set_reason("Missing skill_id");
		return result;
	}

	Ref<SkillDef> def;
	Ref<RuleSet> rules = p_state->get_rules();
	if (rules.is_valid()) {
		def = rules->get_skill_def(skill_id);
	}

	Ref<BattleState> next = p_state->clone();
	CombatEntity *caster = next->get_entity(p_action->get_actor_id());
	if (caster == nullptr) {
		result->set_reason("Actor does not exist");
		return result;
	}
	Ref<CombatEntity> caster_ref = next->get_entity(p_action->get_actor_id());

	// Pay costs.
	Ref<StatsComponent> caster_stats = caster->get_component(StringName("Stats"));
	if (def.is_valid() && caster_stats.is_valid()) {
		TypedArray<SkillCost> costs = def->get_costs();
		for (int i = 0; i < costs.size(); i++) {
			Ref<SkillCost> cost = costs[i];
			if (cost.is_null()) {
				continue;
			}
			switch (cost->get_cost_type()) {
				case 0:
					caster_stats->set_ap(MAX(0, caster_stats->get_ap() - cost->get_amount()));
					break;
				case 1:
					caster_stats->set_mp(MAX(0, caster_stats->get_mp() - cost->get_amount()));
					break;
				case 2:
					caster_stats->set_hp(MAX(1, caster_stats->get_hp() - cost->get_amount()));
					break;
				default:
					break;
			}
		}
	}

	// Mark as acted.
	Ref<TurnComponent> turn = caster->get_component(StringName("Turn"));
	if (turn.is_valid()) {
		turn->set_acted(true);
	}

	Vector2i target_coord = payload.get("target", Vector2i());
	int target_entity = payload.get("target_entity", 0);

	// Emit the cast event first so presenters can play the wind-up animation.
	Dictionary cast_params;
	cast_params["skill_id"] = skill_id;
	cast_params["target"] = target_coord;
	cast_params["target_entity"] = target_entity;
	Ref<GameEvent> cast_ev;
	cast_ev.instantiate();
	cast_ev->set_type(GameEvent::SKILL_CAST);
	cast_ev->set_actor_id(p_action->get_actor_id());
	cast_ev->set_target_id(target_entity);
	cast_ev->set_params(cast_params);
	cast_ev->set_round(next->get_round());
	next->append_event(cast_ev);
	result->append_event(cast_ev);

	// Resolve effects per target.
	TypedArray<int> targets = _gather_targets(next, caster_ref, def, target_coord, target_entity);
	SkillExecutor executor;
	TypedArray<int> died;

	for (int t = 0; t < targets.size(); t++) {
		int tid = targets[t];
		CombatEntity *target = next->get_entity(tid);
		if (target == nullptr) {
			continue;
		}
		if (def.is_valid()) {
			Ref<SkillContext> ctx;
			ctx.instantiate();
			ctx->set_state(next);
			ctx->set_caster_id(p_action->get_actor_id());
			ctx->set_target_id(tid);
			ctx->set_target_coord(target_coord);
			ctx->set_skill(def);
			ctx->set_rules(rules);

			TypedArray<HitResult> hits = executor.execute(ctx);
			_apply_hit_results(next, result, p_action->get_actor_id(), tid, hits, died);
		}
	}

	// Remove dead entities from the battle.
	for (int i = 0; i < died.size(); i++) {
		next->remove_entity(died[i]);
		TypedArray<int> order = next->get_turn_order();
		for (int j = order.size() - 1; j >= 0; j--) {
			if ((int)order[j] == (int)died[i]) {
				order.remove_at(j);
			}
		}
		next->set_turn_order(order);
	}

	result->set_accepted(true);
	result->set_next_state(next);
	return result;
}

void SkillResolver::_apply_hit_results(const Ref<BattleState> &p_state, const Ref<ActionResult> &p_result, int p_caster_id, int p_target_id, const TypedArray<HitResult> &p_hits, TypedArray<int> &r_died) const {
	CombatEntity *target = p_state->get_entity(p_target_id);
	if (target == nullptr) {
		return;
	}
	Ref<StatsComponent> stats = target->get_component(StringName("Stats"));
	Ref<StatusComponent> status = target->get_component(StringName("Status"));

	for (int i = 0; i < p_hits.size(); i++) {
		Ref<HitResult> hit = p_hits[i];
		if (hit.is_null()) {
			continue;
		}
		if (hit->get_hit() && hit->get_damage() > 0 && stats.is_valid()) {
			int remaining = MAX(0, stats->get_hp() - hit->get_damage());
			stats->set_hp(remaining);
			Ref<GameEvent> ev = GameEvent::unit_hit(p_caster_id, p_target_id, hit->get_damage(), remaining);
			Dictionary hit_params = ev->get_params();
			hit_params["hit"] = true;
			ev->set_params(hit_params);
			ev->set_round(p_state->get_round());
			p_state->append_event(ev);
			p_result->append_event(ev);
			if (remaining <= 0) {
				Ref<GameEvent> died_ev = GameEvent::unit_died(p_target_id);
				died_ev->set_round(p_state->get_round());
				p_state->append_event(died_ev);
				p_result->append_event(died_ev);
				r_died.push_back(p_target_id);
			}
		} else if (!hit->get_hit()) {
			// A genuine miss (accuracy system): no damage, but still surface an
			// event so presenters can play the dodge animation.
			int remaining = stats.is_valid() ? stats->get_hp() : 0;
			Ref<GameEvent> ev = GameEvent::unit_hit(p_caster_id, p_target_id, 0, remaining);
			Dictionary hit_params = ev->get_params();
			hit_params["hit"] = false;
			ev->set_params(hit_params);
			ev->set_round(p_state->get_round());
			p_state->append_event(ev);
			p_result->append_event(ev);
		}
		if (hit->get_healing() > 0 && stats.is_valid()) {
			int remaining = MIN(stats->get_max_hp(), stats->get_hp() + hit->get_healing());
			stats->set_hp(remaining);
			Ref<GameEvent> ev = GameEvent::unit_healed(p_caster_id, p_target_id, hit->get_healing(), remaining);
			ev->set_round(p_state->get_round());
			p_state->append_event(ev);
			p_result->append_event(ev);
		}
		if (hit->get_status_id() != StringName() && status.is_valid()) {
			Ref<StatusEffectInstance> inst;
			inst.instantiate();
			inst->set_effect_id(hit->get_status_id());
			inst->set_stacks(hit->get_status_stacks());
			inst->set_remaining_turns(hit->get_status_turns());
			inst->set_source_entity_id(p_caster_id);
			status->add_effect(inst);
			Ref<GameEvent> ev = GameEvent::status_applied(p_caster_id, p_target_id, hit->get_status_id(), hit->get_status_stacks(), hit->get_status_turns());
			ev->set_round(p_state->get_round());
			p_state->append_event(ev);
			p_result->append_event(ev);
		}
	}
}
