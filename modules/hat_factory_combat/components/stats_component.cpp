#include "stats_component.h"

#include "core/object/class_db.h"

void StatsComponent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_hp", "hp"), &StatsComponent::set_hp);
	ClassDB::bind_method(D_METHOD("set_max_hp", "max_hp"), &StatsComponent::set_max_hp);
	ClassDB::bind_method(D_METHOD("set_mp", "mp"), &StatsComponent::set_mp);
	ClassDB::bind_method(D_METHOD("set_max_mp", "max_mp"), &StatsComponent::set_max_mp);
	ClassDB::bind_method(D_METHOD("set_ap", "ap"), &StatsComponent::set_ap);
	ClassDB::bind_method(D_METHOD("set_max_ap", "max_ap"), &StatsComponent::set_max_ap);
	ClassDB::bind_method(D_METHOD("get_hp"), &StatsComponent::get_hp);
	ClassDB::bind_method(D_METHOD("get_max_hp"), &StatsComponent::get_max_hp);
	ClassDB::bind_method(D_METHOD("get_mp"), &StatsComponent::get_mp);
	ClassDB::bind_method(D_METHOD("get_max_mp"), &StatsComponent::get_max_mp);
	ClassDB::bind_method(D_METHOD("get_ap"), &StatsComponent::get_ap);
	ClassDB::bind_method(D_METHOD("get_max_ap"), &StatsComponent::get_max_ap);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "hp"), "set_hp", "get_hp");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_hp"), "set_max_hp", "get_max_hp");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mp"), "set_mp", "get_mp");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_mp"), "set_max_mp", "get_max_mp");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ap"), "set_ap", "get_ap");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_ap"), "set_max_ap", "get_max_ap");
}

Dictionary StatsComponent::to_dict() const {
	Dictionary d;
	d["hp"] = hp;
	d["max_hp"] = max_hp;
	d["mp"] = mp;
	d["max_mp"] = max_mp;
	d["ap"] = ap;
	d["max_ap"] = max_ap;
	return d;
}

void StatsComponent::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("hp")) { hp = p_dict["hp"]; }
	if (p_dict.has("max_hp")) { max_hp = p_dict["max_hp"]; }
	if (p_dict.has("mp")) { mp = p_dict["mp"]; }
	if (p_dict.has("max_mp")) { max_mp = p_dict["max_mp"]; }
	if (p_dict.has("ap")) { ap = p_dict["ap"]; }
	if (p_dict.has("max_ap")) { max_ap = p_dict["max_ap"]; }
}

Ref<CombatComponent> StatsComponent::clone() const {
	Ref<StatsComponent> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
