/**************************************************************************/
/*  hex_detail_template.h                                                 */
/*  Hat Factory World module — micro/detail authoring template.          */
/*                                                                        */
/*  Hand-authored micro cells (locked battlefields, reefs, shortcuts)    */
/*  plus parameterized generator profiles (water/reef expansion, Phase 7). */
/*  Templates are immutable; runtime deltas live on the WorldRun.        */
/**************************************************************************/
#ifndef HEX_DETAIL_TEMPLATE_H
#define HEX_DETAIL_TEMPLATE_H

#include "../world/world_ids.h"

#include "core/io/resource.h"
#include "core/templates/hash_map.h"
#include "core/variant/typed_array.h"

/**
 * @brief One hand-authored micro cell. edge_type_ids follows the fixed
 * neighbor direction order: [(1,0),(1,-1),(0,-1),(-1,0),(-1,1),(0,1)].
 */
class HexDetailCellDef : public Resource {
	GDCLASS(HexDetailCellDef, Resource)
	Ref<MicroCoord> coord;
	StringName base_terrain; // HexBaseTerrainDef id
	int elevation = 0;
	PackedStringArray surface_ids; // HexSurfaceDef ids (stackable)
	StringName object_id; // HexObjectDef id; empty = none
	PackedStringArray edge_type_ids; // up to 6 HexEdgeTypeDef ids; empty slot = open edge
	PackedStringArray area_effect_ids; // HexAreaEffectDef ids
	Ref<FeatureId> feature; // optional stable feature binding

protected:
	static void _bind_methods();

public:
	void set_coord(const Ref<MicroCoord> &p_coord);
	Ref<MicroCoord> get_coord() const;
	void set_base_terrain(const StringName &p_id);
	StringName get_base_terrain() const;
	void set_elevation(int p_elevation);
	int get_elevation() const;
	void set_surface_ids(const PackedStringArray &p_ids);
	PackedStringArray get_surface_ids() const;
	void set_object_id(const StringName &p_id);
	StringName get_object_id() const;
	void set_edge_type_ids(const PackedStringArray &p_ids);
	PackedStringArray get_edge_type_ids() const;
	void set_area_effect_ids(const PackedStringArray &p_ids);
	PackedStringArray get_area_effect_ids() const;
	void set_feature(const Ref<FeatureId> &p_feature);
	Ref<FeatureId> get_feature() const;
	Dictionary to_dict() const;
	static Ref<HexDetailCellDef> from_dict(const Dictionary &p_dict);
};

class HexDetailTemplate : public Resource {
	GDCLASS(HexDetailTemplate, Resource)
	StringName map_id;
	Ref<TemplateVersion> template_version;
	TypedArray<HexDetailCellDef> cells;
	Dictionary generator_profile; // e.g. {"reef_density":0.2,"deep_fraction":0.5,"shore_width":1,"seed_salt":0}
	bool compiled = false;
	HashMap<uint64_t, int> cell_index; // packed micro coord -> index

protected:
	static void _bind_methods();

public:
	void set_map_id(const StringName &p_id);
	StringName get_map_id() const;
	void set_template_version(const Ref<TemplateVersion> &p_version);
	Ref<TemplateVersion> get_template_version() const;
	void set_cells(const TypedArray<HexDetailCellDef> &p_cells);
	TypedArray<HexDetailCellDef> get_cells() const;
	void set_generator_profile(const Dictionary &p_profile);
	Dictionary get_generator_profile() const;

	bool compile();
	bool is_compiled() const;
	int get_cell_count() const;
	Ref<HexDetailCellDef> get_cell_at(const Ref<MicroCoord> &p_coord) const;
	String validate() const;
};

#endif // HEX_DETAIL_TEMPLATE_H
