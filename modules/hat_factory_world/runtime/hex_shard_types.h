/**************************************************************************/
/*  hex_shard_types.h                                                     */
/*  Hat Factory World module — internal typed storage for micro cells.   */
/*                                                                        */
/*  Hot-path storage is struct-of-arrays, indexed (never Dictionary of   */
/*  Variant per cell). Shards are addressed by independent q/r floor     */
/*  division (negative coords supported); COW granularity is the page.   */
/*                                                                        */
/*  Invariant 6: snapshot/clone cost depends only on touched pages.      */
/**************************************************************************/
#ifndef HEX_SHARD_TYPES_H
#define HEX_SHARD_TYPES_H

#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "core/variant/packed_int32_array.h"
#include "core/variant/packed_int64_array.h"

namespace hf_world {

/** Shard covers SHARD_CELLS x SHARD_CELLS axial micro cells. */
static constexpr int32_t SHARD_CELLS = 16;
static constexpr int32_t SHARD_CELL_COUNT = SHARD_CELLS * SHARD_CELLS;
/** COW page covers PAGE_CELLS consecutive linear cells within a shard. */
static constexpr int32_t PAGE_CELLS = 64;
static constexpr int32_t PAGE_COUNT = SHARD_CELL_COUNT / PAGE_CELLS;
static constexpr int EDGE_SLOTS = 6;

/** Sentinel base index for "no data materialized at this cell". */
static constexpr int32_t NO_DATA = -1;

struct MicroAddr {
	int32_t shard_q = 0; // shard coordinate (floor-divided)
	int32_t shard_r = 0;
	int32_t local_q = 0; // 0..SHARD_CELLS-1
	int32_t local_r = 0;
	int32_t linear = 0; // local_r * SHARD_CELLS + local_q
	int32_t page = 0; // linear / PAGE_CELLS
	int32_t offset = 0; // linear % PAGE_CELLS
};

static inline int32_t _floor_div(int32_t a, int32_t b) {
	if (a >= 0) {
		return a / b;
	}
	return (int32_t)(-((-(int64_t)a + (int64_t)b - 1) / b));
}

/** Decomposes a micro coord into shard/local/page addressing. */
static inline MicroAddr addr_from_micro(int32_t p_q, int32_t p_r) {
	MicroAddr a;
	a.shard_q = _floor_div(p_q, SHARD_CELLS);
	a.shard_r = _floor_div(p_r, SHARD_CELLS);
	a.local_q = p_q - a.shard_q * SHARD_CELLS;
	a.local_r = p_r - a.shard_r * SHARD_CELLS;
	a.linear = a.local_r * SHARD_CELLS + a.local_q;
	a.page = a.linear / PAGE_CELLS;
	a.offset = a.linear % PAGE_CELLS;
	return a;
}

static inline uint64_t pack_shard_key(int32_t p_shard_q, int32_t p_shard_r) {
	return ((uint64_t)(uint32_t)p_shard_q << 32) | (uint32_t)p_shard_r;
}

/**
 * @brief One immutable COW page of typed micro cell columns.
 * Pages are never modified in place after publication; writers copy the
 * affected page, mutate the copy, and publish the new reference.
 */
class HexShardPage : public RefCounted {
public:
	int32_t base_offset = 0; // first linear cell index covered by this page
	PackedInt32Array base_terrain; // per cell: compiled table index, or NO_DATA
	PackedInt32Array elevation; // per cell
	PackedInt64Array surfaces; // per cell: bitmask of compiled surface indices
	PackedInt32Array object; // per cell: object def index+1, 0 = none
	PackedInt32Array object_durability; // per cell: -1 = n/a
	PackedInt32Array edges; // per cell*6: edge type index+1, 0 = open edge
	PackedInt32Array edges_enabled; // per cell*6: 0/1 (closed gates / shortcuts)
	PackedInt64Array area_effects; // per cell: bitmask of compiled area effect indices

	void init_empty(int32_t p_base_offset);
	bool is_empty() const; // all NO_DATA / zero — allows base-page fallthrough
	Dictionary to_dict() const;
	static Ref<HexShardPage> from_dict(const Dictionary &p_dict);
};

/**
 * @brief One immutable generation of a shard: page references plus the
 * shard-local stable feature registry. Cloning a snapshot clones the page
 * reference vector (and shares pages); features are copy-on-write.
 */
class HexShardState : public RefCounted {
public:
	int32_t chunk_q = 0;
	int32_t chunk_r = 0;
	Vector<Ref<HexShardPage>> pages; // indexed by page id, null = read base
	Dictionary features; // feature seq(int64) -> {kind, linear, data}
	int64 version = 0; // global_seq at which this generation was committed

	Ref<HexShardState> clone_for_write(int64 p_new_version) const;
	Dictionary to_dict() const;
	static Ref<HexShardState> from_dict(const Dictionary &p_dict);
};

} // namespace hf_world

#endif // HEX_SHARD_TYPES_H
