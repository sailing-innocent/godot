#include "hex_transition_profile.h"

#include "core/object/class_db.h"

void HexTransitionProfile::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_profile_id", "profile_id"), &HexTransitionProfile::set_profile_id);
	ClassDB::bind_method(D_METHOD("get_profile_id"), &HexTransitionProfile::get_profile_id);

	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &HexTransitionProfile::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &HexTransitionProfile::get_mesh);

	ClassDB::bind_method(D_METHOD("set_material", "material"), &HexTransitionProfile::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &HexTransitionProfile::get_material);

	ClassDB::bind_method(D_METHOD("set_mesh_offset", "mesh_offset"), &HexTransitionProfile::set_mesh_offset);
	ClassDB::bind_method(D_METHOD("get_mesh_offset"), &HexTransitionProfile::get_mesh_offset);

	ClassDB::bind_method(D_METHOD("set_mesh_scale", "mesh_scale"), &HexTransitionProfile::set_mesh_scale);
	ClassDB::bind_method(D_METHOD("get_mesh_scale"), &HexTransitionProfile::get_mesh_scale);

	ClassDB::bind_method(D_METHOD("set_height_scale", "height_scale"), &HexTransitionProfile::set_height_scale);
	ClassDB::bind_method(D_METHOD("get_height_scale"), &HexTransitionProfile::get_height_scale);

	ClassDB::bind_method(D_METHOD("set_cast_shadows", "cast_shadows"), &HexTransitionProfile::set_cast_shadows);
	ClassDB::bind_method(D_METHOD("get_cast_shadows"), &HexTransitionProfile::get_cast_shadows);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "profile_id"), "set_profile_id", "get_profile_id");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_material", "get_material");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "mesh_offset"), "set_mesh_offset", "get_mesh_offset");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "mesh_scale"), "set_mesh_scale", "get_mesh_scale");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height_scale"), "set_height_scale", "get_height_scale");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cast_shadows"), "set_cast_shadows", "get_cast_shadows");
}

void HexTransitionProfile::set_profile_id(const StringName &p_id) {
	profile_id = p_id;
}

StringName HexTransitionProfile::get_profile_id() const {
	return profile_id;
}

void HexTransitionProfile::set_mesh(const Ref<Mesh> &p_mesh) {
	mesh = p_mesh;
}

Ref<Mesh> HexTransitionProfile::get_mesh() const {
	return mesh;
}

void HexTransitionProfile::set_material(const Ref<Material> &p_material) {
	material = p_material;
}

Ref<Material> HexTransitionProfile::get_material() const {
	return material;
}

void HexTransitionProfile::set_mesh_offset(const Vector3 &p_offset) {
	mesh_offset = p_offset;
}

Vector3 HexTransitionProfile::get_mesh_offset() const {
	return mesh_offset;
}

void HexTransitionProfile::set_mesh_scale(const Vector3 &p_scale) {
	mesh_scale = p_scale;
}

Vector3 HexTransitionProfile::get_mesh_scale() const {
	return mesh_scale;
}

void HexTransitionProfile::set_height_scale(float p_scale) {
	height_scale = p_scale;
}

float HexTransitionProfile::get_height_scale() const {
	return height_scale;
}

void HexTransitionProfile::set_cast_shadows(bool p_enabled) {
	cast_shadows = p_enabled;
}

bool HexTransitionProfile::get_cast_shadows() const {
	return cast_shadows;
}
