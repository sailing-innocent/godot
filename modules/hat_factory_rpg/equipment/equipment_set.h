#ifndef HAT_FACTORY_RPG_EQUIPMENT_SET_H
#define HAT_FACTORY_RPG_EQUIPMENT_SET_H

#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

class ItemInstance;
class CombatAttrComponent;

class EquipmentSet : public RefCounted {
	GDCLASS(EquipmentSet, RefCounted)

	Dictionary equipped; // EquipmentSlot (int) -> ItemInstance

protected:
	static void _bind_methods();

public:
	bool equip(const Ref<ItemInstance> &p_item);
	Ref<ItemInstance> unequip(int p_slot);
	Ref<ItemInstance> get_equipped(int p_slot) const;
	void apply_to(CombatAttrComponent *p_attr) const;

	Dictionary to_dict() const;
	static Ref<EquipmentSet> from_dict(const Dictionary &p_dict);
	Ref<EquipmentSet> clone() const;
};

#endif // HAT_FACTORY_RPG_EQUIPMENT_SET_H
