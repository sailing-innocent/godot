#ifndef HEX_TERRAIN_LIBRARY_H
#define HEX_TERRAIN_LIBRARY_H

#include "hex_terrain_def.h"
#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

/**
 * @brief A dictionary-like resource that maps terrain ids to HexTerrainDef.
 */
class HexTerrainLibrary : public Resource {
	GDCLASS(HexTerrainLibrary, Resource)

	Dictionary terrains;

protected:
	static void _bind_methods();

public:
	void set_terrains(const Dictionary &p_terrains);
	Dictionary get_terrains() const;

	void add_terrain(const StringName &p_id, const Ref<HexTerrainDef> &p_def);
	void remove_terrain(const StringName &p_id);
	bool has_terrain(const StringName &p_id) const;
	Ref<HexTerrainDef> get_terrain(const StringName &p_id) const;
	TypedArray<StringName> get_terrain_ids() const;
};

#endif // HEX_TERRAIN_LIBRARY_H
