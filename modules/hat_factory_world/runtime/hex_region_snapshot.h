/**************************************************************************/
/*  hex_region_snapshot.h                                                 */
/*  Hat Factory World module — immutable snapshot view of micro state.   */
/*                                                                        */
/*  A snapshot is a map of shard -> shard generation. Clone copies the    */
/*  map and shares all shard state (O(#shards)); writers clone only the   */
/*  affected shard states and COW pages (Invariant 6). Readers fall back  */
/*  to the run's immutable materialized base for shards not present here. */
/**************************************************************************/
#ifndef HEX_REGION_SNAPSHOT_H
#define HEX_REGION_SNAPSHOT_H

#include "../data/hex_terrain_table_set.h"
#include "hex_shard_types.h"

#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"

class HexWorldRun; // defined in hex_world_run.h

class HexRegionSnapshot : public RefCounted {
	GDCLASS(HexRegionSnapshot, RefCounted)
public:
	HashMap<uint64_t, Ref<hf_world::HexShardState>> shards; // delta/committed view over the run base
	Ref<HexTerrainTableSet> tables; // shared immutable tables for id resolution
	int64_t version = 0;

private:
	Ref<HexWorldRun> run; // base materialization fallback

public:
	void set_run(const Ref<HexWorldRun> &p_run);
	Ref<HexWorldRun> get_run() const;

	// --- Internal read path (resolver/query hot path) ---
	Ref<hf_world::HexShardPage> find_page(const hf_world::MicroAddr &p_addr) const;
	Ref<hf_world::HexShardState> find_shard_state(uint64_t p_key) const;

	// --- GDScript read API ---
	bool has_cell_data(const Ref<MicroCoord> &p_coord) const;
	StringName get_base_terrain_at(const Ref<MicroCoord> &p_coord) const;
	int get_elevation_at(const Ref<MicroCoord> &p_coord) const;
	/** Full cell view with resolved def ids (tests, view bridge, debugging). */
	Dictionary get_cell_view(const Ref<MicroCoord> &p_coord) const;
	int64_t get_version() const;
	int get_shard_count() const;

	/** Structural-sharing clone: same state, new version label. */
	Ref<HexRegionSnapshot> clone_shallow(int64_t p_new_version) const;
	/** Replace/insert a shard generation (writers only). */
	void put_shard_state(uint64_t p_key, const Ref<hf_world::HexShardState> &p_state);

	Dictionary to_dict() const;
	static Ref<HexRegionSnapshot> from_dict(const Dictionary &p_dict, const Ref<HexTerrainTableSet> &p_tables);

protected:
	static void _bind_methods();
};

#endif // HEX_REGION_SNAPSHOT_H
