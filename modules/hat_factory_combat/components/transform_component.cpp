#include "transform_component.h"

#include "core/object/class_db.h"

void TransformComponent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_coord", "coord"), &TransformComponent::set_coord);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &TransformComponent::set_height);
	ClassDB::bind_method(D_METHOD("set_face_angle", "face_angle"), &TransformComponent::set_face_angle);
	ClassDB::bind_method(D_METHOD("get_coord"), &TransformComponent::get_coord);
	ClassDB::bind_method(D_METHOD("get_height"), &TransformComponent::get_height);
	ClassDB::bind_method(D_METHOD("get_face_angle"), &TransformComponent::get_face_angle);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "coord"), "set_coord", "get_coord");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "face_angle"), "set_face_angle", "get_face_angle");
}

Dictionary TransformComponent::to_dict() const {
	Dictionary d;
	d["coord"] = coord;
	d["height"] = height;
	d["face_angle"] = face_angle;
	return d;
}

void TransformComponent::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("coord")) { coord = p_dict["coord"]; }
	if (p_dict.has("height")) { height = p_dict["height"]; }
	if (p_dict.has("face_angle")) { face_angle = p_dict["face_angle"]; }
}

Ref<CombatComponent> TransformComponent::clone() const {
	Ref<TransformComponent> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
