/**************************************************************************/
/*  hex_edge_contract.h                                                   */
/*  Hat Factory World module — boundary contract for undirected world     */
/*  cell edges (architecture §3.2).                                       */
/*                                                                        */
/*  Every undirected world-cell edge carries one contract keyed by its    */
/*  normalized endpoints (EdgeKey), so both adjoining shards derive the    */
/*  identical boundary conditions (sea level, river flow, bridge slot,     */
/*  bank outline seed) regardless of which shard loads first.              */
/*                                                                        */
/*  v1 provides static contract derivation + cross-shard consistency       */
/*  validation. Event-driven advancement of these contracts is an          */
/*  explicit extension point (TODO, see docs §留空清单).                   */
/**************************************************************************/
#ifndef HEX_EDGE_CONTRACT_H
#define HEX_EDGE_CONTRACT_H

#include "world_ids.h"

#include "core/io/resource.h"
#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

class HexEdgeContract : public Resource {
	GDCLASS(HexEdgeContract, Resource)
	Ref<EdgeKey> edge;
	bool has_sea_level = false;
	double sea_level = 0.0; // world-space water level when has_sea_level
	double river_flow = 0.0; // boundary flux; 0 = no river crosses this edge
	int bridge_slot = -1; // declared bridge position index along the edge; -1 = none
	uint32_t bank_outline_seed = 0; // deterministic bank/coast outline seed
	uint32_t contract_checksum = 0; // integrity check over all fields above

protected:
	static void _bind_methods();

public:
	void set_edge(const Ref<EdgeKey> &p_edge);
	Ref<EdgeKey> get_edge() const;
	void set_has_sea_level(bool p_enabled);
	bool get_has_sea_level() const;
	void set_sea_level(double p_level);
	double get_sea_level() const;
	void set_river_flow(double p_flow);
	double get_river_flow() const;
	void set_bridge_slot(int p_slot);
	int get_bridge_slot() const;
	void set_bank_outline_seed(uint32_t p_seed);
	uint32_t get_bank_outline_seed() const;
	void set_contract_checksum(uint32_t p_checksum);
	uint32_t get_contract_checksum() const;

	bool is_valid() const;
	Dictionary to_dict() const;
	static Ref<HexEdgeContract> from_dict(const Dictionary &p_dict);

	/**
	 * Deterministically derives the contract for an edge from its normalized
	 * endpoints and the map seed. Any shard, any load order: same result.
	 */
	static Ref<HexEdgeContract> derive(const Ref<EdgeKey> &p_edge, int64_t p_map_seed);
	/** Recomputes contract_checksum from the current fields. */
	void recompute_checksum();

	/**
	 * Cross-shard consistency check: true when both contracts describe the
	 * same normalized edge with identical boundary conditions.
	 */
	bool is_consistent_with(const Ref<HexEdgeContract> &p_other) const;
	/**
	 * Validation entry for a set of contracts allegedly describing the same
	 * edge (e.g. claims arriving from multiple shards). Empty string = valid.
	 */
	static String validate_cross_shard(const TypedArray<HexEdgeContract> &p_contracts);
};

#endif // HEX_EDGE_CONTRACT_H
