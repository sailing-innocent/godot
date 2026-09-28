#include "inventory_slot.h"

#include "item/item_instance.h"

#include "core/object/class_db.h"

void InventorySlot::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_index", "index"), &InventorySlot::set_index);
	ClassDB::bind_method(D_METHOD("get_index"), &InventorySlot::get_index);

	ClassDB::bind_method(D_METHOD("set_item", "item"), &InventorySlot::set_item);
	ClassDB::bind_method(D_METHOD("get_item"), &InventorySlot::get_item);

	ClassDB::bind_method(D_METHOD("set_locked", "locked"), &InventorySlot::set_locked);
	ClassDB::bind_method(D_METHOD("is_locked"), &InventorySlot::is_locked);

	ClassDB::bind_method(D_METHOD("is_empty"), &InventorySlot::is_empty);

	ClassDB::bind_method(D_METHOD("to_dict"), &InventorySlot::to_dict);
	ClassDB::bind_static_method("InventorySlot", D_METHOD("from_dict", "dict"), &InventorySlot::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &InventorySlot::clone);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "index"), "set_index", "get_index");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "item", PROPERTY_HINT_RESOURCE_TYPE, "ItemInstance"), "set_item", "get_item");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "locked"), "set_locked", "is_locked");
}

Dictionary InventorySlot::to_dict() const {
	Dictionary d;
	d["index"] = index;
	d["locked"] = locked;
	if (item.is_valid()) {
		d["item"] = item->to_dict();
	} else {
		d["item"] = Dictionary();
	}
	return d;
}

Ref<InventorySlot> InventorySlot::from_dict(const Dictionary &p_dict) {
	Ref<InventorySlot> slot;
	slot.instantiate();

	if (p_dict.has("index")) {
		slot->index = p_dict["index"];
	}
	if (p_dict.has("locked")) {
		slot->locked = p_dict["locked"];
	}
	if (p_dict.has("item")) {
		Variant item_var = p_dict["item"];
		if (item_var.get_type() == Variant::DICTIONARY) {
			Dictionary item_dict = item_var;
			if (!item_dict.is_empty()) {
				slot->item = ItemInstance::from_dict(item_dict);
			}
		}
	}

	return slot;
}

Ref<InventorySlot> InventorySlot::clone() const {
	Ref<InventorySlot> copy;
	copy.instantiate();
	copy->index = index;
	copy->locked = locked;
	if (item.is_valid()) {
		copy->item = item->clone();
	}
	return copy;
}
