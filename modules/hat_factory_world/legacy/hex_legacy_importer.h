/**************************************************************************/
/*  hex_legacy_importer.h                                                 */
/*  Hat Factory World module — legacy HexGridMapData import (Phase 7).    */
/*                                                                        */
/*  Old single-scale maps become: base-terrain table entries keyed by the  */
/*  legacy terrain ids (move_cost/blocks_movement preserved; blocking     */
/*  surfaces gain an "impassable" tag), plus a world template whose world  */
/*  cells cover the micro extent via the declared scale, plus a detail     */
/*  template with one micro cell per legacy cell. Original .tres untouched. */
/**************************************************************************/
#ifndef HEX_LEGACY_IMPORTER_H
#define HEX_LEGACY_IMPORTER_H

#include "../data/hex_detail_template.h"
#include "../data/hex_terrain_table_set.h"
#include "../data/hex_world_template.h"

#include "core/object/ref_counted.h"

class HexGridMapData; // hat_factory_hex_grid
class HexTerrainLibrary; // hat_factory_hex_grid

class HexLegacyImporter : public RefCounted {
	GDCLASS(HexLegacyImporter, RefCounted)

public:
	/**
	 * Returns {tables, world, detail, warnings: PackedStringArray} or an
	 * empty dictionary with "error" set. p_scale is the dual-map subdivision.
	 */
	static Dictionary import_map(const Ref<HexGridMapData> &p_legacy, const Ref<HexTerrainLibrary> &p_library, const StringName &p_map_id, int p_scale);

protected:
	static void _bind_methods();
};

#endif // HEX_LEGACY_IMPORTER_H
