#include "equipment_set.h"

#include "item/item_def.h"
#include "item/item_instance.h"
#include "item/enchantment.h"

#include "modules/hat_factory_combat/components/combat_attr_component.h"

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

static void _apply_enchantment_to_attr(CombatAttrComponent *p_attr, const Ref<Enchantment> &p_enchant) {
	if (p_attr == nullptr || p_enchant.is_null()) {
		return;
	}

	const StringName attr = p_enchant->get_affected_attr();
	const int value = p_enchant->get_value();

	if (attr == StringName("attack")) {
		p_attr->set_attack(p_attr->get_attack() + value);
	} else if (attr == StringName("defense")) {
		p_attr->set_defense(p_attr->get_defense() + value);
	} else if (attr == StringName("magic")) {
		p_attr->set_magic(p_attr->get_magic() + value);
	} else if (attr == StringName("resistance")) {
		p_attr->set_resistance(p_attr->get_resistance() + value);
	} else if (attr == StringName("aim")) {
		p_attr->set_aim(p_attr->get_aim() + value);
	} else if (attr == StringName("evasion")) {
		p_attr->set_evasion(p_attr->get_evasion() + value);
	} else if (attr == StringName("crit_rate")) {
		p_attr->set_crit_rate(p_attr->get_crit_rate() + static_cast<float>(value));
	} else if (attr == StringName("crit_damage")) {
		p_attr->set_crit_damage(p_attr->get_crit_damage() + static_cast<float>(value));
	}
}

void EquipmentSet::_bind_methods() {
	ClassDB::bind_method(D_METHOD("equip", "item"), &EquipmentSet::equip);
	ClassDB::bind_method(D_METHOD("unequip", "slot"), &EquipmentSet::unequip);
	ClassDB::bind_method(D_METHOD("get_equipped", "slot"), &EquipmentSet::get_equipped);
	ClassDB::bind_method(D_METHOD("apply_to", "attr"), &EquipmentSet::apply_to);

	ClassDB::bind_method(D_METHOD("to_dict"), &EquipmentSet::to_dict);
	ClassDB::bind_static_method("EquipmentSet", D_METHOD("from_dict", "dict"), &EquipmentSet::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &EquipmentSet::clone);

	ADD_SIGNAL(MethodInfo("changed"));
}

bool EquipmentSet::equip(const Ref<ItemInstance> &p_item) {
	if (p_item.is_null()) {
		return false;
	}

	Ref<ItemDef> def = _lookup_item_def(p_item->get_item_id());
	if (def.is_null()) {
		return false;
	}

	const int slot = def->get_equipment_slot();
	if (slot == 0) {
		return false;
	}

	unequip(slot);
	equipped[slot] = p_item;

	emit_signal("changed");
	return true;
}

Ref<ItemInstance> EquipmentSet::unequip(int p_slot) {
	if (!equipped.has(p_slot)) {
		return Ref<ItemInstance>();
	}

	Ref<ItemInstance> item = equipped[p_slot];
	equipped.erase(p_slot);

	emit_signal("changed");
	return item;
}

Ref<ItemInstance> EquipmentSet::get_equipped(int p_slot) const {
	if (!equipped.has(p_slot)) {
		return Ref<ItemInstance>();
	}
	return equipped[p_slot];
}

void EquipmentSet::apply_to(CombatAttrComponent *p_attr) const {
	if (p_attr == nullptr) {
		return;
	}

	Array keys = equipped.keys();
	for (int i = 0; i < keys.size(); ++i) {
		Ref<ItemInstance> item = equipped[keys[i]];
		if (item.is_null()) {
			continue;
		}

		TypedArray<Enchantment> enchants = item->get_enchantments();
		for (int j = 0; j < enchants.size(); ++j) {
			_apply_enchantment_to_attr(p_attr, enchants[j]);
		}
	}
}

Dictionary EquipmentSet::to_dict() const {
	Dictionary d;
	Array keys = equipped.keys();
	for (int i = 0; i < keys.size(); ++i) {
		const int slot = keys[i];
		Ref<ItemInstance> item = equipped[slot];
		if (item.is_valid()) {
			d[slot] = item->to_dict();
		}
	}
	return d;
}

Ref<EquipmentSet> EquipmentSet::from_dict(const Dictionary &p_dict) {
	Ref<EquipmentSet> set;
	set.instantiate();

	Array keys = p_dict.keys();
	for (int i = 0; i < keys.size(); ++i) {
		const int slot = keys[i];
		Ref<ItemInstance> item = ItemInstance::from_dict(p_dict[slot]);
		if (item.is_valid()) {
			set->equipped[slot] = item;
		}
	}

	return set;
}

Ref<EquipmentSet> EquipmentSet::clone() const {
	return from_dict(to_dict());
}
