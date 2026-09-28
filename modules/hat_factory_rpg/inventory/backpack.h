#ifndef HAT_FACTORY_RPG_BACKPACK_H
#define HAT_FACTORY_RPG_BACKPACK_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/typed_array.h"

#include "modules/hat_factory_rpg/inventory/inventory_slot.h"

class ItemInstance;
class CombatEntity;

class Backpack : public RefCounted {
	GDCLASS(Backpack, RefCounted)

	int capacity = 60;
	TypedArray<InventorySlot> slots;

protected:
	static void _bind_methods();

	void _ensure_slots();
	Ref<InventorySlot> _get_slot(int p_index) const;

public:
	void set_capacity(int p_capacity);
	int get_capacity() const { return capacity; }

	void set_slots(const TypedArray<InventorySlot> &p_slots);
	TypedArray<InventorySlot> get_slots() const { return slots; }

	bool add(const Ref<ItemInstance> &p_item);
	bool remove(const StringName &p_item_id, int p_count = 1);
	bool move(int p_from_slot, int p_to_slot);
	bool split(int p_slot, int p_count);
	bool merge(int p_from_slot, int p_to_slot);
	Ref<ItemInstance> use(int p_slot, const Ref<CombatEntity> &p_target = nullptr);
	int count(const StringName &p_item_id) const;
	TypedArray<InventorySlot> find_by_category(int p_category) const;

	Dictionary to_dict() const;
	static Ref<Backpack> from_dict(const Dictionary &p_dict);
	Ref<Backpack> clone() const;
};

#endif // HAT_FACTORY_RPG_BACKPACK_H
