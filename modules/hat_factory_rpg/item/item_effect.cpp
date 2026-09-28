#include "item_effect.h"

#include "modules/hat_factory_combat/state/combat_entity.h"
#include "modules/hat_factory_combat/components/combat_attr_component.h"
#include "modules/hat_factory_combat/components/stats_component.h"
#include "modules/hat_factory_combat/components/status_component.h"

#include "core/math/math_funcs.h"
#include "core/object/class_db.h"

void ItemEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_effect_id", "effect_id"), &ItemEffect::set_effect_id);
	ClassDB::bind_method(D_METHOD("get_effect_id"), &ItemEffect::get_effect_id);

	ClassDB::bind_method(D_METHOD("set_effect_type", "effect_type"), &ItemEffect::set_effect_type);
	ClassDB::bind_method(D_METHOD("get_effect_type"), &ItemEffect::get_effect_type);

	ClassDB::bind_method(D_METHOD("set_value", "value"), &ItemEffect::set_value);
	ClassDB::bind_method(D_METHOD("get_value"), &ItemEffect::get_value);

	ClassDB::bind_method(D_METHOD("set_duration", "duration"), &ItemEffect::set_duration);
	ClassDB::bind_method(D_METHOD("get_duration"), &ItemEffect::get_duration);

	ClassDB::bind_method(D_METHOD("apply", "target"), &ItemEffect::apply);

	ClassDB::bind_method(D_METHOD("to_dict"), &ItemEffect::to_dict);
	ClassDB::bind_method(D_METHOD("from_dict", "dict"), &ItemEffect::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &ItemEffect::clone);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "effect_id"), "set_effect_id", "get_effect_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "effect_type", PROPERTY_HINT_ENUM, "Attack,Defense,Magic,Resistance,Aim,Evasion,CritRate,CritDamage,HealHP,RestoreMP,RestoreAP,ApplyStatus"), "set_effect_type", "get_effect_type");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "value"), "set_value", "get_value");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "duration"), "set_duration", "get_duration");

	BIND_ENUM_CONSTANT(EFFECT_ATTACK);
	BIND_ENUM_CONSTANT(EFFECT_DEFENSE);
	BIND_ENUM_CONSTANT(EFFECT_MAGIC);
	BIND_ENUM_CONSTANT(EFFECT_RESISTANCE);
	BIND_ENUM_CONSTANT(EFFECT_AIM);
	BIND_ENUM_CONSTANT(EFFECT_EVASION);
	BIND_ENUM_CONSTANT(EFFECT_CRIT_RATE);
	BIND_ENUM_CONSTANT(EFFECT_CRIT_DAMAGE);
	BIND_ENUM_CONSTANT(EFFECT_HEAL_HP);
	BIND_ENUM_CONSTANT(EFFECT_RESTORE_MP);
	BIND_ENUM_CONSTANT(EFFECT_RESTORE_AP);
	BIND_ENUM_CONSTANT(EFFECT_APPLY_STATUS);
}

void ItemEffect::apply(const Ref<CombatEntity> &p_target) const {
	if (p_target.is_null()) {
		return;
	}

	// Stat-restoring and status effects operate on components other than CombatAttr.
	switch (effect_type) {
		case EFFECT_HEAL_HP: {
			Ref<StatsComponent> stats = p_target->get_component(StringName("Stats"));
			if (stats.is_valid()) {
				stats->set_hp(MIN(stats->get_max_hp(), stats->get_hp() + value));
			}
			return;
		}
		case EFFECT_RESTORE_MP: {
			Ref<StatsComponent> stats = p_target->get_component(StringName("Stats"));
			if (stats.is_valid()) {
				stats->set_mp(MIN(stats->get_max_mp(), stats->get_mp() + value));
			}
			return;
		}
		case EFFECT_RESTORE_AP: {
			Ref<StatsComponent> stats = p_target->get_component(StringName("Stats"));
			if (stats.is_valid()) {
				stats->set_ap(MIN(stats->get_max_ap(), stats->get_ap() + value));
			}
			return;
		}
		case EFFECT_APPLY_STATUS: {
			Ref<StatusComponent> status = p_target->get_component(StringName("Status"));
			if (status.is_valid() && effect_id != StringName()) {
				Ref<StatusEffectInstance> inst;
				inst.instantiate();
				inst->set_effect_id(effect_id);
				inst->set_stacks(MAX(1, value));
				inst->set_remaining_turns(MAX(1, duration));
				status->add_effect(inst);
			}
			return;
		}
		default:
			break;
	}

	Ref<CombatComponent> comp = p_target->get_component(StringName("CombatAttr"));
	if (comp.is_null()) {
		return;
	}

	CombatAttrComponent *attr = Object::cast_to<CombatAttrComponent>(comp.ptr());
	if (attr == nullptr) {
		return;
	}

	switch (effect_type) {
		case EFFECT_ATTACK:
			attr->set_attack(attr->get_attack() + value);
			break;
		case EFFECT_DEFENSE:
			attr->set_defense(attr->get_defense() + value);
			break;
		case EFFECT_MAGIC:
			attr->set_magic(attr->get_magic() + value);
			break;
		case EFFECT_RESISTANCE:
			attr->set_resistance(attr->get_resistance() + value);
			break;
		case EFFECT_AIM:
			attr->set_aim(attr->get_aim() + value);
			break;
		case EFFECT_EVASION:
			attr->set_evasion(attr->get_evasion() + value);
			break;
		case EFFECT_CRIT_RATE:
			attr->set_crit_rate(attr->get_crit_rate() + static_cast<float>(value));
			break;
		case EFFECT_CRIT_DAMAGE:
			attr->set_crit_damage(attr->get_crit_damage() + static_cast<float>(value));
			break;
		default:
			break;
	}
}

Dictionary ItemEffect::to_dict() const {
	Dictionary d;
	d["effect_id"] = effect_id;
	d["effect_type"] = effect_type;
	d["value"] = value;
	d["duration"] = duration;
	return d;
}

void ItemEffect::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("effect_id")) {
		effect_id = p_dict["effect_id"];
	}
	if (p_dict.has("effect_type")) {
		effect_type = p_dict["effect_type"];
	}
	if (p_dict.has("value")) {
		value = p_dict["value"];
	}
	if (p_dict.has("duration")) {
		duration = p_dict["duration"];
	}
}

Ref<ItemEffect> ItemEffect::clone() const {
	Ref<ItemEffect> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
