/**************************************************************************/
/*  hex_movement_profile.h                                                */
/*  Hat Factory World module — traversal profile (MAP-11/12/14).         */
/*                                                                        */
/*  A movement profile decides "who can walk": size class + hat tags +    */
/*  state/item tags, evaluated against terrain/surface/edge tags.         */
/*  Costs come from data only; rule code must never hardcode terrain ids. */
/**************************************************************************/
#ifndef HEX_MOVEMENT_PROFILE_H
#define HEX_MOVEMENT_PROFILE_H

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"

class HexMovementProfile : public Resource {
	GDCLASS(HexMovementProfile, Resource)
	StringName profile_id;
	StringName size_class = StringName("medium");
	PackedStringArray hat_tags; // hat ability tags that grant actions (e.g. "feather" -> glide)
	int base_move_cost = 1;
	Dictionary terrain_tag_costs; // tag -> cost override (smallest matching cost wins)
	PackedStringArray impassable_tags; // any matching tag blocks entry
	PackedStringArray allowed_edge_actions; // walk/hop/fall/glide/wade/break_through
	int max_climb_height = 1; // max elevation delta climbed as a walk action
	int glide_max_drop = 4; // max elevation delta crossed by a glide action
	bool can_cross_friendly = true; // friendly occupancy may be crossed but not stopped on

protected:
	static void _bind_methods();

public:
	void set_profile_id(const StringName &p_id);
	StringName get_profile_id() const;
	/** Uniform id accessor shared with layer defs (table compilation). */
	StringName get_def_id() const { return get_profile_id(); }
	void set_size_class(const StringName &p_size);
	StringName get_size_class() const;
	void set_hat_tags(const PackedStringArray &p_tags);
	PackedStringArray get_hat_tags() const;
	bool has_hat_tag(const StringName &p_tag) const;
	void set_base_move_cost(int p_cost);
	int get_base_move_cost() const;
	void set_terrain_tag_costs(const Dictionary &p_costs);
	Dictionary get_terrain_tag_costs() const;
	void set_impassable_tags(const PackedStringArray &p_tags);
	PackedStringArray get_impassable_tags() const;
	void set_allowed_edge_actions(const PackedStringArray &p_actions);
	PackedStringArray get_allowed_edge_actions() const;
	bool can_use_edge_action(const StringName &p_action) const;
	void set_max_climb_height(int p_height);
	int get_max_climb_height() const;
	void set_glide_max_drop(int p_drop);
	int get_glide_max_drop() const;
	void set_can_cross_friendly(bool p_enabled);
	bool get_can_cross_friendly() const;

	/**
	 * Cost to enter a cell carrying the given tags; -1 = impassable.
	 * Semantics: any impassable tag match wins; otherwise the smallest
	 * matching terrain_tag_cost, else base_move_cost.
	 */
	int get_cost_for_tags(const PackedStringArray &p_cell_tags) const;
	Dictionary to_dict() const;
	static Ref<HexMovementProfile> from_dict(const Dictionary &p_dict);
	String validate() const;
};

#endif // HEX_MOVEMENT_PROFILE_H
