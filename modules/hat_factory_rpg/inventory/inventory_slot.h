#ifndef HAT_FACTORY_RPG_INVENTORY_SLOT_H
#define HAT_FACTORY_RPG_INVENTORY_SLOT_H

#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"

#include "modules/hat_factory_rpg/item/item_instance.h"

class InventorySlot : public RefCounted {
	GDCLASS(InventorySlot, RefCounted)

	int index = 0;
	Ref<ItemInstance> item;
	bool locked = false;

protected:
	static void _bind_methods();

public:
	void set_index(int p_index) { index = p_index; }
	int get_index() const { return index; }

	void set_item(const Ref<ItemInstance> &p_item) { item = p_item; }
	Ref<ItemInstance> get_item() const { return item; }

	void set_locked(bool p_locked) { locked = p_locked; }
	bool is_locked() const { return locked; }

	bool is_empty() const { return item.is_null(); }

	Dictionary to_dict() const;
	static Ref<InventorySlot> from_dict(const Dictionary &p_dict);
	Ref<InventorySlot> clone() const;
};

#endif // HAT_FACTORY_RPG_INVENTORY_SLOT_H
