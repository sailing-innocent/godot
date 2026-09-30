/**************************************************************************/
/*  hex_legacy_importer.cpp                                               */
/**************************************************************************/
#include "hex_legacy_importer.h"

#include "../world/hex_scale_mapping.h"
#include "modules/hat_factory_hex_grid/hex_grid_map_data.h"
#include "modules/hat_factory_hex_grid/hex_terrain_library.h"

#include "core/object/class_db.h"

void HexLegacyImporter::_bind_methods() {
	ClassDB::bind_static_method("HexLegacyImporter", D_METHOD("import_map", "legacy_data", "library", "map_id", "scale"), &HexLegacyImporter::import_map);
}

Dictionary HexLegacyImporter::import_map(const Ref<HexGridMapData> &p_legacy, const Ref<HexTerrainLibrary> &p_library, const StringName &p_map_id, int p_scale) {
	Dictionary out;
	out[StringName("error")] = String();
	if (p_legacy.is_null() || p_library.is_null()) {
		out[StringName("error")] = "legacy data/library is null";
		return out;
	}
	if (p_scale < 2) {
		out[StringName("error")] = "scale must be >= 2";
		return out;
	}
	PackedStringArray warnings;

	// 1) Table migration: one base terrain per legacy id, preserving the
	//    movement profile; blocking terrains gain the impassable tag.
	Ref<HexTerrainTableSet> tables;
	tables.instantiate();
	tables->set_table_set_id(StringName(String(p_map_id) + "_tables"));
	Dictionary base_defs;
	const TypedArray<StringName> terrain_ids = p_library->get_terrain_ids();
	for (int i = 0; i < terrain_ids.size(); i++) {
		const StringName id = terrain_ids[i];
		Ref<HexTerrainDef> legacy_def = p_library->get_terrain(id);
		if (legacy_def.is_null()) {
			warnings.append("terrain id '" + String(id) + "' has no def; cells using it import as dirt");
			continue;
		}
		Ref<HexBaseTerrainDef> def;
		def.instantiate();
		def->set_def_id(id);
		def->set_display_name(String(id));
		def->set_move_cost(legacy_def->get_move_cost());
		def->set_blocks_movement(legacy_def->get_blocks_movement());
		PackedStringArray tags;
		if (legacy_def->get_blocks_movement()) {
			tags.append(StringName("impassable")); // blocks_movement -> tag semantics
		}
		def->set_tags(tags);
		base_defs[id] = def;
	}
	tables->set_base_terrains(base_defs);

	// 2) Micro cells: one per legacy cell, base terrain id carried over.
	Ref<HexDetailTemplate> detail;
	detail.instantiate();
	detail->set_map_id(p_map_id);
	Ref<TemplateVersion> version;
	version.instantiate();
	detail->set_template_version(version);
	Array detail_cells;
	const Dictionary legacy_cells = p_legacy->get_cells();
	const LocalVector<Variant> keys = legacy_cells.get_key_list();
	// Default terrain fallback for unknown ids.
	StringName default_terrain = p_legacy->get_default_terrain();
	if (!p_library->has_terrain(default_terrain) && terrain_ids.size() > 0) {
		default_terrain = terrain_ids[0];
	}
	int32_t min_q = INT32_MAX, max_q = INT32_MIN, min_r = INT32_MAX, max_r = INT32_MIN;
	for (const Variant &k : keys) {
		const Vector2i coord = k;
		Ref<HexCellData> cell = legacy_cells[k];
		if (cell.is_null()) {
			continue;
		}
		StringName terrain_id = cell->get_terrain_id();
		if (!p_library->has_terrain(terrain_id)) {
			warnings.append("cell (" + itos(coord.x) + "," + itos(coord.y) + ") uses unknown terrain '" + String(terrain_id) + "'; using default");
			terrain_id = default_terrain;
		}
		Ref<HexDetailCellDef> dc;
		dc.instantiate();
		dc->set_coord(MicroCoord::make(coord.x, coord.y, 1));
		dc->set_base_terrain(terrain_id);
		dc->set_elevation(cell->get_height());
		detail_cells.append(dc);
		min_q = MIN(min_q, coord.x);
		max_q = MAX(max_q, coord.x);
		min_r = MIN(min_r, coord.y);
		max_r = MAX(max_r, coord.y);
	}
	detail->set_cells(detail_cells);

	// 3) World template: world cells covering the micro extent by ownership.
	Ref<DetailMapDescriptor> desc;
	desc.instantiate();
	desc->set_map_id(p_map_id);
	desc->set_scale(p_scale);
	Ref<HexScaleMapping> mapping = HexScaleMapping::create(desc);
	Ref<HexWorldTemplate> world;
	world.instantiate();
	world->set_map_id(p_map_id);
	world->set_template_version(version);
	world->set_world_hex_size(4.0);
	world->set_detail_descriptor(desc);
	Array world_cells;
	HashMap<uint64_t, bool> seen;
	for (const Variant &k : keys) {
		const Vector2i coord = k;
		Ref<WorldCellCoord> wc = mapping->micro_cell_to_world_cell(MicroCoord::make(coord.x, coord.y, 1));
		const uint64_t wkey = (((uint64_t)(uint32_t)wc->get_q()) << 32) | (uint32_t)wc->get_r();
		if (seen.has(wkey)) {
			continue;
		}
		seen.insert(wkey, true);
		Ref<HexWorldCellDef> wdef;
		wdef.instantiate();
		wdef->set_coord(wc);
		wdef->set_biome(StringName("imported"));
		world_cells.append(wdef);
	}
	world->set_cells(world_cells);

	out[StringName("tables")] = tables;
	out[StringName("world")] = world;
	out[StringName("detail")] = detail;
	out[StringName("warnings")] = warnings;
	return out;
}
