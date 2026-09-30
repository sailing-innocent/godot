/**************************************************************************/
/*  hex_encounter_bounds.h                                                */
/*  Hat Factory World module — battle boundary declaration (MAP-02/05).  */
/*                                                                        */
/*  D1: battle radius lives here in WORLD cells and micro radius is      */
/*  derived via the descriptor scale; never reuse the macro radius as a   */
/*  micro cell count. Commit/defeat strategies are declared per encounter */
/*  (D3).                                                                 */
/**************************************************************************/
#ifndef HEX_ENCOUNTER_BOUNDS_H
#define HEX_ENCOUNTER_BOUNDS_H

#include "../world/world_ids.h"

#include "core/io/resource.h"
#include "core/variant/typed_array.h"

class HexEncounterBounds : public Resource {
	GDCLASS(HexEncounterBounds, Resource)
	StringName bounds_kind = StringName("radius"); // radius | cells
	Ref<WorldCellCoord> center;
	int world_radius = 1; // world cells (D1: independent of micro count)
	TypedArray<WorldCellCoord> explicit_cells; // for "cells" kind
	TypedArray<WorldCellCoord> deployment_zone; // pre-battle deployment cells (world)
	Dictionary participants; // entity_id (StringName) -> world pos {x, z} (continuous)
	StringName commit_strategy = StringName("discard"); // commit_all | commit_on_victory | discard
	StringName defeat_strategy = StringName("rollback"); // rollback | keep
	double coverage_summary_threshold = 0.5; // macro summary recompute threshold (v1)

protected:
	static void _bind_methods();

public:
	void set_bounds_kind(const StringName &p_kind);
	StringName get_bounds_kind() const;
	void set_center(const Ref<WorldCellCoord> &p_center);
	Ref<WorldCellCoord> get_center() const;
	void set_world_radius(int p_radius);
	int get_world_radius() const;
	void set_explicit_cells(const TypedArray<WorldCellCoord> &p_cells);
	TypedArray<WorldCellCoord> get_explicit_cells() const;
	void set_deployment_zone(const TypedArray<WorldCellCoord> &p_cells);
	TypedArray<WorldCellCoord> get_deployment_zone() const;
	void set_participants(const Dictionary &p_participants);
	Dictionary get_participants() const;
	int get_participant_count() const;
	void set_commit_strategy(const StringName &p_strategy);
	StringName get_commit_strategy() const;
	void set_defeat_strategy(const StringName &p_strategy);
	StringName get_defeat_strategy() const;
	void set_coverage_summary_threshold(double p_threshold);
	double get_coverage_summary_threshold() const;

	/** Empty string when valid; arena entry is refused otherwise. */
	String validate() const;
};

#endif // HEX_ENCOUNTER_BOUNDS_H
