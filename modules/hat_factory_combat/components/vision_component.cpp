#include "vision_component.h"

#include "core/object/class_db.h"

void VisionComponent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_range", "range"), &VisionComponent::set_range);
	ClassDB::bind_method(D_METHOD("set_height_bonus", "height_bonus"), &VisionComponent::set_height_bonus);
	ClassDB::bind_method(D_METHOD("get_range"), &VisionComponent::get_range);
	ClassDB::bind_method(D_METHOD("get_height_bonus"), &VisionComponent::get_height_bonus);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "range"), "set_range", "get_range");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height_bonus"), "set_height_bonus", "get_height_bonus");
}

Dictionary VisionComponent::to_dict() const {
	Dictionary d;
	d["range"] = range;
	d["height_bonus"] = height_bonus;
	return d;
}

void VisionComponent::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("range")) { range = p_dict["range"]; }
	if (p_dict.has("height_bonus")) { height_bonus = p_dict["height_bonus"]; }
}

Ref<CombatComponent> VisionComponent::clone() const {
	Ref<VisionComponent> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
