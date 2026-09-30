/**************************************************************************/
/*  hex_encounter_overlay.h                                               */
/*  Hat Factory World module — battle expansion at real world position.  */
/*                                                                        */
/*  The overlay shares the materialized world pages (MAP-03: snapshot     */
/*  cost tracks changed cells, never the world) and layers battle-local   */
/*  COW writes on top. Placements are legal and unique; without a legal   */
/*  cell the battle is refused (arena_not_ready / no legal placement).    */
/**************************************************************************/
#ifndef HEX_ENCOUNTER_OVERLAY_H
#define HEX_ENCOUNTER_OVERLAY_H

#include "../query/hex_occupancy.h"
#include "../runtime/hex_map_command.h"
#include "../runtime/hex_region_snapshot.h"
#include "hex_encounter_bounds.h"

#include "core/object/ref_counted.h"

class HexCommandResult; // defined in hex_command_resolver.h

class HexEncounterOverlay : public RefCounted {
	GDCLASS(HexEncounterOverlay, RefCounted)
public:
	Ref<HexEncounterBounds> bounds;
	Ref<HexRegionSnapshot> base_view; // shared world state at encounter start
	Ref<HexRegionSnapshot> overlay; // battle-local COW state (queries read this)
	Dictionary placements; // entity_id (StringName) -> micro coord dict
	String placement_error;
	int64_t start_seq = 0;

public:
	/**
	 * Expands the battle. Returns null on refusal; the reason is available
	 * via get_last_expand_error() (set on the calling thread before returning).
	 */
	static Ref<HexEncounterOverlay> expand(const Ref<HexWorldRun> &p_run, const Ref<HexEncounterBounds> &p_bounds);
	static String get_last_expand_error();

	/** World cells (authoritative scale) covered by this battle. */
	TypedArray<WorldCellCoord> get_world_cells() const;
	/** Micro cells inside the battle; empty when expansion failed. */
	TypedArray<MicroCoord> get_battle_cells() const;
	bool contains_micro(const Ref<MicroCoord> &p_coord) const;

	/** Query view for the battle: the overlay state. */
	Ref<HexRegionSnapshot> get_view() const { return overlay; }
	int64_t get_start_seq() const { return start_seq; }

	/**
	 * Projects participants to legal, unique micro cells (nearest to their
	 * world position; ties by stable id then lexicographic coord). Returns
	 * false when any participant has no legal cell — the battle must not
	 * start (level layout error via get_placement_error()).
	 */
	bool project_placements(const Ref<HexMovementProfile> &p_profile);
	String get_placement_error() const { return placement_error; }
	Dictionary get_placements() const { return placements; }
	Ref<MicroCoord> get_placement(const StringName &p_entity) const;

	/** Battle-local command application (goes through the same resolver). */
	Ref<HexCommandResult> apply_battle_command(const Ref<HexMapCommand> &p_command);
	/** Micro cell -> continuous world position (no re-entry drift). */
	Vector3 world_position_of(const Ref<MicroCoord> &p_coord) const;

	/**
	 * Deltas between overlay and base view for every battle cell, as a list
	 * of resolver commands ready for writeback.
	 */
	TypedArray<HexMapCommand> collect_deltas() const;

protected:
	static void _bind_methods();
	static bool _is_cell_equal(const Dictionary &p_before, const Dictionary &p_after);
	static void _push_delta_commands(const Dictionary &p_before, const Dictionary &p_after, const Ref<MicroCoord> &p_coord, TypedArray<HexMapCommand> &r_out);
};

#endif // HEX_ENCOUNTER_OVERLAY_H
