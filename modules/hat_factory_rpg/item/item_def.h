#ifndef HAT_FACTORY_RPG_ITEM_DEF_H
#define HAT_FACTORY_RPG_ITEM_DEF_H

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "core/variant/type_info.h"

#include "modules/hat_factory_rpg/item/item_effect.h"
#include "modules/hat_factory_rpg/equipment/affix_def.h"

enum ItemCategory {
	MATERIAL = 0,
	CONSUMABLE = 1,
	EQUIPMENT = 2,
	KEY = 3,
	QUEST = 4
};

enum EquipmentSlot {
	NONE = 0,
	WEAPON = 1,
	HELMET = 2,
	ARMOR = 3,
	BOOTS = 4,
	ACCESSORY = 5
};

enum ItemRarity {
	COMMON = 0,
	UNCOMMON = 1,
	RARE = 2,
	EPIC = 3,
	LEGENDARY = 4
};

class ItemDef : public Resource {
	GDCLASS(ItemDef, Resource)

	StringName item_id;
	String display_name;
	String description;
	int category = MATERIAL;
	int rarity = COMMON;
	bool stackable = true;
	int max_stack = 99;
	int base_value = 0;
	bool tradeable = true;
	int equipment_slot = NONE;
	TypedArray<ItemEffect> effects;
	TypedArray<AffixDef> possible_affixes;

protected:
	static void _bind_methods();

public:
	void set_item_id(const StringName &p_id) { item_id = p_id; }
	StringName get_item_id() const { return item_id; }

	void set_display_name(const String &p_name) { display_name = p_name; }
	String get_display_name() const { return display_name; }

	void set_description(const String &p_desc) { description = p_desc; }
	String get_description() const { return description; }

	void set_category(int p_category) { category = p_category; }
	int get_category() const { return category; }

	void set_rarity(int p_rarity) { rarity = p_rarity; }
	int get_rarity() const { return rarity; }

	void set_stackable(bool p_stackable) { stackable = p_stackable; }
	bool is_stackable() const { return stackable; }

	void set_max_stack(int p_max) { max_stack = p_max; }
	int get_max_stack() const { return max_stack; }

	void set_base_value(int p_value) { base_value = p_value; }
	int get_base_value() const { return base_value; }

	void set_tradeable(bool p_tradeable) { tradeable = p_tradeable; }
	bool is_tradeable() const { return tradeable; }

	void set_equipment_slot(int p_slot) { equipment_slot = p_slot; }
	int get_equipment_slot() const { return equipment_slot; }

	void set_effects(const TypedArray<ItemEffect> &p_effects) { effects = p_effects; }
	TypedArray<ItemEffect> get_effects() const { return effects; }

	void set_possible_affixes(const TypedArray<AffixDef> &p_affixes) { possible_affixes = p_affixes; }
	TypedArray<AffixDef> get_possible_affixes() const { return possible_affixes; }

	Dictionary to_dict() const;
	void from_dict(const Dictionary &p_dict);
	Ref<ItemDef> clone() const;
};

VARIANT_ENUM_CAST(ItemCategory);
VARIANT_ENUM_CAST(EquipmentSlot);
VARIANT_ENUM_CAST(ItemRarity);

#endif // HAT_FACTORY_RPG_ITEM_DEF_H
