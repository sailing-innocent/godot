#include "hex_cell_data.h"

#include "core/object/class_db.h"

void HexCellData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_coord", "coord"), &HexCellData::set_coord);
	ClassDB::bind_method(D_METHOD("get_coord"), &HexCellData::get_coord);

	ClassDB::bind_method(D_METHOD("set_terrain_id", "terrain_id"), &HexCellData::set_terrain_id);
	ClassDB::bind_method(D_METHOD("get_terrain_id"), &HexCellData::get_terrain_id);

	ClassDB::bind_method(D_METHOD("set_height", "height"), &HexCellData::set_height);
	ClassDB::bind_method(D_METHOD("get_height"), &HexCellData::get_height);

	ClassDB::bind_method(D_METHOD("set_variant", "variant"), &HexCellData::set_variant);
	ClassDB::bind_method(D_METHOD("get_variant"), &HexCellData::get_variant);

	ClassDB::bind_method(D_METHOD("set_flags", "flags"), &HexCellData::set_flags);
	ClassDB::bind_method(D_METHOD("get_flags"), &HexCellData::get_flags);

	ClassDB::bind_method(D_METHOD("set_dynamic_state", "dynamic_state"), &HexCellData::set_dynamic_state);
	ClassDB::bind_method(D_METHOD("get_dynamic_state"), &HexCellData::get_dynamic_state);

	ClassDB::bind_method(D_METHOD("set_effects", "effects"), &HexCellData::set_effects);
	ClassDB::bind_method(D_METHOD("get_effects"), &HexCellData::get_effects);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "coord"), "set_coord", "get_coord");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "terrain_id"), "set_terrain_id", "get_terrain_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "variant"), "set_variant", "get_variant");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "flags"), "set_flags", "get_flags");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "dynamic_state"), "set_dynamic_state", "get_dynamic_state");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "effects", PROPERTY_HINT_ARRAY_TYPE, "HexCellEffect"), "set_effects", "get_effects");
}

void HexCellData::set_coord(const Vector2i &p_coord) {
	coord = p_coord;
}

Vector2i HexCellData::get_coord() const {
	return coord;
}

void HexCellData::set_terrain_id(const StringName &p_id) {
	terrain_id = p_id;
}

StringName HexCellData::get_terrain_id() const {
	return terrain_id;
}

void HexCellData::set_height(int p_height) {
	height = p_height;
}

int HexCellData::get_height() const {
	return height;
}

void HexCellData::set_variant(int p_variant) {
	variant = p_variant;
}

int HexCellData::get_variant() const {
	return variant;
}

void HexCellData::set_flags(uint32_t p_flags) {
	flags = p_flags;
}

uint32_t HexCellData::get_flags() const {
	return flags;
}

void HexCellData::set_dynamic_state(const Dictionary &p_state) {
	dynamic_state = p_state;
}

Dictionary HexCellData::get_dynamic_state() const {
	return dynamic_state;
}

void HexCellData::set_effects(const TypedArray<HexCellEffect> &p_effects) {
	effects = p_effects;
}

TypedArray<HexCellEffect> HexCellData::get_effects() const {
	return effects;
}
