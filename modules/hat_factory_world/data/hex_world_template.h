/**************************************************************************/
/*  hex_world_template.h                                                  */
/*  Hat Factory World module — macro/world authoring template.           */
/*                                                                        */
/*  Templates are immutable (Invariant 4). A WorldRun = template version  */
/*  + seed + versioned deltas; templates never hold runtime damage/       */
/*  occupancy.                                                             */
/**************************************************************************/
#ifndef HEX_WORLD_TEMPLATE_H
#define HEX_WORLD_TEMPLATE_H

#include "../world/hex_scale_mapping.h"
#include "../world/world_ids.h"

#include "core/io/resource.h"
#include "core/templates/hash_map.h"

class HexDetailTemplate; // defined in hex_detail_template.h

/**
 * @brief One macro world cell: biome/region summary, level-design chunking,
 * generator or manual micro override reference, world-space calibration.
 */
class HexWorldCellDef : public Resource {
	GDCLASS(HexWorldCellDef, Resource)
	Ref<WorldCellCoord> coord;
	StringName biome; // grassland / water / mountain / road / town ...
	StringName region_summary; // derived summary id, recomputed on writeback
	PackedStringArray tags; // e.g. "navigable_boat", "camp", "boss_arena"
	StringName chunk_id; // level-design logical chunk (independent of storage shard / render chunk)
	bool is_choke = false; // 咽喉格 marker (1-2 per chunk in dungeon topology)
	StringName generator_profile; // micro generator profile id (Phase 7)
	Ref<HexDetailTemplate> detail_override; // manual micro override

protected:
	static void _bind_methods();

public:
	void set_coord(const Ref<WorldCellCoord> &p_coord);
	Ref<WorldCellCoord> get_coord() const;
	void set_biome(const StringName &p_biome);
	StringName get_biome() const;
	void set_region_summary(const StringName &p_summary);
	StringName get_region_summary() const;
	void set_tags(const PackedStringArray &p_tags);
	PackedStringArray get_tags() const;
	bool has_tag(const StringName &p_tag) const;
	void set_chunk_id(const StringName &p_id);
	StringName get_chunk_id() const;
	void set_is_choke(bool p_enabled);
	bool get_is_choke() const;
	void set_generator_profile(const StringName &p_profile);
	StringName get_generator_profile() const;
	void set_detail_override(const Ref<HexDetailTemplate> &p_template);
	Ref<HexDetailTemplate> get_detail_override() const;
	Dictionary to_dict() const;
	static Ref<HexWorldCellDef> from_dict(const Dictionary &p_dict);
};

/**
 * @brief Explicit portal between two adjacent world cells (gate/shortcut).
 * Open portals need no entry; gates carry lock data (MAP-33/34).
 */
class HexWorldPortalDef : public Resource {
	GDCLASS(HexWorldPortalDef, Resource)
	Ref<EdgeKey> edge;
	StringName kind = StringName("open"); // open | gate | checkpoint | boat
	StringName required_key_id; // empty = unlocked
	bool enabled_by_default = true; // false = shortcut opened at runtime

protected:
	static void _bind_methods();

public:
	void set_edge(const Ref<EdgeKey> &p_edge);
	Ref<EdgeKey> get_edge() const;
	void set_kind(const StringName &p_kind);
	StringName get_kind() const;
	void set_required_key_id(const StringName &p_key);
	StringName get_required_key_id() const;
	void set_enabled_by_default(bool p_enabled);
	bool get_enabled_by_default() const;
	Dictionary to_dict() const;
	static Ref<HexWorldPortalDef> from_dict(const Dictionary &p_dict);
};

class HexWorldTemplate : public Resource {
	GDCLASS(HexWorldTemplate, Resource)
	StringName map_id;
	Ref<TemplateVersion> template_version;
	double world_hex_size = 4.0;
	StringName default_biome = StringName("grassland");
	Ref<DetailMapDescriptor> detail_descriptor;
	TypedArray<HexWorldCellDef> cells;
	TypedArray<HexWorldPortalDef> portals;
	bool compiled = false;
	HashMap<uint64_t, int> cell_index; // WorldCellCoord hash -> cell index

protected:
	static void _bind_methods();

public:
	void set_map_id(const StringName &p_id);
	StringName get_map_id() const;
	void set_template_version(const Ref<TemplateVersion> &p_version);
	Ref<TemplateVersion> get_template_version() const;
	void set_world_hex_size(double p_size);
	double get_world_hex_size() const;
	void set_default_biome(const StringName &p_biome);
	StringName get_default_biome() const;
	void set_detail_descriptor(const Ref<DetailMapDescriptor> &p_descriptor);
	Ref<DetailMapDescriptor> get_detail_descriptor() const;
	void set_cells(const TypedArray<HexWorldCellDef> &p_cells);
	TypedArray<HexWorldCellDef> get_cells() const;
	void set_portals(const TypedArray<HexWorldPortalDef> &p_portals);
	TypedArray<HexWorldPortalDef> get_portals() const;

	bool compile();
	bool is_compiled() const;
	int get_cell_count() const;
	Ref<HexWorldCellDef> get_cell_at(const Ref<WorldCellCoord> &p_coord) const;
	String validate() const;
};

#endif // HEX_WORLD_TEMPLATE_H
