/**************************************************************************/
/*  hex_edge_contract.cpp                                                 */
/**************************************************************************/
#include "hex_edge_contract.h"

#include "core/object/class_db.h"
#include "core/templates/hashfuncs.h"

void HexEdgeContract::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_edge", "edge"), &HexEdgeContract::set_edge);
	ClassDB::bind_method(D_METHOD("get_edge"), &HexEdgeContract::get_edge);
	ClassDB::bind_method(D_METHOD("set_has_sea_level", "enabled"), &HexEdgeContract::set_has_sea_level);
	ClassDB::bind_method(D_METHOD("get_has_sea_level"), &HexEdgeContract::get_has_sea_level);
	ClassDB::bind_method(D_METHOD("set_sea_level", "level"), &HexEdgeContract::set_sea_level);
	ClassDB::bind_method(D_METHOD("get_sea_level"), &HexEdgeContract::get_sea_level);
	ClassDB::bind_method(D_METHOD("set_river_flow", "flow"), &HexEdgeContract::set_river_flow);
	ClassDB::bind_method(D_METHOD("get_river_flow"), &HexEdgeContract::get_river_flow);
	ClassDB::bind_method(D_METHOD("set_bridge_slot", "slot"), &HexEdgeContract::set_bridge_slot);
	ClassDB::bind_method(D_METHOD("get_bridge_slot"), &HexEdgeContract::get_bridge_slot);
	ClassDB::bind_method(D_METHOD("set_bank_outline_seed", "seed"), &HexEdgeContract::set_bank_outline_seed);
	ClassDB::bind_method(D_METHOD("get_bank_outline_seed"), &HexEdgeContract::get_bank_outline_seed);
	ClassDB::bind_method(D_METHOD("set_contract_checksum", "checksum"), &HexEdgeContract::set_contract_checksum);
	ClassDB::bind_method(D_METHOD("get_contract_checksum"), &HexEdgeContract::get_contract_checksum);
	ClassDB::bind_method(D_METHOD("is_valid"), &HexEdgeContract::is_valid);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexEdgeContract::to_dict);
	ClassDB::bind_static_method("HexEdgeContract", D_METHOD("from_dict", "dict"), &HexEdgeContract::from_dict);
	ClassDB::bind_static_method("HexEdgeContract", D_METHOD("derive", "edge", "map_seed"), &HexEdgeContract::derive);
	ClassDB::bind_method(D_METHOD("recompute_checksum"), &HexEdgeContract::recompute_checksum);
	ClassDB::bind_method(D_METHOD("is_consistent_with", "other"), &HexEdgeContract::is_consistent_with);
	ClassDB::bind_static_method("HexEdgeContract", D_METHOD("validate_cross_shard", "contracts"), &HexEdgeContract::validate_cross_shard);
}

void HexEdgeContract::set_edge(const Ref<EdgeKey> &p_edge) { edge = p_edge; }
Ref<EdgeKey> HexEdgeContract::get_edge() const { return edge; }
void HexEdgeContract::set_has_sea_level(bool p_enabled) { has_sea_level = p_enabled; }
bool HexEdgeContract::get_has_sea_level() const { return has_sea_level; }
void HexEdgeContract::set_sea_level(double p_level) { sea_level = p_level; }
double HexEdgeContract::get_sea_level() const { return sea_level; }
void HexEdgeContract::set_river_flow(double p_flow) { river_flow = p_flow; }
double HexEdgeContract::get_river_flow() const { return river_flow; }
void HexEdgeContract::set_bridge_slot(int p_slot) { bridge_slot = p_slot; }
int HexEdgeContract::get_bridge_slot() const { return bridge_slot; }
void HexEdgeContract::set_bank_outline_seed(uint32_t p_seed) { bank_outline_seed = p_seed; }
uint32_t HexEdgeContract::get_bank_outline_seed() const { return bank_outline_seed; }
void HexEdgeContract::set_contract_checksum(uint32_t p_checksum) { contract_checksum = p_checksum; }
uint32_t HexEdgeContract::get_contract_checksum() const { return contract_checksum; }

bool HexEdgeContract::is_valid() const {
	if (edge.is_null() || !edge->is_valid()) {
		return false;
	}
	if (has_sea_level && sea_level < 0.0) {
		return false;
	}
	if (river_flow < 0.0) {
		return false;
	}
	return true;
}

Dictionary HexEdgeContract::to_dict() const {
	Dictionary d;
	d[StringName("edge")] = edge.is_valid() ? edge->to_dict() : Dictionary();
	d[StringName("has_sea_level")] = has_sea_level;
	d[StringName("sea_level")] = sea_level;
	d[StringName("river_flow")] = river_flow;
	d[StringName("bridge_slot")] = bridge_slot;
	d[StringName("bank_outline_seed")] = (int64_t)bank_outline_seed;
	d[StringName("contract_checksum")] = (int64_t)contract_checksum;
	return d;
}

Ref<HexEdgeContract> HexEdgeContract::from_dict(const Dictionary &p_dict) {
	Ref<HexEdgeContract> c;
	c.instantiate();
	Dictionary edge_dict = p_dict.get(StringName("edge"), Dictionary());
	c->set_edge(EdgeKey::from_dict(edge_dict));
	c->set_has_sea_level(p_dict.get(StringName("has_sea_level"), false));
	c->set_sea_level(p_dict.get(StringName("sea_level"), 0.0));
	c->set_river_flow(p_dict.get(StringName("river_flow"), 0.0));
	c->set_bridge_slot(p_dict.get(StringName("bridge_slot"), -1));
	c->set_bank_outline_seed((uint32_t)(int64_t)p_dict.get(StringName("bank_outline_seed"), (int64_t)0));
	c->set_contract_checksum((uint32_t)(int64_t)p_dict.get(StringName("contract_checksum"), (int64_t)0));
	return c;
}

static uint32_t _edge_field_hash(const Ref<EdgeKey> &p_edge, int64_t p_map_seed) {
	uint32_t h = hash_murmur3_one_64((uint64_t)p_map_seed);
	if (p_edge.is_valid() && p_edge->get_endpoint_a().is_valid() && p_edge->get_endpoint_b().is_valid()) {
		h = hash_murmur3_one_32((uint32_t)p_edge->get_endpoint_a()->get_q(), h);
		h = hash_murmur3_one_32((uint32_t)p_edge->get_endpoint_a()->get_r(), h);
		h = hash_murmur3_one_32((uint32_t)p_edge->get_endpoint_b()->get_q(), h);
		h = hash_murmur3_one_32((uint32_t)p_edge->get_endpoint_b()->get_r(), h);
	}
	return hash_fmix32(h);
}

Ref<HexEdgeContract> HexEdgeContract::derive(const Ref<EdgeKey> &p_edge, int64_t p_map_seed) {
	ERR_FAIL_COND_V(p_edge.is_null() || !p_edge->is_valid(), Ref<HexEdgeContract>());
	const uint32_t h = _edge_field_hash(p_edge, p_map_seed);
	Ref<HexEdgeContract> c;
	c.instantiate();
	c->set_edge(p_edge);
	// Deterministic placeholder boundary conditions. Real conditions arrive
	// from world templates in Phase 2/7; the derivation contract (normalized
	// endpoints + seed -> identical contract everywhere) is what v1 guarantees.
	c->set_has_sea_level((h & 0x1u) != 0);
	c->set_sea_level(c->get_has_sea_level() ? 1.0 : 0.0);
	c->set_river_flow((double)((h >> 1) & 0xFFu));
	c->set_bridge_slot(((int)((h >> 9) & 0x3u)) - 1);
	c->set_bank_outline_seed(h >> 11);
	c->recompute_checksum();
	return c;
}

void HexEdgeContract::recompute_checksum() {
	uint32_t h = edge.is_valid() ? edge->hash() : 0;
	h = hash_murmur3_one_32(has_sea_level ? 1u : 0u, h);
	h = hash_murmur3_one_32((uint32_t)sea_level, h);
	h = hash_murmur3_one_32((uint32_t)river_flow, h);
	h = hash_murmur3_one_32((uint32_t)bridge_slot, h);
	h = hash_murmur3_one_32(bank_outline_seed, h);
	contract_checksum = hash_fmix32(h);
}

bool HexEdgeContract::is_consistent_with(const Ref<HexEdgeContract> &p_other) const {
	if (p_other.is_null()) {
		return false;
	}
	if ((edge.is_null()) != (p_other->edge.is_null())) {
		return false;
	}
	if (edge.is_valid() && !edge->equals(p_other->edge)) {
		return false;
	}
	return has_sea_level == p_other->has_sea_level &&
			sea_level == p_other->sea_level &&
			river_flow == p_other->river_flow &&
			bridge_slot == p_other->bridge_slot &&
			bank_outline_seed == p_other->bank_outline_seed;
}

String HexEdgeContract::validate_cross_shard(const TypedArray<HexEdgeContract> &p_contracts) {
	if (p_contracts.is_empty()) {
		return "no contracts to validate";
	}
	Ref<HexEdgeContract> first = p_contracts[0];
	if (first.is_null()) {
		return "contract[0] is null";
	}
	if (!first->is_valid()) {
		return "contract[0] is invalid";
	}
	const uint32_t expected_checksum = first->get_contract_checksum();
	for (int i = 1; i < p_contracts.size(); i++) {
		Ref<HexEdgeContract> c = p_contracts[i];
		if (c.is_null()) {
			return "contract[" + itos(i) + "] is null";
		}
		if (!first->is_consistent_with(c)) {
			return "contract[" + itos(i) + "] disagrees with contract[0] on edge " + String(first->get_edge()->to_string());
		}
	}
	// TODO(extension point): event-driven advancement of edge contracts
	// (tides, flood propagation) plugs in here once the environment engine
	// lands; v1 validates static contracts only.
	return (first->get_contract_checksum() == expected_checksum) ? String() : "contract checksum mismatch";
}
