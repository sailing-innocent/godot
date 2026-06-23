#ifndef HEX_GRID_MAP_DATA_H
#define HEX_GRID_MAP_DATA_H

#include "hex_cell_data.h"
#include "core/io/resource.h"
#include "core/math/vector2i.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

/**
 * @brief Serializable container for all cells of a hexagonal map.
 *
 * This resource is the authoritative data layer. Gameplay systems,
 * level editors and workshop exporters modify this resource; the
 * HexGridMap node listens to its changed signal and rebuilds only
 * the affected render chunks.
 */
class HexGridMapData : public Resource {
	GDCLASS(HexGridMapData, Resource)

	Dictionary cells; // key: Vector2i, value: Ref<HexCellData>
	StringName default_terrain;

protected:
	static void _bind_methods();

public:
	void set_cells(const Dictionary &p_cells);
	Dictionary get_cells() const;

	void set_default_terrain(const StringName &p_id);
	StringName get_default_terrain() const;

	void set_cell(const Vector2i &p_coord, const Ref<HexCellData> &p_cell);
	void set_cells_bulk(const TypedArray<HexCellData> &p_cells);
	void erase_cell(const Vector2i &p_coord);
	void erase_cells_bulk(const TypedArray<Vector2i> &p_coords);
	Ref<HexCellData> get_cell(const Vector2i &p_coord) const;
	Ref<HexCellData> get_cell_or_default(const Vector2i &p_coord) const;
	Ref<HexCellData> duplicate_cell(const Vector2i &p_coord) const;
	bool has_cell(const Vector2i &p_coord) const;
	TypedArray<Vector2i> get_used_cells() const;
	void clear();
};

#endif // HEX_GRID_MAP_DATA_H
