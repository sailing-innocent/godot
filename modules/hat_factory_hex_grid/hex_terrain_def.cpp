#include "hex_terrain_def.h"

#include "core/object/class_db.h"

void HexTerrainDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_terrain_id", "terrain_id"), &HexTerrainDef::set_terrain_id);
	ClassDB::bind_method(D_METHOD("get_terrain_id"), &HexTerrainDef::get_terrain_id);

	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &HexTerrainDef::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &HexTerrainDef::get_mesh);

	ClassDB::bind_method(D_METHOD("set_material", "material"), &HexTerrainDef::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &HexTerrainDef::get_material);

	ClassDB::bind_method(D_METHOD("set_physics_shape", "physics_shape"), &HexTerrainDef::set_physics_shape);
	ClassDB::bind_method(D_METHOD("get_physics_shape"), &HexTerrainDef::get_physics_shape);

	ClassDB::bind_method(D_METHOD("set_flags", "flags"), &HexTerrainDef::set_flags);
	ClassDB::bind_method(D_METHOD("get_flags"), &HexTerrainDef::get_flags);

	ClassDB::bind_method(D_METHOD("set_cast_shadows", "cast_shadows"), &HexTerrainDef::set_cast_shadows);
	ClassDB::bind_method(D_METHOD("get_cast_shadows"), &HexTerrainDef::get_cast_shadows);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "terrain_id"), "set_terrain_id", "get_terrain_id");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_mesh", "get_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_material", "get_material");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "physics_shape", PROPERTY_HINT_RESOURCE_TYPE, "Shape3D"), "set_physics_shape", "get_physics_shape");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "flags"), "set_flags", "get_flags");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cast_shadows"), "set_cast_shadows", "get_cast_shadows");
}

void HexTerrainDef::set_terrain_id(const StringName &p_id) {
	terrain_id = p_id;
}

StringName HexTerrainDef::get_terrain_id() const {
	return terrain_id;
}

void HexTerrainDef::set_mesh(const Ref<Mesh> &p_mesh) {
	mesh = p_mesh;
}

Ref<Mesh> HexTerrainDef::get_mesh() const {
	return mesh;
}

void HexTerrainDef::set_material(const Ref<Material> &p_material) {
	material = p_material;
}

Ref<Material> HexTerrainDef::get_material() const {
	return material;
}

void HexTerrainDef::set_physics_shape(const Ref<Shape3D> &p_shape) {
	physics_shape = p_shape;
}

Ref<Shape3D> HexTerrainDef::get_physics_shape() const {
	return physics_shape;
}

void HexTerrainDef::set_flags(uint32_t p_flags) {
	flags = p_flags;
}

uint32_t HexTerrainDef::get_flags() const {
	return flags;
}

void HexTerrainDef::set_cast_shadows(bool p_enabled) {
	cast_shadows = p_enabled;
}

bool HexTerrainDef::get_cast_shadows() const {
	return cast_shadows;
}
