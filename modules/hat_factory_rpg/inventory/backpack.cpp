#include "backpack.h"

#include "inventory_slot.h"
#include "item/item_def.h"
#include "item/item_instance.h"
#include "item/item_effect.h"

#include "modules/hat_factory_combat/state/combat_entity.h"

#include "core/io/resource_loader.h"
#include "core/object/class_db.h"
#include "core/string/ustring.h"

static Ref<ItemDef> _lookup_item_def(const StringName &p_item_id) {
	String path = "res://resources/items/" + String(p_item_id) + ".tres";
	Ref<Resource> res = ResourceLoader::load(path);
	if (res.is_valid()) {
		Ref<ItemDef> def = Object::cast_to<ItemDef>(res.ptr());
		if (def.is_valid()) {
			return def;
		}
	}
	return Ref<ItemDef>();
}

void Backpack::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_capacity", "capacity"), &Backpack::set_capacity);
	ClassDB::bind_method(D_METHOD("get_capacity"), &Backpack::get_capacity);

	ClassDB::bind_method(D_METHOD("set_slots", "slots"), &Backpack::set_slots);
	ClassDB::bind_method(D_METHOD("get_slots"), &Backpack::get_slots);

	ClassDB::bind_method(D_METHOD("add", "item"), &Backpack::add);
	ClassDB::bind_method(D_METHOD("remove", "item_id", "count"), &Backpack::remove, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("move", "from_slot", "to_slot"), &Backpack::move);
	ClassDB::bind_method(D_METHOD("split", "slot", "count"), &Backpack::split);
	ClassDB::bind_method(D_METHOD("merge", "from_slot", "to_slot"), &Backpack::merge);
	ClassDB::bind_method(D_METHOD("use", "slot", "target"), &Backpack::use, DEFVAL(Ref<CombatEntity>()));
	ClassDB::bind_method(D_METHOD("count", "item_id"), &Backpack::count);
	ClassDB::bind_method(D_METHOD("find_by_category", "category"), &Backpack::find_by_category);

	ClassDB::bind_method(D_METHOD("to_dict"), &Backpack::to_dict);
	ClassDB::bind_static_method("Backpack", D_METHOD("from_dict", "dict"), &Backpack::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &Backpack::clone);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "capacity"), "set_capacity", "get_capacity");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "slots", PROPERTY_HINT_ARRAY_TYPE, "InventorySlot"), "set_slots", "get_slots");

	ADD_SIGNAL(MethodInfo("changed"));
}

void Backpack::_ensure_slots() {
	while (slots.size() < capacity) {
		Ref<InventorySlot> slot;
		slot.instantiate();
		slot->set_index(slots.size());
		slots.append(slot);
	}
	if (slots.size() > capacity) {
		slots.resize(capacity);
	}
}

Ref<InventorySlot> Backpack::_get_slot(int p_index) const {
	if (p_index < 0 || p_index >= slots.size()) {
		return Ref<InventorySlot>();
	}
	return slots[p_index];
}

void Backpack::set_capacity(int p_capacity) {
	if (p_capacity < 0) {
		p_capacity = 0;
	}
	capacity = p_capacity;
	_ensure_slots();
	emit_signal("changed");
}

void Backpack::set_slots(const TypedArray<InventorySlot> &p_slots) {
	slots = p_slots;
	for (int i = 0; i < slots.size(); ++i) {
		Ref<InventorySlot> slot = slots[i];
		if (slot.is_valid()) {
			slot->set_index(i);
		}
	}
	emit_signal("changed");
}

bool Backpack::add(const Ref<ItemInstance> &p_item) {
	if (p_item.is_null() || p_item->get_quantity() <= 0 || p_item->get_item_id() == StringName()) {
		return false;
	}

	_ensure_slots();

	Ref<ItemDef> def = _lookup_item_def(p_item->get_item_id());
	const bool stackable = def.is_valid() ? def->is_stackable() : true;
	const int max_stack = def.is_valid() ? def->get_max_stack() : 99;

	int remaining = p_item->get_quantity();
	bool any_added = false;

	if (stackable) {
		for (int i = 0; i < slots.size() && remaining > 0; ++i) {
			Ref<InventorySlot> slot = slots[i];
			if (slot.is_null() || slot->is_locked() || slot->is_empty()) {
				continue;
			}
			Ref<ItemInstance> existing = slot->get_item();
			if (existing.is_null() || existing->get_item_id() != p_item->get_item_id()) {
				continue;
			}
			const int room = max_stack - existing->get_quantity();
			if (room <= 0) {
				continue;
			}
			const int add_qty = MIN(room, remaining);
			existing->set_quantity(existing->get_quantity() + add_qty);
			remaining -= add_qty;
			any_added = true;
		}
	}

	for (int i = 0; i < slots.size() && remaining > 0; ++i) {
		Ref<InventorySlot> slot = slots[i];
		if (slot.is_null() || slot->is_locked() || !slot->is_empty()) {
			continue;
		}
		const int add_qty = stackable ? MIN(max_stack, remaining) : 1;
		Ref<ItemInstance> placed = p_item->clone();
		placed->set_quantity(add_qty);
		slot->set_item(placed);
		remaining -= add_qty;
		any_added = true;
	}

	if (any_added) {
		emit_signal("changed");
	}
	return remaining == 0;
}

bool Backpack::remove(const StringName &p_item_id, int p_count) {
	if (p_count <= 0 || p_item_id == StringName()) {
		return false;
	}

	_ensure_slots();

	int remaining = p_count;
	bool any_removed = false;

	for (int i = 0; i < slots.size() && remaining > 0; ++i) {
		Ref<InventorySlot> slot = slots[i];
		if (slot.is_null() || slot->is_locked() || slot->is_empty()) {
			continue;
		}
		Ref<ItemInstance> item = slot->get_item();
		if (item.is_null() || item->get_item_id() != p_item_id) {
			continue;
		}

		if (item->get_quantity() <= remaining) {
			remaining -= item->get_quantity();
			slot->set_item(Ref<ItemInstance>());
			any_removed = true;
		} else {
			item->set_quantity(item->get_quantity() - remaining);
			remaining = 0;
			any_removed = true;
		}
	}

	if (any_removed) {
		emit_signal("changed");
	}
	return remaining == 0;
}

bool Backpack::move(int p_from_slot, int p_to_slot) {
	_ensure_slots();
	if (p_from_slot < 0 || p_from_slot >= slots.size() || p_to_slot < 0 || p_to_slot >= slots.size()) {
		return false;
	}
	if (p_from_slot == p_to_slot) {
		return false;
	}

	Ref<InventorySlot> from = slots[p_from_slot];
	Ref<InventorySlot> to = slots[p_to_slot];
	if (from.is_null() || from->is_locked() || from->is_empty() || to.is_null() || to->is_locked()) {
		return false;
	}

	Ref<ItemInstance> from_item = from->get_item();
	if (to->is_empty()) {
		to->set_item(from_item);
		from->set_item(Ref<ItemInstance>());
		emit_signal("changed");
		return true;
	}

	Ref<ItemInstance> to_item = to->get_item();
	if (from_item->get_item_id() == to_item->get_item_id()) {
		Ref<ItemDef> def = _lookup_item_def(from_item->get_item_id());
		const int max_stack = def.is_valid() ? def->get_max_stack() : 99;
		const int room = max_stack - to_item->get_quantity();
		if (room > 0) {
			const int move_qty = MIN(room, from_item->get_quantity());
			to_item->set_quantity(to_item->get_quantity() + move_qty);
			from_item->set_quantity(from_item->get_quantity() - move_qty);
			if (from_item->get_quantity() <= 0) {
				from->set_item(Ref<ItemInstance>());
			}
			emit_signal("changed");
			return true;
		}
	}

	// Different item or no room: swap.
	to->set_item(from_item);
	from->set_item(to_item);
	emit_signal("changed");
	return true;
}

bool Backpack::split(int p_slot, int p_count) {
	_ensure_slots();
	if (p_slot < 0 || p_slot >= slots.size() || p_count <= 0) {
		return false;
	}

	Ref<InventorySlot> source = slots[p_slot];
	if (source.is_null() || source->is_locked() || source->is_empty()) {
		return false;
	}
	Ref<ItemInstance> item = source->get_item();
	if (item->get_quantity() <= p_count) {
		return false;
	}

	int target_idx = -1;
	for (int i = 0; i < slots.size(); ++i) {
		Ref<InventorySlot> slot = slots[i];
		if (i != p_slot && slot.is_valid() && !slot->is_locked() && slot->is_empty()) {
			target_idx = i;
			break;
		}
	}
	if (target_idx < 0) {
		return false;
	}

	Ref<ItemInstance> split_item = item->clone();
	split_item->set_quantity(p_count);
	item->set_quantity(item->get_quantity() - p_count);

	Ref<InventorySlot> target = slots[target_idx];
	target->set_item(split_item);

	emit_signal("changed");
	return true;
}

bool Backpack::merge(int p_from_slot, int p_to_slot) {
	_ensure_slots();
	if (p_from_slot < 0 || p_from_slot >= slots.size() || p_to_slot < 0 || p_to_slot >= slots.size()) {
		return false;
	}
	if (p_from_slot == p_to_slot) {
		return false;
	}

	Ref<InventorySlot> from = slots[p_from_slot];
	Ref<InventorySlot> to = slots[p_to_slot];
	if (from.is_null() || from->is_locked() || from->is_empty() || to.is_null() || to->is_locked() || to->is_empty()) {
		return false;
	}

	Ref<ItemInstance> from_item = from->get_item();
	Ref<ItemInstance> to_item = to->get_item();
	if (from_item->get_item_id() != to_item->get_item_id()) {
		return false;
	}

	Ref<ItemDef> def = _lookup_item_def(from_item->get_item_id());
	const int max_stack = def.is_valid() ? def->get_max_stack() : 99;

	const int room = max_stack - to_item->get_quantity();
	if (room <= 0) {
		return false;
	}

	const int move_qty = MIN(room, from_item->get_quantity());
	to_item->set_quantity(to_item->get_quantity() + move_qty);
	from_item->set_quantity(from_item->get_quantity() - move_qty);
	if (from_item->get_quantity() <= 0) {
		from->set_item(Ref<ItemInstance>());
	}

	emit_signal("changed");
	return true;
}

Ref<ItemInstance> Backpack::use(int p_slot, const Ref<CombatEntity> &p_target) {
	_ensure_slots();
	if (p_slot < 0 || p_slot >= slots.size()) {
		return Ref<ItemInstance>();
	}

	Ref<InventorySlot> slot = slots[p_slot];
	if (slot.is_null() || slot->is_empty()) {
		return Ref<ItemInstance>();
	}

	Ref<ItemInstance> item = slot->get_item();
	if (item->get_quantity() <= 0) {
		return Ref<ItemInstance>();
	}

	Ref<ItemDef> def = _lookup_item_def(item->get_item_id());
	if (def.is_valid()) {
		TypedArray<ItemEffect> effects = def->get_effects();
		for (int i = 0; i < effects.size(); ++i) {
			Ref<ItemEffect> eff = effects[i];
			if (eff.is_valid()) {
				eff->apply(p_target);
			}
		}
	}

	Ref<ItemInstance> used = item->clone();
	used->set_quantity(1);

	if (item->get_quantity() == 1) {
		slot->set_item(Ref<ItemInstance>());
	} else {
		item->set_quantity(item->get_quantity() - 1);
	}

	emit_signal("changed");
	return used;
}

int Backpack::count(const StringName &p_item_id) const {
	int total = 0;
	for (int i = 0; i < slots.size(); ++i) {
		Ref<InventorySlot> slot = slots[i];
		if (slot.is_null() || slot->is_empty()) {
			continue;
		}
		Ref<ItemInstance> item = slot->get_item();
		if (item.is_valid() && item->get_item_id() == p_item_id) {
			total += item->get_quantity();
		}
	}
	return total;
}

TypedArray<InventorySlot> Backpack::find_by_category(int p_category) const {
	TypedArray<InventorySlot> result;
	for (int i = 0; i < slots.size(); ++i) {
		Ref<InventorySlot> slot = slots[i];
		if (slot.is_null() || slot->is_empty()) {
			continue;
		}
		Ref<ItemInstance> item = slot->get_item();
		if (item.is_null()) {
			continue;
		}
		Ref<ItemDef> def = _lookup_item_def(item->get_item_id());
		if (def.is_valid() && def->get_category() == p_category) {
			result.append(slot);
		}
	}
	return result;
}

Dictionary Backpack::to_dict() const {
	Dictionary d;
	d["capacity"] = capacity;
	Array slots_array;
	for (int i = 0; i < slots.size(); ++i) {
		Ref<InventorySlot> slot = slots[i];
		if (slot.is_valid()) {
			slots_array.append(slot->to_dict());
		} else {
			slots_array.append(Dictionary());
		}
	}
	d["slots"] = slots_array;
	return d;
}

Ref<Backpack> Backpack::from_dict(const Dictionary &p_dict) {
	Ref<Backpack> pack;
	pack.instantiate();

	if (p_dict.has("capacity")) {
		pack->capacity = p_dict["capacity"];
	}

	pack->slots.clear();
	if (p_dict.has("slots")) {
		Array slots_array = p_dict["slots"];
		for (int i = 0; i < slots_array.size(); ++i) {
			Ref<InventorySlot> slot = InventorySlot::from_dict(slots_array[i]);
			if (slot.is_valid()) {
				slot->set_index(i);
			}
			pack->slots.append(slot);
		}
	}
	pack->_ensure_slots();

	return pack;
}

Ref<Backpack> Backpack::clone() const {
	return from_dict(to_dict());
}
