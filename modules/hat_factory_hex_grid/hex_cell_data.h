#ifndef HEX_CELL_DATA_H
#define HEX_CELL_DATA_H

#include "hex_cell_effect.h"
#include "core/io/resource.h"
#include "core/math/vector2i.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

/**
 * @brief Runtime-serializable data for a single hexagonal cell.
 *
 * Kept deliberately flat so it can be authored as a Resource,
 * exported to JSON/CSV, and hot-reloaded by the HexGridMap node.
 */
class HexCellData : public Resource {
	GDCLASS(HexCellData, Resource)

	Vector2i coord;
	StringName terrain_id;
	int height = 0;
	int variant = 0;
	uint32_t flags = 0;
	Dictionary dynamic_state;
	TypedArray<HexCellEffect> effects;

protected:
	static void _bind_methods();

public:
	void set_coord(const Vector2i &p_coord);
	Vector2i get_coord() const;

	void set_terrain_id(const StringName &p_id);
	StringName get_terrain_id() const;

	void set_height(int p_height);
	int get_height() const;

	void set_variant(int p_variant);
	int get_variant() const;

	void set_flags(uint32_t p_flags);
	uint32_t get_flags() const;

	void set_dynamic_state(const Dictionary &p_state);
	Dictionary get_dynamic_state() const;

	void set_effects(const TypedArray<HexCellEffect> &p_effects);
	TypedArray<HexCellEffect> get_effects() const;
};

#endif // HEX_CELL_DATA_H
