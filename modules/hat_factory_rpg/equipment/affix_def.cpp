#include "affix_def.h"

#include "core/object/class_db.h"

void AffixDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_affix_id", "affix_id"), &AffixDef::set_affix_id);
	ClassDB::bind_method(D_METHOD("get_affix_id"), &AffixDef::get_affix_id);

	ClassDB::bind_method(D_METHOD("set_display_name", "display_name"), &AffixDef::set_display_name);
	ClassDB::bind_method(D_METHOD("get_display_name"), &AffixDef::get_display_name);

	ClassDB::bind_method(D_METHOD("set_affected_attr", "affected_attr"), &AffixDef::set_affected_attr);
	ClassDB::bind_method(D_METHOD("get_affected_attr"), &AffixDef::get_affected_attr);

	ClassDB::bind_method(D_METHOD("set_min_value", "min_value"), &AffixDef::set_min_value);
	ClassDB::bind_method(D_METHOD("get_min_value"), &AffixDef::get_min_value);

	ClassDB::bind_method(D_METHOD("set_max_value", "max_value"), &AffixDef::set_max_value);
	ClassDB::bind_method(D_METHOD("get_max_value"), &AffixDef::get_max_value);

	ClassDB::bind_method(D_METHOD("set_weight", "weight"), &AffixDef::set_weight);
	ClassDB::bind_method(D_METHOD("get_weight"), &AffixDef::get_weight);

	ClassDB::bind_method(D_METHOD("to_dict"), &AffixDef::to_dict);
	ClassDB::bind_method(D_METHOD("from_dict", "dict"), &AffixDef::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &AffixDef::clone);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "affix_id"), "set_affix_id", "get_affix_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_name"), "set_display_name", "get_display_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "affected_attr"), "set_affected_attr", "get_affected_attr");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "min_value"), "set_min_value", "get_min_value");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_value"), "set_max_value", "get_max_value");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "weight"), "set_weight", "get_weight");
}

Dictionary AffixDef::to_dict() const {
	Dictionary d;
	d["affix_id"] = affix_id;
	d["display_name"] = display_name;
	d["affected_attr"] = affected_attr;
	d["min_value"] = min_value;
	d["max_value"] = max_value;
	d["weight"] = weight;
	return d;
}

void AffixDef::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("affix_id")) {
		affix_id = p_dict["affix_id"];
	}
	if (p_dict.has("display_name")) {
		display_name = p_dict["display_name"];
	}
	if (p_dict.has("affected_attr")) {
		affected_attr = p_dict["affected_attr"];
	}
	if (p_dict.has("min_value")) {
		min_value = p_dict["min_value"];
	}
	if (p_dict.has("max_value")) {
		max_value = p_dict["max_value"];
	}
	if (p_dict.has("weight")) {
		weight = p_dict["weight"];
	}
}

Ref<AffixDef> AffixDef::clone() const {
	Ref<AffixDef> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
