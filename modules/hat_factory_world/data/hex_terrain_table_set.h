/**************************************************************************/
/*  hex_terrain_table_set.h                                               */
/*  Hat Factory World module — the terrain/def table collection.         */
/*                                                                        */
/*  Authoring uses id-keyed Dictionaries (editor friendly). At load time  */
/*  compile() flattens them into indexed arrays + id->index hash maps so   */
/*  hot paths never touch Dictionary-of-Variant. Validate rejects broken   */
/*  references ( destroyed forms, reaction results, verb templates).      */
/**************************************************************************/
#ifndef HEX_TERRAIN_TABLE_SET_H
#define HEX_TERRAIN_TABLE_SET_H

#include "hex_movement_profile.h"
#include "hex_terrain_layer_defs.h"
#include "hex_verb_def.h"

#include "core/io/resource.h"
#include "core/templates/hash_map.h"
#include "core/variant/typed_array.h"

class HexTerrainTableSet : public Resource {
	GDCLASS(HexTerrainTableSet, Resource)
	StringName table_set_id;
	// Authoring form (id -> def).
	Dictionary base_terrains;
	Dictionary surfaces;
	Dictionary objects;
	Dictionary edge_types;
	Dictionary area_effects;
	Dictionary movement_profiles;
	Dictionary verbs;
	TypedArray<HexReactionDef> reactions; // ordered; reactions match by tags, id optional

	// Compiled form (built by compile(); read-only for consumers).
	bool compiled = false;
	String compile_error;
	TypedArray<HexBaseTerrainDef> c_bases;
	TypedArray<HexSurfaceDef> c_surfaces;
	TypedArray<HexObjectDef> c_objects;
	TypedArray<HexEdgeTypeDef> c_edge_types;
	TypedArray<HexAreaEffectDef> c_area_effects;
	TypedArray<HexMovementProfile> c_profiles;
	TypedArray<HexVerbDef> c_verbs;
	HashMap<StringName, int> i_base;
	HashMap<StringName, int> i_surface;
	HashMap<StringName, int> i_object;
	HashMap<StringName, int> i_edge_type;
	HashMap<StringName, int> i_area_effect;
	HashMap<StringName, int> i_profile;
	HashMap<StringName, int> i_verb;

protected:
	static void _bind_methods();
	template <typename T>
	static bool _compile_table(const Dictionary &p_src, TypedArray<T> &r_array, HashMap<StringName, int> &r_index, String &r_error, const char *p_label);

public:
	void set_table_set_id(const StringName &p_id);
	StringName get_table_set_id() const;

	void set_base_terrains(const Dictionary &p_defs);
	Dictionary get_base_terrains() const;
	void set_surfaces(const Dictionary &p_defs);
	Dictionary get_surfaces() const;
	void set_objects(const Dictionary &p_defs);
	Dictionary get_objects() const;
	void set_edge_types(const Dictionary &p_defs);
	Dictionary get_edge_types() const;
	void set_area_effects(const Dictionary &p_defs);
	Dictionary get_area_effects() const;
	void set_movement_profiles(const Dictionary &p_defs);
	Dictionary get_movement_profiles() const;
	void set_verbs(const Dictionary &p_defs);
	Dictionary get_verbs() const;
	void set_reactions(const TypedArray<HexReactionDef> &p_defs);
	TypedArray<HexReactionDef> get_reactions() const;

	/** Flattens authoring tables into indexed arrays. False + get_compile_error() on duplicates. */
	bool compile();
	bool is_compiled() const;
	String get_compile_error() const;

	int find_base_terrain(const StringName &p_id) const;
	int find_surface(const StringName &p_id) const;
	int find_object(const StringName &p_id) const;
	int find_edge_type(const StringName &p_id) const;
	int find_area_effect(const StringName &p_id) const;
	int find_movement_profile(const StringName &p_id) const;
	int find_verb(const StringName &p_id) const;
	Ref<HexBaseTerrainDef> get_base_terrain(int p_index) const;
	Ref<HexSurfaceDef> get_surface(int p_index) const;
	Ref<HexObjectDef> get_object(int p_index) const;
	Ref<HexEdgeTypeDef> get_edge_type(int p_index) const;
	Ref<HexAreaEffectDef> get_area_effect(int p_index) const;
	Ref<HexMovementProfile> get_movement_profile(int p_index) const;
	Ref<HexVerbDef> get_verb(int p_index) const;
	int get_base_terrain_count() const;
	int get_surface_count() const;
	int get_object_count() const;
	int get_edge_type_count() const;
	int get_area_effect_count() const;
	int get_movement_profile_count() const;
	int get_verb_count() const;
	int get_reaction_count() const;
	Ref<HexReactionDef> get_reaction(int p_index) const;

	/**
	 * Full reference validation; empty string = valid. Compiles first if
	 * needed (validate is logically const; compilation is idempotent caching).
	 */
	String validate() const;
};

#endif // HEX_TERRAIN_TABLE_SET_H
