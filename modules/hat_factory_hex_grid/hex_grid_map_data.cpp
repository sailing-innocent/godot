#include "hex_grid_map_data.h"

#include "core/object/class_db.h"

void HexGridMapData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_cells", "cells"), &HexGridMapData::set_cells);
	ClassDB::bind_method(D_METHOD("get_cells"), &HexGridMapData::get_cells);

	ClassDB::bind_method(D_METHOD("set_default_terrain", "default_terrain"), &HexGridMapData::set_default_terrain);
	ClassDB::bind_method(D_METHOD("get_default_terrain"), &HexGridMapData::get_default_terrain);

	ClassDB::bind_method(D_METHOD("set_cell", "coord", "cell"), &HexGridMapData::set_cell);
	ClassDB::bind_method(D_METHOD("set_cells_bulk", "cells"), &HexGridMapData::set_cells_bulk);
	ClassDB::bind_method(D_METHOD("erase_cell", "coord"), &HexGridMapData::erase_cell);
	ClassDB::bind_method(D_METHOD("erase_cells_bulk", "coords"), &HexGridMapData::erase_cells_bulk);
	ClassDB::bind_method(D_METHOD("get_cell", "coord"), &HexGridMapData::get_cell);
	ClassDB::bind_method(D_METHOD("get_cell_or_default", "coord"), &HexGridMapData::get_cell_or_default);
	ClassDB::bind_method(D_METHOD("duplicate_cell", "coord"), &HexGridMapData::duplicate_cell);
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

void HexGridMapData::set_cells_bulk(const TypedArray<HexCellData> &p_cells) {
	bool any = false;
	for (int i = 0; i < p_cells.size(); i++) {
		Ref<HexCellData> cell = p_cells[i];
		if (cell.is_null()) {
			continue;
		}
		cells[cell->get_coord()] = cell;
		any = true;
	}
	if (any) {
		emit_changed();
	}
}

void HexGridMapData::erase_cell(const Vector2i &p_coord) {
	cells.erase(p_coord);
	emit_changed();
}

void HexGridMapData::erase_cells_bulk(const TypedArray<Vector2i> &p_coords) {
	bool any = false;
	for (int i = 0; i < p_coords.size(); i++) {
		Vector2i coord = p_coords[i];
		if (cells.has(coord)) {
			cells.erase(coord);
			any = true;
		}
	}
	if (any) {
		emit_changed();
	}
}

Ref<HexCellData> HexGridMapData::get_cell(const Vector2i &p_coord) const {
	return cells.get(p_coord, Variant());
}

Ref<HexCellData> HexGridMapData::get_cell_or_default(const Vector2i &p_coord) const {
	Ref<HexCellData> cell = cells.get(p_coord, Variant());
	if (cell.is_valid()) {
		return cell;
	}
	Ref<HexCellData> def;
	def.instantiate();
	def->set_coord(p_coord);
	def->set_terrain_id(default_terrain);
	return def;
}

Ref<HexCellData> HexGridMapData::duplicate_cell(const Vector2i &p_coord) const {
	Ref<HexCellData> cell = cells.get(p_coord, Variant());
	if (cell.is_null()) {
		return Ref<HexCellData>();
	}
	Ref<HexCellData> dup = cell->duplicate();
	return dup;
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
