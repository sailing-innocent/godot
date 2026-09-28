#include "combat_attr_component.h"

#include "core/object/class_db.h"

void CombatAttrComponent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_attack", "attack"), &CombatAttrComponent::set_attack);
	ClassDB::bind_method(D_METHOD("set_defense", "defense"), &CombatAttrComponent::set_defense);
	ClassDB::bind_method(D_METHOD("set_magic", "magic"), &CombatAttrComponent::set_magic);
	ClassDB::bind_method(D_METHOD("set_resistance", "resistance"), &CombatAttrComponent::set_resistance);
	ClassDB::bind_method(D_METHOD("set_aim", "aim"), &CombatAttrComponent::set_aim);
	ClassDB::bind_method(D_METHOD("set_evasion", "evasion"), &CombatAttrComponent::set_evasion);
	ClassDB::bind_method(D_METHOD("set_crit_rate", "crit_rate"), &CombatAttrComponent::set_crit_rate);
	ClassDB::bind_method(D_METHOD("set_crit_damage", "crit_damage"), &CombatAttrComponent::set_crit_damage);
	ClassDB::bind_method(D_METHOD("get_attack"), &CombatAttrComponent::get_attack);
	ClassDB::bind_method(D_METHOD("get_defense"), &CombatAttrComponent::get_defense);
	ClassDB::bind_method(D_METHOD("get_magic"), &CombatAttrComponent::get_magic);
	ClassDB::bind_method(D_METHOD("get_resistance"), &CombatAttrComponent::get_resistance);
	ClassDB::bind_method(D_METHOD("get_aim"), &CombatAttrComponent::get_aim);
	ClassDB::bind_method(D_METHOD("get_evasion"), &CombatAttrComponent::get_evasion);
	ClassDB::bind_method(D_METHOD("get_crit_rate"), &CombatAttrComponent::get_crit_rate);
	ClassDB::bind_method(D_METHOD("get_crit_damage"), &CombatAttrComponent::get_crit_damage);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "attack"), "set_attack", "get_attack");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "defense"), "set_defense", "get_defense");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "magic"), "set_magic", "get_magic");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "resistance"), "set_resistance", "get_resistance");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "aim"), "set_aim", "get_aim");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "evasion"), "set_evasion", "get_evasion");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "crit_rate"), "set_crit_rate", "get_crit_rate");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "crit_damage"), "set_crit_damage", "get_crit_damage");
}

Dictionary CombatAttrComponent::to_dict() const {
	Dictionary d;
	d["attack"] = attack;
	d["defense"] = defense;
	d["magic"] = magic;
	d["resistance"] = resistance;
	d["aim"] = aim;
	d["evasion"] = evasion;
	d["crit_rate"] = crit_rate;
	d["crit_damage"] = crit_damage;
	return d;
}

void CombatAttrComponent::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("attack")) { attack = p_dict["attack"]; }
	if (p_dict.has("defense")) { defense = p_dict["defense"]; }
	if (p_dict.has("magic")) { magic = p_dict["magic"]; }
	if (p_dict.has("resistance")) { resistance = p_dict["resistance"]; }
	if (p_dict.has("aim")) { aim = p_dict["aim"]; }
	if (p_dict.has("evasion")) { evasion = p_dict["evasion"]; }
	if (p_dict.has("crit_rate")) { crit_rate = p_dict["crit_rate"]; }
	if (p_dict.has("crit_damage")) { crit_damage = p_dict["crit_damage"]; }
}

Ref<CombatComponent> CombatAttrComponent::clone() const {
	Ref<CombatAttrComponent> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
