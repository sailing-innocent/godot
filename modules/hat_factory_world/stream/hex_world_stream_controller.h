/**************************************************************************/
/*  hex_world_stream_controller.h                                         */
/*  Hat Factory World module — residency/streaming API (MAP-44).          */
/*                                                                        */
/*  v1: the whole region is resident in memory (Rules/Index never stream  */
/*  out); the FULL v1 API is exposed now — request/pin/unpin/ready query/   */
/*  priority/failure callback — with a synchronous placeholder engine.    */
/*  The async state machine (Unloaded→Requested→IO→…→Ready) and budget     */
/*  eviction are explicit extension points: request epochs and the stale-   */
/*  result rejection path are already in place so callers never accrue      */
/*  write-debt.                                                             */
/**************************************************************************/
#ifndef HEX_WORLD_STREAM_CONTROLLER_H
#define HEX_WORLD_STREAM_CONTROLLER_H

#include "../world/world_ids.h"

#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"

class HexRegionSnapshot; // defined in runtime/hex_region_snapshot.h

class HexWorldStreamController : public RefCounted {
	GDCLASS(HexWorldStreamController, RefCounted)
public:
	enum RegionState {
		STATE_UNLOADED = 0, // reserved for the async engine (v1 never reports this)
		STATE_REQUESTED = 1, // reserved
		STATE_READY = 2,
		STATE_FAILED = 3,
	};

private:
	Ref<HexRegionSnapshot> view;
	uint32_t epoch = 0;
	int next_pin = 1;
	// v1 synchronous pins: shard key -> pin count. Async engine replaces this
	// with the residency state machine; the pin/epoch semantics stay.
	HashMap<uint64_t, int> pin_counts;
	HashMap<int, uint64_t> pins;
	Callable on_failed;

public:
	void bind_view(const Ref<HexRegionSnapshot> &p_view);
	/**
	 * Requests a region (list of micro coords). v1: synchronous — everything
	 * resident — but returns an epoch so a later async engine can invalidate
	 * in-flight results without API changes.
	 */
	uint32_t request_region(const TypedArray<MicroCoord> &p_cells);
	int pin_region(const TypedArray<MicroCoord> &p_cells, int p_priority = 0);
	void unpin(int p_pin_handle);
	bool is_ready(const Ref<MicroCoord> &p_coord) const;
	int get_state(const Ref<MicroCoord> &p_coord) const;
	uint32_t get_epoch() const { return epoch; }
	void set_failure_callback(const Callable &p_callback);
	/** True when the epoch still matches the bound view version. */
	bool is_epoch_current(uint32_t p_epoch) const;

protected:
	static void _bind_methods();
};

VARIANT_ENUM_CAST(HexWorldStreamController::RegionState);

#endif // HEX_WORLD_STREAM_CONTROLLER_H
