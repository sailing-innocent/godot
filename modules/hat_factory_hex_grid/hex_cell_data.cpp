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
	ClassDB::bind_method(D_METHOD("to_dict"), &HexCellData::to_dict);
	ClassDB::bind_static_method("HexCellData", D_METHOD("from_dict", "dict"), &HexCellData::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &HexCellData::clone);

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

Dictionary HexCellData::to_dict() const {
	Dictionary d;
	d["coord"] = coord;
	d["terrain_id"] = terrain_id;
	d["height"] = (int)height;
	d["variant"] = variant;
	d["flags"] = (int)flags;
	d["dynamic_state"] = dynamic_state;
	Array eff;
	for (int i = 0; i < effects.size(); i++) {
		Ref<HexCellEffect> e = effects[i];
		if (e.is_valid()) eff.push_back(e->to_dict());
	}
	d["effects"] = eff;
	return d;
}

Ref<HexCellData> HexCellData::from_dict(const Dictionary &p_dict) {
	Ref<HexCellData> cell;
	cell.instantiate();
	if (p_dict.has("coord")) cell->coord = p_dict["coord"];
	if (p_dict.has("terrain_id")) cell->terrain_id = p_dict["terrain_id"];
	if (p_dict.has("height")) cell->height = p_dict["height"];
	if (p_dict.has("variant")) cell->variant = p_dict["variant"];
	if (p_dict.has("flags")) cell->flags = (uint32_t)(int)p_dict["flags"];
	if (p_dict.has("dynamic_state")) cell->dynamic_state = p_dict["dynamic_state"];
	if (p_dict.has("effects")) {
		Array eff = p_dict["effects"];
		// Effects are kept as raw dictionaries; reconstruction via HexCellEffect factory not required for base snapshot.
		cell->effects = eff;
	}
	return cell;
}

Ref<HexCellData> HexCellData::clone() const {
	return from_dict(to_dict());
}
