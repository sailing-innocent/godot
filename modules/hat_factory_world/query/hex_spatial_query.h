/**************************************************************************/
/*  hex_spatial_query.h                                                   */
/*  Hat Factory World module — the single source of spatial truth.       */
/*                                                                        */
/*  Players, AI, UI and previews all call the same implementation         */
/*  (MAP-11/13/25). Micro scale runs weighted Dijkstra with per-step      */
/*  action typing (walk/hop/fall/glide/wade/break_through); macro scale   */
/*  plans over the world portal graph. Failure reasons are typed and      */
/*  include PENDING_DATA (neighbor shard not ready) vs BLOCKED.           */
/**************************************************************************/
#ifndef HEX_SPATIAL_QUERY_H
#define HEX_SPATIAL_QUERY_H

#include "../data/hex_movement_profile.h"
#include "../data/hex_world_template.h"
#include "../runtime/hex_region_snapshot.h"
#include "hex_occupancy.h"

#include "core/object/ref_counted.h"

class HexQueryStep : public RefCounted {
	GDCLASS(HexQueryStep, RefCounted)
public:
	Ref<MicroCoord> to;
	StringName action = StringName("walk"); // walk/hop/fall/glide/wade/break_through
	int cost = 0; // cumulative cost including this step

	void set_to(const Ref<MicroCoord> &p_coord) { to = p_coord; }
	Ref<MicroCoord> get_to() const { return to; }
	void set_action(const StringName &p_action) { action = p_action; }
	StringName get_action() const { return action; }
	void set_cost(int p_cost) { cost = p_cost; }
	int get_cost() const { return cost; }

protected:
	static void _bind_methods();
};

class HexPathResult : public RefCounted {
	GDCLASS(HexPathResult, RefCounted)
public:
	bool found = false;
	StringName reason = StringName("reachable"); // typed failure reason when !found
	TypedArray<HexQueryStep> steps;
	Array world_steps; // Dictionary {q, r} for macro paths
	int total_cost = 0;

	bool get_found() const { return found; }
	StringName get_reason() const { return reason; }
	TypedArray<HexQueryStep> get_steps() const { return steps; }
	Array get_world_steps() const { return world_steps; }
	int get_total_cost() const { return total_cost; }

protected:
	static void _bind_methods();
};

class HexSpatialQuery : public RefCounted {
	GDCLASS(HexSpatialQuery, RefCounted)

public:
	// --- Micro scale (the one tactical truth) ---
	static Ref<HexPathResult> find_path(const Ref<HexRegionSnapshot> &p_view, const Ref<MicroCoord> &p_from, const Ref<MicroCoord> &p_to, const Ref<HexMovementProfile> &p_profile, const Ref<HexOccupancySnapshot> &p_occupancy, int p_max_cost = 64);
	static TypedArray<MicroCoord> reachable_cells(const Ref<HexRegionSnapshot> &p_view, const Ref<MicroCoord> &p_from, const Ref<HexMovementProfile> &p_profile, const Ref<HexOccupancySnapshot> &p_occupancy, int p_max_cost);
	static bool has_line_of_sight(const Ref<HexRegionSnapshot> &p_view, const Ref<MicroCoord> &p_from, const Ref<MicroCoord> &p_to);
	/** Vision = LOS clipped range (alert zones use this, MAP-25). */
	static TypedArray<MicroCoord> visible_cells(const Ref<HexRegionSnapshot> &p_view, const Ref<MicroCoord> &p_from, int p_range);

	// --- Macro scale (portal graph planning, heuristic only) ---
	static Ref<HexPathResult> find_world_path(const Ref<HexWorldTemplate> &p_template, const Ref<WorldCellCoord> &p_from, const Ref<WorldCellCoord> &p_to, const Ref<HexMovementProfile> &p_profile);

protected:
	static void _bind_methods();
};

#endif // HEX_SPATIAL_QUERY_H
