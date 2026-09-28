#ifndef HAT_FACTORY_RPG_ITEM_INSTANCE_H
#define HAT_FACTORY_RPG_ITEM_INSTANCE_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

#include "modules/hat_factory_rpg/item/enchantment.h"

class ItemInstance : public RefCounted {
	GDCLASS(ItemInstance, RefCounted)

	StringName item_id;
	int quantity = 1;
	int durability = 0;
	int max_durability = 0;
	int affix_seed = 0;
	TypedArray<Enchantment> enchantments;

protected:
	static void _bind_methods();

public:
	void set_item_id(const StringName &p_id) { item_id = p_id; }
	StringName get_item_id() const { return item_id; }

	void set_quantity(int p_quantity) { quantity = p_quantity; }
	int get_quantity() const { return quantity; }

	void set_durability(int p_durability) { durability = p_durability; }
	int get_durability() const { return durability; }

	void set_max_durability(int p_max) { max_durability = p_max; }
	int get_max_durability() const { return max_durability; }

	void set_affix_seed(int p_seed) { affix_seed = p_seed; }
	int get_affix_seed() const { return affix_seed; }

	void set_enchantments(const TypedArray<Enchantment> &p_enchantments) { enchantments = p_enchantments; }
	TypedArray<Enchantment> get_enchantments() const { return enchantments; }

	Dictionary to_dict() const;
	static Ref<ItemInstance> from_dict(const Dictionary &p_dict);
	Ref<ItemInstance> clone() const;
};

#endif // HAT_FACTORY_RPG_ITEM_INSTANCE_H
