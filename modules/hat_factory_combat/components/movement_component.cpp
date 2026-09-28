#include "movement_component.h"

#include "core/object/class_db.h"

void MovementComponent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_range", "range"), &MovementComponent::set_range);
	ClassDB::bind_method(D_METHOD("set_flying", "flying"), &MovementComponent::set_flying);
	ClassDB::bind_method(D_METHOD("set_jump_height", "jump_height"), &MovementComponent::set_jump_height);
	ClassDB::bind_method(D_METHOD("get_range"), &MovementComponent::get_range);
	ClassDB::bind_method(D_METHOD("get_flying"), &MovementComponent::get_flying);
	ClassDB::bind_method(D_METHOD("get_jump_height"), &MovementComponent::get_jump_height);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "range"), "set_range", "get_range");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flying"), "set_flying", "get_flying");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "jump_height"), "set_jump_height", "get_jump_height");
}

Dictionary MovementComponent::to_dict() const {
	Dictionary d;
	d["range"] = range;
	d["flying"] = flying;
	d["jump_height"] = jump_height;
	return d;
}

void MovementComponent::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("range")) { range = p_dict["range"]; }
	if (p_dict.has("flying")) { flying = p_dict["flying"]; }
	if (p_dict.has("jump_height")) { jump_height = p_dict["jump_height"]; }
}

Ref<CombatComponent> MovementComponent::clone() const {
	Ref<MovementComponent> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
