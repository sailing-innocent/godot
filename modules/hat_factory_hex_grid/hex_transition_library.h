#ifndef HEX_TRANSITION_LIBRARY_H
#define HEX_TRANSITION_LIBRARY_H

#include "hex_transition_def.h"
#include "hex_terrain_library.h"
#include "hex_cell_data.h"
#include "core/io/resource.h"
#include "core/variant/typed_array.h"

/**
 * @brief Collection of transition rules used by HexTransitionGenerator.
 */
class HexTransitionLibrary : public Resource {
	GDCLASS(HexTransitionLibrary, Resource)

	TypedArray<HexTransitionDef> definitions;

protected:
	static void _bind_methods();

	static bool _tags_match(const PackedStringArray &p_rule_tags, const PackedStringArray &p_terrain_tags);

public:
	void set_definitions(const TypedArray<HexTransitionDef> &p_definitions);
	TypedArray<HexTransitionDef> get_definitions() const;

	void add_definition(const Ref<HexTransitionDef> &p_def);
	void remove_definition(const Ref<HexTransitionDef> &p_def);

	Ref<HexTransitionDef> find_rule(const Ref<HexCellData> &p_source,
			const Ref<HexCellData> &p_target,
			int p_height_delta,
			const Ref<HexTerrainLibrary> &p_terrain_library) const;
};

#endif // HEX_TRANSITION_LIBRARY_H
