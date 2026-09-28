#include "damage_effect.h"

#include "../hit_result.h"
#include "../skill_context.h"
#include "../skill_def.h"
#include "state/battle_state.h"
#include "state/combat_entity.h"
#include "state/rule_set.h"
#include "components/combat_attr_component.h"
#include "components/transform_component.h"
#include "modules/hat_factory_hex_grid/hex_cell_data.h"
#include "modules/hat_factory_hex_grid/hex_grid_map_data.h"

#include "core/math/math_funcs.h"
#include "core/object/class_db.h"

namespace {
int _cell_height_at(const Ref<BattleState> &p_state, const Vector2i &p_coord) {
	if (p_state.is_null()) {
		return 0;
	}
	Ref<HexGridMapData> grid = p_state->get_grid_data();
	if (grid.is_null()) {
		return 0;
	}
	Ref<HexCellData> cell = grid->get_cell(p_coord);
	if (cell.is_null()) {
		return 0;
	}
	return cell->get_height();
}

StringName _terrain_id_at(const Ref<BattleState> &p_state, const Vector2i &p_coord) {
	if (p_state.is_null()) {
		return StringName();
	}
	Ref<HexGridMapData> grid = p_state->get_grid_data();
	if (grid.is_null()) {
		return StringName();
	}
	Ref<HexCellData> cell = grid->get_cell(p_coord);
	if (cell.is_null()) {
		return StringName();
	}
	return cell->get_terrain_id();
}
} //namespace

void DamageEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_base_damage", "base_damage"), &DamageEffect::set_base_damage);
	ClassDB::bind_method(D_METHOD("get_base_damage"), &DamageEffect::get_base_damage);
	ClassDB::bind_method(D_METHOD("set_use_rule_formula", "use_rule_formula"), &DamageEffect::set_use_rule_formula);
	ClassDB::bind_method(D_METHOD("get_use_rule_formula"), &DamageEffect::get_use_rule_formula);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "base_damage"), "set_base_damage", "get_base_damage");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_rule_formula"), "set_use_rule_formula", "get_use_rule_formula");
}

void DamageEffect::apply(const Ref<SkillContext> &p_ctx, TypedArray<HitResult> &r_results) const {
	if (p_ctx.is_null() || p_ctx->get_state().is_null()) {
		return;
	}
	CombatEntity *caster = p_ctx->get_state()->get_entity(p_ctx->get_caster_id());
	CombatEntity *target = p_ctx->get_state()->get_entity(p_ctx->get_target_id());
	if (target == nullptr) {
		return;
	}

	int damage = base_damage;
	if (use_rule_formula && p_ctx->get_rules().is_valid()) {
		Ref<CombatEntity> caster_ref;
		if (caster != nullptr) {
			// Find caster ref via entity id in entities array.
			TypedArray<CombatEntity> ents = p_ctx->get_state()->get_entities();
			for (int i = 0; i < ents.size(); i++) {
				Ref<CombatEntity> e = ents[i];
				if (e.is_valid() && e->get_entity_id() == p_ctx->get_caster_id()) {
					caster_ref = e;
					break;
				}
			}
		}
		Ref<CombatEntity> target_ref;
		TypedArray<CombatEntity> ents = p_ctx->get_state()->get_entities();
		for (int i = 0; i < ents.size(); i++) {
			Ref<CombatEntity> e = ents[i];
			if (e.is_valid() && e->get_entity_id() == p_ctx->get_target_id()) {
				target_ref = e;
				break;
			}
		}
		damage = p_ctx->get_rules()->evaluate_damage(caster_ref, target_ref, base_damage);
	}

	Ref<RuleSet> rules = p_ctx->get_rules();
	Ref<BattleState> state = p_ctx->get_state();

	// Terrain height modifier: standing above the target increases damage,
	// standing below it reduces damage (symmetric, capped by the ruleset).
	Vector2i caster_coord;
	Vector2i target_coord;
	if (caster != nullptr && caster->has_component(StringName("Transform"))) {
		Ref<TransformComponent> t = caster->get_component(StringName("Transform"));
		if (t.is_valid()) {
			caster_coord = t->get_coord();
		}
	}
	if (target->has_component(StringName("Transform"))) {
		Ref<TransformComponent> t = target->get_component(StringName("Transform"));
		if (t.is_valid()) {
			target_coord = t->get_coord();
		}
	}
	if (rules.is_valid()) {
		int height_diff = _cell_height_at(state, caster_coord) - _cell_height_at(state, target_coord);
		damage = Math::round((float)damage * rules->get_height_damage_multiplier(height_diff));
	}

	bool landed = true;
	if (rules.is_valid() && rules->get_use_accuracy_system()) {
		int aim = 0;
		int evasion = 0;
		if (caster != nullptr && caster->has_component(StringName("CombatAttr"))) {
			Ref<CombatAttrComponent> attr = caster->get_component(StringName("CombatAttr"));
			if (attr.is_valid()) {
				aim = attr->get_aim();
			}
		}
		if (target->has_component(StringName("CombatAttr"))) {
			Ref<CombatAttrComponent> attr = target->get_component(StringName("CombatAttr"));
			if (attr.is_valid()) {
				evasion = attr->get_evasion();
			}
		}
		StringName defender_terrain = _terrain_id_at(state, target_coord);
		if (rules->is_conceal_terrain(defender_terrain)) {
			evasion += rules->get_conceal_evasion_bonus();
		}
		int hit_chance = CLAMP(100 - (aim - evasion), 5, 100);
		float roll = state.is_valid() ? state->next_random_float() * 100.0f : 0.0f;
		landed = roll < (float)hit_chance;
	}

	Ref<HitResult> hr;
	hr.instantiate();
	hr->set_target_id(p_ctx->get_target_id());
	hr->set_hit(landed);
	if (landed) {
		hr->set_damage(damage);
		hr->set_log("Damage: " + String::num_int64(damage));
	} else {
		hr->set_damage(0);
		hr->set_log("Miss");
	}
	r_results.push_back(hr);
}
