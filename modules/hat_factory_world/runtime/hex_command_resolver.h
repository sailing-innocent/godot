/**************************************************************************/
/*  hex_command_resolver.h                                                */
/*  Hat Factory World module — validates and commits map commands.       */
/*                                                                        */
/*  Invariant 3: MapCommand -> validation (ownership/capability/boundary/ */
/*  version/occupancy/lock) -> COW pages -> seq + dirty domains -> events */
/*  + persistence delta. Failures are atomic: no partial writes, no item/ */
/*  AP consumption.                                                       */
/**************************************************************************/
#ifndef HEX_COMMAND_RESOLVER_H
#define HEX_COMMAND_RESOLVER_H

#include "hex_map_command.h"
#include "hex_region_snapshot.h"

#include "core/object/ref_counted.h"

class HexCommandResult : public RefCounted {
	GDCLASS(HexCommandResult, RefCounted)
public:
	bool accepted = false;
	StringName reason = StringName("ok");
	Ref<HexRegionSnapshot> next_snapshot;
	Array events; // Dictionary events ready for the journal
	Array dirty_cells; // Vector2i micro coords
	Array dirty_edges; // Dictionary {coord: Dictionary, direction: int}
	int64_t new_version = 0;

	void set_rejected(const StringName &p_reason);
	bool get_accepted() const { return accepted; }
	StringName get_reason() const { return reason; }
	Ref<HexRegionSnapshot> get_next_snapshot() const { return next_snapshot; }
	Array get_events() const { return events; }
	Array get_dirty_cells() const { return dirty_cells; }
	Array get_dirty_edges() const { return dirty_edges; }
	int64_t get_new_version() const { return new_version; }

protected:
	static void _bind_methods();
};

class HexCommandResolver : public RefCounted {
	GDCLASS(HexCommandResolver, RefCounted)

public:
	/**
	 * Applies a command to a snapshot, returning a result whose next_snapshot
	 * shares all untouched state (COW). Never mutates the input snapshot.
	 * expected_version is enforced when non-zero (production callers stamp it).
	 */
	static Ref<HexCommandResult> apply(const Ref<HexRegionSnapshot> &p_snapshot, const Ref<HexMapCommand> &p_cmd);

protected:
	static void _bind_methods();
};

#endif // HEX_COMMAND_RESOLVER_H
