#include "hex_grid_map_data.h"

#include "core/object/class_db.h"

void HexGridMapData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_cells", "cells"), &HexGridMapData::set_cells);
	ClassDB::bind_method(D_METHOD("get_cells"), &HexGridMapData::get_cells);

	ClassDB::bind_method(D_METHOD("set_default_terrain", "default_terrain"), &HexGridMapData::set_default_terrain);
	ClassDB::bind_method(D_METHOD("get_default_terrain"), &HexGridMapData::get_default_terrain);

	ClassDB::bind_method(D_METHOD("set_cell", "coord", "cell"), &HexGridMapData::set_cell);
	ClassDB::bind_method(D_METHOD("erase_cell", "coord"), &HexGridMapData::erase_cell);
	ClassDB::bind_method(D_METHOD("get_cell", "coord"), &HexGridMapData::get_cell);
	ClassDB::bind_method(D_METHOD("has_cell", "coord"), &HexGridMapData::has_cell);
	ClassDB::bind_method(D_METHOD("get_used_cells"), &HexGridMapData::get_used_cells);
	ClassDB::bind_method(D_METHOD("clear"), &HexGridMapData::clear);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "cells"), "set_cells", "get_cells");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "default_terrain"), "set_default_terrain", "get_default_terrain");
}

void HexGridMapData::set_cells(const Dictionary &p_cells) {
	cells = p_cells;
	emit_changed();
}

Dictionary HexGridMapData::get_cells() const {
	return cells;
}

void HexGridMapData::set_default_terrain(const StringName &p_id) {
	default_terrain = p_id;
}

StringName HexGridMapData::get_default_terrain() const {
	return default_terrain;
}

void HexGridMapData::set_cell(const Vector2i &p_coord, const Ref<HexCellData> &p_cell) {
	if (p_cell.is_null()) {
		erase_cell(p_coord);
		return;
	}
	cells[p_coord] = p_cell;
	emit_changed();
}

void HexGridMapData::erase_cell(const Vector2i &p_coord) {
	cells.erase(p_coord);
	emit_changed();
}

Ref<HexCellData> HexGridMapData::get_cell(const Vector2i &p_coord) const {
	return cells.get(p_coord, Variant());
}

bool HexGridMapData::has_cell(const Vector2i &p_coord) const {
	return cells.has(p_coord);
}

TypedArray<Vector2i> HexGridMapData::get_used_cells() const {
	TypedArray<Vector2i> used;
	Array keys = cells.keys();
	for (int i = 0; i < keys.size(); i++) {
		used.push_back(keys[i]);
	}
	return used;
}

void HexGridMapData::clear() {
	cells.clear();
	emit_changed();
}
