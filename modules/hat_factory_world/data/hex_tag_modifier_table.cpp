/**************************************************************************/
/*  hex_tag_modifier_table.cpp                                            */
/**************************************************************************/
#include "hex_tag_modifier_table.h"

#include "core/object/class_db.h"

void HexTagModifierTable::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_table_id", "id"), &HexTagModifierTable::set_table_id);
	ClassDB::bind_method(D_METHOD("get_table_id"), &HexTagModifierTable::get_table_id);
	ClassDB::bind_method(D_METHOD("set_modifiers", "modifiers"), &HexTagModifierTable::set_modifiers);
	ClassDB::bind_method(D_METHOD("get_modifiers"), &HexTagModifierTable::get_modifiers);
	ClassDB::bind_method(D_METHOD("set_value", "tag", "stat", "value"), &HexTagModifierTable::set_value);
	ClassDB::bind_method(D_METHOD("get_value", "tag", "stat", "default"), &HexTagModifierTable::get_value, DEFVAL(0.0));
	ClassDB::bind_method(D_METHOD("get_stats_for_tag", "tag"), &HexTagModifierTable::get_stats_for_tag);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexTagModifierTable::to_dict);
	ClassDB::bind_static_method("HexTagModifierTable", D_METHOD("from_dict", "dict"), &HexTagModifierTable::from_dict);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "table_id"), "set_table_id", "get_table_id");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "modifiers"), "set_modifiers", "get_modifiers");
}

void HexTagModifierTable::set_table_id(const StringName &p_id) { table_id = p_id; }
StringName HexTagModifierTable::get_table_id() const { return table_id; }
void HexTagModifierTable::set_modifiers(const Dictionary &p_modifiers) { modifiers = p_modifiers; }
Dictionary HexTagModifierTable::get_modifiers() const { return modifiers; }

void HexTagModifierTable::set_value(const StringName &p_tag, const StringName &p_stat, double p_value) {
	Dictionary stats;
	if (modifiers.has(p_tag)) {
		stats = modifiers[p_tag];
	}
	stats[p_stat] = p_value;
	modifiers[p_tag] = stats;
}

double HexTagModifierTable::get_value(const StringName &p_tag, const StringName &p_stat, double p_default) const {
	if (!modifiers.has(p_tag)) {
		return p_default;
	}
	Dictionary stats = modifiers[p_tag];
	return stats.get(p_stat, p_default);
}

Dictionary HexTagModifierTable::get_stats_for_tag(const StringName &p_tag) const {
	if (modifiers.has(p_tag)) {
		return modifiers[p_tag];
	}
	return Dictionary();
}

Dictionary HexTagModifierTable::to_dict() const {
	Dictionary d;
	d[StringName("table_id")] = table_id;
	d[StringName("modifiers")] = modifiers;
	return d;
}

Ref<HexTagModifierTable> HexTagModifierTable::from_dict(const Dictionary &p_dict) {
	Ref<HexTagModifierTable> t;
	t.instantiate();
	t->set_table_id(p_dict.get(StringName("table_id"), StringName()));
	t->set_modifiers(p_dict.get(StringName("modifiers"), Dictionary()));
	return t;
}
