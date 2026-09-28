#include "hex_transition_instance_data.h"

#include "core/object/class_db.h"

void HexTransitionInstanceData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_source_coord", "source_coord"), &HexTransitionInstanceData::set_source_coord);
	ClassDB::bind_method(D_METHOD("get_source_coord"), &HexTransitionInstanceData::get_source_coord);

	ClassDB::bind_method(D_METHOD("set_direction", "direction"), &HexTransitionInstanceData::set_direction);
	ClassDB::bind_method(D_METHOD("get_direction"), &HexTransitionInstanceData::get_direction);

	ClassDB::bind_method(D_METHOD("set_profile", "profile"), &HexTransitionInstanceData::set_profile);
	ClassDB::bind_method(D_METHOD("get_profile"), &HexTransitionInstanceData::get_profile);

	ClassDB::bind_method(D_METHOD("set_transform", "transform"), &HexTransitionInstanceData::set_transform);
	ClassDB::bind_method(D_METHOD("get_transform"), &HexTransitionInstanceData::get_transform);

	ClassDB::bind_method(D_METHOD("set_gameplay_flags", "gameplay_flags"), &HexTransitionInstanceData::set_gameplay_flags);
	ClassDB::bind_method(D_METHOD("get_gameplay_flags"), &HexTransitionInstanceData::get_gameplay_flags);

	ClassDB::bind_method(D_METHOD("set_gameplay_state_id", "gameplay_state_id"), &HexTransitionInstanceData::set_gameplay_state_id);
	ClassDB::bind_method(D_METHOD("get_gameplay_state_id"), &HexTransitionInstanceData::get_gameplay_state_id);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "source_coord"), "set_source_coord", "get_source_coord");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "direction"), "set_direction", "get_direction");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "profile", PROPERTY_HINT_RESOURCE_TYPE, "HexTransitionProfile"), "set_profile", "get_profile");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM3D, "transform"), "set_transform", "get_transform");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "gameplay_flags"), "set_gameplay_flags", "get_gameplay_flags");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "gameplay_state_id"), "set_gameplay_state_id", "get_gameplay_state_id");
}

void HexTransitionInstanceData::set_source_coord(const Vector2i &p_coord) {
	source_coord = p_coord;
}

Vector2i HexTransitionInstanceData::get_source_coord() const {
	return source_coord;
}

void HexTransitionInstanceData::set_direction(int p_direction) {
	direction = p_direction;
}

int HexTransitionInstanceData::get_direction() const {
	return direction;
}

void HexTransitionInstanceData::set_profile(const Ref<HexTransitionProfile> &p_profile) {
	profile = p_profile;
}

Ref<HexTransitionProfile> HexTransitionInstanceData::get_profile() const {
	return profile;
}

void HexTransitionInstanceData::set_transform(const Transform3D &p_transform) {
	transform = p_transform;
}

Transform3D HexTransitionInstanceData::get_transform() const {
	return transform;
}

void HexTransitionInstanceData::set_gameplay_flags(uint32_t p_flags) {
	gameplay_flags = p_flags;
}

uint32_t HexTransitionInstanceData::get_gameplay_flags() const {
	return gameplay_flags;
}

void HexTransitionInstanceData::set_gameplay_state_id(const StringName &p_id) {
	gameplay_state_id = p_id;
}

StringName HexTransitionInstanceData::get_gameplay_state_id() const {
	return gameplay_state_id;
}
