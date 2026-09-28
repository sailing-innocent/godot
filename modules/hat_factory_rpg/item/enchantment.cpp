#include "enchantment.h"

#include "core/object/class_db.h"

void Enchantment::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_affix_id", "affix_id"), &Enchantment::set_affix_id);
	ClassDB::bind_method(D_METHOD("get_affix_id"), &Enchantment::get_affix_id);

	ClassDB::bind_method(D_METHOD("set_value", "value"), &Enchantment::set_value);
	ClassDB::bind_method(D_METHOD("get_value"), &Enchantment::get_value);

	ClassDB::bind_method(D_METHOD("set_affected_attr", "affected_attr"), &Enchantment::set_affected_attr);
	ClassDB::bind_method(D_METHOD("get_affected_attr"), &Enchantment::get_affected_attr);

	ClassDB::bind_method(D_METHOD("to_dict"), &Enchantment::to_dict);
	ClassDB::bind_static_method("Enchantment", D_METHOD("from_dict", "dict"), &Enchantment::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &Enchantment::clone);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "affix_id"), "set_affix_id", "get_affix_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "value"), "set_value", "get_value");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "affected_attr"), "set_affected_attr", "get_affected_attr");
}

Dictionary Enchantment::to_dict() const {
	Dictionary d;
	d["affix_id"] = affix_id;
	d["value"] = value;
	d["affected_attr"] = affected_attr;
	return d;
}

Ref<Enchantment> Enchantment::from_dict(const Dictionary &p_dict) {
	Ref<Enchantment> ench;
	ench.instantiate();

	if (p_dict.has("affix_id")) {
		ench->affix_id = p_dict["affix_id"];
	}
	if (p_dict.has("value")) {
		ench->value = p_dict["value"];
	}
	if (p_dict.has("affected_attr")) {
		ench->affected_attr = p_dict["affected_attr"];
	}

	return ench;
}

Ref<Enchantment> Enchantment::clone() const {
	return from_dict(to_dict());
}
