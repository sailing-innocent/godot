#include "item_instance.h"

#include "enchantment.h"

#include "core/object/class_db.h"

void ItemInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_item_id", "item_id"), &ItemInstance::set_item_id);
	ClassDB::bind_method(D_METHOD("get_item_id"), &ItemInstance::get_item_id);

	ClassDB::bind_method(D_METHOD("set_quantity", "quantity"), &ItemInstance::set_quantity);
	ClassDB::bind_method(D_METHOD("get_quantity"), &ItemInstance::get_quantity);

	ClassDB::bind_method(D_METHOD("set_durability", "durability"), &ItemInstance::set_durability);
	ClassDB::bind_method(D_METHOD("get_durability"), &ItemInstance::get_durability);

	ClassDB::bind_method(D_METHOD("set_max_durability", "max_durability"), &ItemInstance::set_max_durability);
	ClassDB::bind_method(D_METHOD("get_max_durability"), &ItemInstance::get_max_durability);

	ClassDB::bind_method(D_METHOD("set_affix_seed", "affix_seed"), &ItemInstance::set_affix_seed);
	ClassDB::bind_method(D_METHOD("get_affix_seed"), &ItemInstance::get_affix_seed);

	ClassDB::bind_method(D_METHOD("set_enchantments", "enchantments"), &ItemInstance::set_enchantments);
	ClassDB::bind_method(D_METHOD("get_enchantments"), &ItemInstance::get_enchantments);

	ClassDB::bind_method(D_METHOD("to_dict"), &ItemInstance::to_dict);
	ClassDB::bind_static_method("ItemInstance", D_METHOD("from_dict", "dict"), &ItemInstance::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &ItemInstance::clone);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "item_id"), "set_item_id", "get_item_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "quantity"), "set_quantity", "get_quantity");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "durability"), "set_durability", "get_durability");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_durability"), "set_max_durability", "get_max_durability");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "affix_seed"), "set_affix_seed", "get_affix_seed");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "enchantments", PROPERTY_HINT_ARRAY_TYPE, "Enchantment"), "set_enchantments", "get_enchantments");
}

Dictionary ItemInstance::to_dict() const {
	Dictionary d;
	d["item_id"] = item_id;
	d["quantity"] = quantity;
	d["durability"] = durability;
	d["max_durability"] = max_durability;
	d["affix_seed"] = affix_seed;

	Array ench_array;
	for (int i = 0; i < enchantments.size(); ++i) {
		Ref<Enchantment> ench = enchantments[i];
		if (ench.is_valid()) {
			ench_array.append(ench->to_dict());
		} else {
			ench_array.append(Dictionary());
		}
	}
	d["enchantments"] = ench_array;

	return d;
}

Ref<ItemInstance> ItemInstance::from_dict(const Dictionary &p_dict) {
	Ref<ItemInstance> inst;
	inst.instantiate();

	if (p_dict.has("item_id")) {
		inst->item_id = p_dict["item_id"];
	}
	if (p_dict.has("quantity")) {
		inst->quantity = p_dict["quantity"];
	}
	if (p_dict.has("durability")) {
		inst->durability = p_dict["durability"];
	}
	if (p_dict.has("max_durability")) {
		inst->max_durability = p_dict["max_durability"];
	}
	if (p_dict.has("affix_seed")) {
		inst->affix_seed = p_dict["affix_seed"];
	}
	if (p_dict.has("enchantments")) {
		Array ench_array = p_dict["enchantments"];
		inst->enchantments.clear();
		for (int i = 0; i < ench_array.size(); ++i) {
			Ref<Enchantment> ench;
			ench.instantiate();
			ench->from_dict(ench_array[i]);
			inst->enchantments.append(ench);
		}
	}

	return inst;
}

Ref<ItemInstance> ItemInstance::clone() const {
	return from_dict(to_dict());
}
