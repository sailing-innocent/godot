#include "item_def.h"

#include "item_effect.h"
#include "equipment/affix_def.h"

#include "core/object/class_db.h"

void ItemDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_item_id", "item_id"), &ItemDef::set_item_id);
	ClassDB::bind_method(D_METHOD("get_item_id"), &ItemDef::get_item_id);

	ClassDB::bind_method(D_METHOD("set_display_name", "display_name"), &ItemDef::set_display_name);
	ClassDB::bind_method(D_METHOD("get_display_name"), &ItemDef::get_display_name);

	ClassDB::bind_method(D_METHOD("set_description", "description"), &ItemDef::set_description);
	ClassDB::bind_method(D_METHOD("get_description"), &ItemDef::get_description);

	ClassDB::bind_method(D_METHOD("set_category", "category"), &ItemDef::set_category);
	ClassDB::bind_method(D_METHOD("get_category"), &ItemDef::get_category);

	ClassDB::bind_method(D_METHOD("set_rarity", "rarity"), &ItemDef::set_rarity);
	ClassDB::bind_method(D_METHOD("get_rarity"), &ItemDef::get_rarity);

	ClassDB::bind_method(D_METHOD("set_stackable", "stackable"), &ItemDef::set_stackable);
	ClassDB::bind_method(D_METHOD("is_stackable"), &ItemDef::is_stackable);

	ClassDB::bind_method(D_METHOD("set_max_stack", "max_stack"), &ItemDef::set_max_stack);
	ClassDB::bind_method(D_METHOD("get_max_stack"), &ItemDef::get_max_stack);

	ClassDB::bind_method(D_METHOD("set_base_value", "base_value"), &ItemDef::set_base_value);
	ClassDB::bind_method(D_METHOD("get_base_value"), &ItemDef::get_base_value);

	ClassDB::bind_method(D_METHOD("set_tradeable", "tradeable"), &ItemDef::set_tradeable);
	ClassDB::bind_method(D_METHOD("is_tradeable"), &ItemDef::is_tradeable);

	ClassDB::bind_method(D_METHOD("set_equipment_slot", "equipment_slot"), &ItemDef::set_equipment_slot);
	ClassDB::bind_method(D_METHOD("get_equipment_slot"), &ItemDef::get_equipment_slot);

	ClassDB::bind_method(D_METHOD("set_effects", "effects"), &ItemDef::set_effects);
	ClassDB::bind_method(D_METHOD("get_effects"), &ItemDef::get_effects);

	ClassDB::bind_method(D_METHOD("set_possible_affixes", "possible_affixes"), &ItemDef::set_possible_affixes);
	ClassDB::bind_method(D_METHOD("get_possible_affixes"), &ItemDef::get_possible_affixes);

	ClassDB::bind_method(D_METHOD("to_dict"), &ItemDef::to_dict);
	ClassDB::bind_method(D_METHOD("from_dict", "dict"), &ItemDef::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &ItemDef::clone);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "item_id"), "set_item_id", "get_item_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_name"), "set_display_name", "get_display_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description"), "set_description", "get_description");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "category"), "set_category", "get_category");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rarity"), "set_rarity", "get_rarity");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "stackable"), "set_stackable", "is_stackable");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_stack"), "set_max_stack", "get_max_stack");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "base_value"), "set_base_value", "get_base_value");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "tradeable"), "set_tradeable", "is_tradeable");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "equipment_slot"), "set_equipment_slot", "get_equipment_slot");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "effects", PROPERTY_HINT_ARRAY_TYPE, "ItemEffect"), "set_effects", "get_effects");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "possible_affixes", PROPERTY_HINT_ARRAY_TYPE, "AffixDef"), "set_possible_affixes", "get_possible_affixes");

	BIND_ENUM_CONSTANT(MATERIAL);
	BIND_ENUM_CONSTANT(CONSUMABLE);
	BIND_ENUM_CONSTANT(EQUIPMENT);
	BIND_ENUM_CONSTANT(KEY);
	BIND_ENUM_CONSTANT(QUEST);

	BIND_ENUM_CONSTANT(NONE);
	BIND_ENUM_CONSTANT(WEAPON);
	BIND_ENUM_CONSTANT(HELMET);
	BIND_ENUM_CONSTANT(ARMOR);
	BIND_ENUM_CONSTANT(BOOTS);
	BIND_ENUM_CONSTANT(ACCESSORY);

	BIND_ENUM_CONSTANT(COMMON);
	BIND_ENUM_CONSTANT(UNCOMMON);
	BIND_ENUM_CONSTANT(RARE);
	BIND_ENUM_CONSTANT(EPIC);
	BIND_ENUM_CONSTANT(LEGENDARY);
}

Dictionary ItemDef::to_dict() const {
	Dictionary d;
	d["item_id"] = item_id;
	d["display_name"] = display_name;
	d["description"] = description;
	d["category"] = category;
	d["rarity"] = rarity;
	d["stackable"] = stackable;
	d["max_stack"] = max_stack;
	d["base_value"] = base_value;
	d["tradeable"] = tradeable;
	d["equipment_slot"] = equipment_slot;

	Array effects_array;
	for (int i = 0; i < effects.size(); ++i) {
		Ref<ItemEffect> eff = effects[i];
		if (eff.is_valid()) {
			effects_array.append(eff->to_dict());
		} else {
			effects_array.append(Dictionary());
		}
	}
	d["effects"] = effects_array;

	Array affix_array;
	for (int i = 0; i < possible_affixes.size(); ++i) {
		Ref<AffixDef> aff = possible_affixes[i];
		if (aff.is_valid()) {
			affix_array.append(aff->to_dict());
		} else {
			affix_array.append(Dictionary());
		}
	}
	d["possible_affixes"] = affix_array;

	return d;
}

void ItemDef::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("item_id")) {
		item_id = p_dict["item_id"];
	}
	if (p_dict.has("display_name")) {
		display_name = p_dict["display_name"];
	}
	if (p_dict.has("description")) {
		description = p_dict["description"];
	}
	if (p_dict.has("category")) {
		category = p_dict["category"];
	}
	if (p_dict.has("rarity")) {
		rarity = p_dict["rarity"];
	}
	if (p_dict.has("stackable")) {
		stackable = p_dict["stackable"];
	}
	if (p_dict.has("max_stack")) {
		max_stack = p_dict["max_stack"];
	}
	if (p_dict.has("base_value")) {
		base_value = p_dict["base_value"];
	}
	if (p_dict.has("tradeable")) {
		tradeable = p_dict["tradeable"];
	}
	if (p_dict.has("equipment_slot")) {
		equipment_slot = p_dict["equipment_slot"];
	}

	if (p_dict.has("effects")) {
		Array effects_array = p_dict["effects"];
		effects.clear();
		for (int i = 0; i < effects_array.size(); ++i) {
			Ref<ItemEffect> eff;
			eff.instantiate();
			eff->from_dict(effects_array[i]);
			effects.append(eff);
		}
	}

	if (p_dict.has("possible_affixes")) {
		Array affix_array = p_dict["possible_affixes"];
		possible_affixes.clear();
		for (int i = 0; i < affix_array.size(); ++i) {
			Ref<AffixDef> aff;
			aff.instantiate();
			aff->from_dict(affix_array[i]);
			possible_affixes.append(aff);
		}
	}
}

Ref<ItemDef> ItemDef::clone() const {
	Ref<ItemDef> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
