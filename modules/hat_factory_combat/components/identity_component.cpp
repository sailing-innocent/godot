#include "identity_component.h"

#include "core/object/class_db.h"

void IdentityComponent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_def_id", "def_id"), &IdentityComponent::set_def_id);
	ClassDB::bind_method(D_METHOD("set_display_name", "display_name"), &IdentityComponent::set_display_name);
	ClassDB::bind_method(D_METHOD("set_sub_name", "sub_name"), &IdentityComponent::set_sub_name);
	ClassDB::bind_method(D_METHOD("get_def_id"), &IdentityComponent::get_def_id);
	ClassDB::bind_method(D_METHOD("get_display_name"), &IdentityComponent::get_display_name);
	ClassDB::bind_method(D_METHOD("get_sub_name"), &IdentityComponent::get_sub_name);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "def_id"), "set_def_id", "get_def_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_name"), "set_display_name", "get_display_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "sub_name"), "set_sub_name", "get_sub_name");
}

Dictionary IdentityComponent::to_dict() const {
	Dictionary d;
	d["def_id"] = def_id;
	d["display_name"] = display_name;
	d["sub_name"] = sub_name;
	return d;
}

void IdentityComponent::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("def_id")) { def_id = p_dict["def_id"]; }
	if (p_dict.has("display_name")) { display_name = p_dict["display_name"]; }
	if (p_dict.has("sub_name")) { sub_name = p_dict["sub_name"]; }
}

Ref<CombatComponent> IdentityComponent::clone() const {
	Ref<IdentityComponent> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
