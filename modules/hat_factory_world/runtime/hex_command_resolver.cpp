/**************************************************************************/
/*  hex_command_resolver.cpp                                              */
/**************************************************************************/
#include "hex_command_resolver.h"

#include "../data/hex_verb_def.h"
#include "hex_world_run.h"

#include "core/object/class_db.h"

void HexCommandResult::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_rejected", "reason"), &HexCommandResult::set_rejected);
	ClassDB::bind_method(D_METHOD("get_accepted"), &HexCommandResult::get_accepted);
	ClassDB::bind_method(D_METHOD("get_reason"), &HexCommandResult::get_reason);
	ClassDB::bind_method(D_METHOD("get_next_snapshot"), &HexCommandResult::get_next_snapshot);
	ClassDB::bind_method(D_METHOD("get_events"), &HexCommandResult::get_events);
	ClassDB::bind_method(D_METHOD("get_dirty_cells"), &HexCommandResult::get_dirty_cells);
	ClassDB::bind_method(D_METHOD("get_dirty_edges"), &HexCommandResult::get_dirty_edges);
	ClassDB::bind_method(D_METHOD("get_new_version"), &HexCommandResult::get_new_version);
}

void HexCommandResolver::_bind_methods() {
	ClassDB::bind_static_method("HexCommandResolver", D_METHOD("apply", "snapshot", "command"), &HexCommandResolver::apply);
}

void HexCommandResult::set_rejected(const StringName &p_reason) {
	accepted = false;
	reason = p_reason;
	next_snapshot = Ref<HexRegionSnapshot>();
	events.clear();
	dirty_cells.clear();
	dirty_edges.clear();
	new_version = 0;
}

namespace {

/** One write transaction: lazily clones shard states and COW pages. */
class Tx {
public:
	Ref<HexRegionSnapshot> src;
	Ref<HexRegionSnapshot> dst;
	Ref<HexCommandResult> result;
	HashMap<uint64_t, Ref<hf_world::HexShardState>> states;
	HashMap<uint64_t, HashSet<int32_t>> copied_pages;

	Tx(const Ref<HexRegionSnapshot> &p_src, const Ref<HexCommandResult> &p_result) {
		src = p_src;
		result = p_result;
		dst = p_src->clone_shallow(p_src->get_version() + 1);
	}

	Ref<hf_world::HexShardState> writable_shard(const hf_world::MicroAddr &p_addr) {
		const uint64_t key = hf_world::pack_shard_key(p_addr.shard_q, p_addr.shard_r);
		Ref<hf_world::HexShardState> *existing = states.getptr(key);
		if (existing != nullptr) {
			return *existing;
		}
		Ref<hf_world::HexShardState> base = src->find_shard_state(key);
		if (base.is_null()) {
			return Ref<hf_world::HexShardState>();
		}
		Ref<hf_world::HexShardState> clone = base->clone_for_write(dst->get_version());
		states.insert(key, clone);
		return clone;
	}

	Ref<hf_world::HexShardPage> writable_page(const Ref<hf_world::HexShardState> &p_state, const hf_world::MicroAddr &p_addr) {
		const uint64_t key = hf_world::pack_shard_key(p_state->chunk_q, p_state->chunk_r);
		HashSet<int32_t> *copied = copied_pages.getptr(key);
		if (copied == nullptr) {
			copied_pages.insert(key, HashSet<int32_t>());
			copied = copied_pages.getptr(key);
		}
		if (!copied->has(p_addr.page)) {
			Ref<hf_world::HexShardPage> page;
			if (p_addr.page < p_state->pages.size() && p_state->pages[p_addr.page].is_valid()) {
				page = p_state->pages[p_addr.page]->clone(); // COW: copy on first write
			} else {
				page.instantiate();
				page->init_empty(p_addr.page * hf_world::PAGE_CELLS);
			}
			p_state->pages.write[p_addr.page] = page;
			copied->insert(p_addr.page);
		}
		return p_state->pages[p_addr.page];
	}

	void commit() {
		for (const KeyValue<uint64_t, Ref<hf_world::HexShardState>> &kv : states) {
			dst->put_shard_state(kv.key, kv.value);
		}
		result->next_snapshot = dst;
		result->new_version = dst->get_version();
	}

	void reject(const StringName &p_reason) {
		result->set_rejected(p_reason);
	}
};

} // namespace

Ref<HexCommandResult> HexCommandResolver::apply(const Ref<HexRegionSnapshot> &p_snapshot, const Ref<HexMapCommand> &p_cmd) {
	Ref<HexCommandResult> result;
	result.instantiate();
	result->accepted = false;

	ERR_FAIL_COND_V(p_snapshot.is_null(), result);
	ERR_FAIL_COND_V(p_cmd.is_null(), result);
	Ref<HexTerrainTableSet> tables = p_snapshot->get_run().is_valid() ? p_snapshot->get_run()->get_tables() : p_snapshot->tables;
	if (tables.is_null()) {
		result->set_rejected(StringName("missing_tables"));
		return result;
	}

	// Version gate (Invariant: callers stamp expected_version).
	if (p_cmd->get_expected_version() != 0 && p_cmd->get_expected_version() != p_snapshot->get_version()) {
		result->set_rejected(StringName("version_conflict"));
		return result;
	}

	const StringName op = p_cmd->get_operation();
	const Ref<MicroCoord> coord = p_cmd->get_coord();
	static const char *cell_ops[] = {
		"set_cell_base", "set_cell_elevation", "set_edge", "set_edge_enabled", "damage_edge",
		"place_object", "move_object", "damage_object", "destroy_object",
		"apply_area_effect", "remove_area_effect", "interact_cell", "interact_object", nullptr
	};
	bool known = false;
	for (int i = 0; cell_ops[i]; i++) {
		if (op == StringName(cell_ops[i])) {
			known = true;
			break;
		}
	}
	if (!known) {
		result->set_rejected(StringName("invalid_op"));
		return result;
	}
	if (coord.is_null()) {
		result->set_rejected(StringName("invalid_value"));
		return result;
	}

	Tx tx(p_snapshot, result);
	const hf_world::MicroAddr addr = hf_world::addr_from_micro(coord->get_q(), coord->get_r());
	const Dictionary payload = p_cmd->get_payload();

	auto fail = [&](const StringName &p_reason) {
		tx.reject(p_reason);
		return result;
	};

	auto require_cell = [&]() -> Ref<hf_world::HexShardPage> {
		Ref<hf_world::HexShardState> state = tx.writable_shard(addr);
		if (state.is_null()) {
			return Ref<hf_world::HexShardPage>();
		}
		Ref<hf_world::HexShardPage> page = tx.writable_page(state, addr);
		return page;
	};

	auto event = [&]() {
		Dictionary ev;
		ev[StringName("scale")] = StringName("micro");
		ev[StringName("operation")] = op;
		ev[StringName("coord")] = coord->to_dict();
		if (p_cmd->get_direction() >= 0) {
			ev[StringName("direction")] = p_cmd->get_direction();
		}
		if (p_cmd->get_source_id() != StringName()) {
			ev[StringName("source_id")] = p_cmd->get_source_id();
		}
		if (p_cmd->get_idempotency_id() != StringName()) {
			ev[StringName("idempotency_id")] = p_cmd->get_idempotency_id();
		}
		ev[StringName("payload")] = payload;
		return ev;
	};

	auto finish = [&]() {
		result->accepted = true;
		result->reason = StringName("ok");
		result->events.append(event());
		tx.commit();
	};

	// --- interact_* : capability-validated stamped events (verb pipeline, MAP-30) ---
	if (op == StringName("interact_cell") || op == StringName("interact_object")) {
		const StringName verb_id = payload.get(StringName("verb_id"), StringName());
		const int vidx = tables->find_verb(verb_id);
		if (verb_id == StringName() || vidx < 0) {
			return fail(StringName("invalid_value"));
		}
		Ref<HexVerbDef> verb = tables->get_verb(vidx);
		const PackedStringArray actor_tags = payload.get(StringName("actor_tags"), PackedStringArray());
		for (int i = 0; i < verb->get_required_hat_tags().size(); i++) {
			bool have = false;
			for (int j = 0; j < actor_tags.size(); j++) {
				if (actor_tags[j] == verb->get_required_hat_tags()[i]) {
					have = true;
					break;
				}
			}
			if (!have) {
				return fail(StringName("insufficient_capability"));
			}
		}
		const StringName need_key = verb->get_required_key_id();
		const StringName have_key = payload.get(StringName("key_id"), StringName());
		if (need_key != StringName() && have_key != need_key) {
			return fail(StringName("locked"));
		}
		finish();
		return result;
	}

	Ref<hf_world::HexShardPage> page = require_cell();
	if (page.is_null()) {
		return fail(StringName("out_of_bounds"));
	}
	if (page->base_terrain[addr.offset] == hf_world::NO_DATA) {
		return fail(StringName("no_data"));
	}

	// --- cell layer ops ---
	if (op == StringName("set_cell_base")) {
		const StringName terrain_id = payload.get(StringName("terrain_id"), StringName());
		const int idx = tables->find_base_terrain(terrain_id);
		if (idx < 0) {
			return fail(StringName("invalid_value"));
		}
		page->base_terrain.write[addr.offset] = idx;
		result->dirty_cells.append(coord->to_dict());
		finish();
		return result;
	}
	if (op == StringName("set_cell_elevation")) {
		const int elevation = payload.get(StringName("elevation"), 0);
		if (elevation < -8 || elevation > 8) {
			return fail(StringName("invalid_value"));
		}
		page->elevation.write[addr.offset] = elevation;
		result->dirty_cells.append(coord->to_dict());
		finish();
		return result;
	}

	// --- edge ops ---
	const int dir = p_cmd->get_direction();
	if (op == StringName("set_edge") || op == StringName("set_edge_enabled") || op == StringName("damage_edge")) {
		if (dir < 0 || dir >= hf_world::EDGE_SLOTS) {
			return fail(StringName("invalid_value"));
		}
		const int slot = addr.offset * hf_world::EDGE_SLOTS + dir;
		if (op == StringName("set_edge")) {
			const StringName type_id = payload.get(StringName("edge_type_id"), StringName());
			const int idx = tables->find_edge_type(type_id);
			if (idx < 0) {
				return fail(StringName("invalid_value"));
			}
			Ref<HexEdgeTypeDef> def = tables->get_edge_type(idx);
			page->edges.write[slot] = idx + 1;
			page->edges_enabled.write[slot] = payload.get(StringName("enabled"), true) ? 1 : 0;
			page->edges_durability.write[slot] = def->is_destructible() ? def->get_durability() : -1;
		} else if (op == StringName("set_edge_enabled")) {
			if (page->edges[slot] == 0) {
				return fail(StringName("missing_target"));
			}
			const bool enabling = payload.get(StringName("enabled"), true);
			if (enabling && page->edges_enabled[slot] == 0) {
				Ref<HexEdgeTypeDef> def = tables->get_edge_type(page->edges[slot] - 1);
				const StringName need_key = def->get_required_key_id();
				const StringName have_key = payload.get(StringName("key_id"), StringName());
				if (need_key != StringName() && have_key != need_key) {
					return fail(StringName("locked"));
				}
			}
			page->edges_enabled.write[slot] = enabling ? 1 : 0;
		} else { // damage_edge
			const int amount = payload.get(StringName("amount"), 0);
			if (amount <= 0) {
				return fail(StringName("invalid_value"));
			}
			if (page->edges[slot] == 0) {
				return fail(StringName("missing_target"));
			}
			Ref<HexEdgeTypeDef> def = tables->get_edge_type(page->edges[slot] - 1);
			if (!def->is_destructible() || page->edges_durability[slot] < 0) {
				return fail(StringName("not_destructible"));
			}
			const int remaining = page->edges_durability[slot] - amount;
			if (remaining > 0) {
				page->edges_durability.write[slot] = remaining;
			} else {
				const StringName form = def->get_destroyed_form();
				const int form_idx = form != StringName() ? tables->find_edge_type(form) : -1;
				if (form_idx >= 0) {
					Ref<HexEdgeTypeDef> form_def = tables->get_edge_type(form_idx);
					page->edges.write[slot] = form_idx + 1;
					page->edges_enabled.write[slot] = 1;
					page->edges_durability.write[slot] = form_def->is_destructible() ? form_def->get_durability() : -1;
				} else {
					page->edges.write[slot] = 0;
					page->edges_enabled.write[slot] = 0;
					page->edges_durability.write[slot] = -1;
				}
			}
		}
		Dictionary de;
		de[StringName("coord")] = coord->to_dict();
		de[StringName("direction")] = dir;
		result->dirty_edges.append(de);
		result->dirty_cells.append(coord->to_dict());
		finish();
		return result;
	}

	// --- object ops ---
	if (op == StringName("place_object")) {
		const StringName object_id = payload.get(StringName("object_id"), StringName());
		const int idx = tables->find_object(object_id);
		if (idx < 0) {
			return fail(StringName("invalid_value"));
		}
		if (page->object[addr.offset] != 0) {
			return fail(StringName("occupied"));
		}
		Ref<HexObjectDef> def = tables->get_object(idx);
		page->object.write[addr.offset] = idx + 1;
		page->object_durability.write[addr.offset] = def->is_destructible() ? def->get_durability() : -1;
		result->dirty_cells.append(coord->to_dict());
		finish();
		return result;
	}
	if (op == StringName("move_object")) {
		if (page->object[addr.offset] == 0) {
			return fail(StringName("missing_target"));
		}
		Dictionary to_dict = payload.get(StringName("to_coord"), Dictionary());
		if (to_dict.is_empty()) {
			return fail(StringName("invalid_value"));
		}
		Ref<MicroCoord> to = MicroCoord::from_dict(to_dict);
		const hf_world::MicroAddr to_addr = hf_world::addr_from_micro(to->get_q(), to->get_r());
		Ref<hf_world::HexShardState> to_state = tx.writable_shard(to_addr);
		if (to_state.is_null()) {
			return fail(StringName("out_of_bounds"));
		}
		Ref<hf_world::HexShardPage> to_page = tx.writable_page(to_state, to_addr);
		if (to_page->base_terrain[to_addr.offset] == hf_world::NO_DATA) {
			return fail(StringName("no_data"));
		}
		if (to_page->object[to_addr.offset] != 0) {
			return fail(StringName("occupied"));
		}
		to_page->object.write[to_addr.offset] = page->object[addr.offset];
		to_page->object_durability.write[to_addr.offset] = page->object_durability[addr.offset];
		page->object.write[addr.offset] = 0;
		page->object_durability.write[addr.offset] = -1;
		result->dirty_cells.append(coord->to_dict());
		result->dirty_cells.append(to->to_dict());
		finish();
		return result;
	}
	if (op == StringName("damage_object")) {
		const int amount = payload.get(StringName("amount"), 0);
		if (amount <= 0) {
			return fail(StringName("invalid_value"));
		}
		if (page->object[addr.offset] == 0) {
			return fail(StringName("missing_target"));
		}
		Ref<HexObjectDef> def = tables->get_object(page->object[addr.offset] - 1);
		if (!def->is_destructible() || page->object_durability[addr.offset] < 0) {
			return fail(StringName("not_destructible"));
		}
		const int remaining = page->object_durability[addr.offset] - amount;
		if (remaining > 0) {
			page->object_durability.write[addr.offset] = remaining;
		} else {
			const StringName form = def->get_destroyed_form();
			const int form_idx = form != StringName() ? tables->find_object(form) : -1;
			if (form_idx >= 0) {
				Ref<HexObjectDef> form_def = tables->get_object(form_idx);
				page->object.write[addr.offset] = form_idx + 1;
				page->object_durability.write[addr.offset] = form_def->is_destructible() ? form_def->get_durability() : -1;
			} else {
				page->object.write[addr.offset] = 0;
				page->object_durability.write[addr.offset] = -1;
			}
		}
		result->dirty_cells.append(coord->to_dict());
		finish();
		return result;
	}
	if (op == StringName("destroy_object")) {
		if (page->object[addr.offset] == 0) {
			return fail(StringName("missing_target"));
		}
		page->object.write[addr.offset] = 0;
		page->object_durability.write[addr.offset] = -1;
		result->dirty_cells.append(coord->to_dict());
		finish();
		return result;
	}

	// --- area effect ops ---
	if (op == StringName("apply_area_effect")) {
		const StringName effect_id = payload.get(StringName("effect_id"), StringName());
		const int idx = tables->find_area_effect(effect_id);
		if (idx < 0 || idx >= 64) {
			return fail(StringName("invalid_value"));
		}
		page->area_effects.write[addr.offset] |= (1ULL << idx);
		result->dirty_cells.append(coord->to_dict());
		finish();
		return result;
	}
	if (op == StringName("remove_area_effect")) {
		const StringName effect_id = payload.get(StringName("effect_id"), StringName());
		const int idx = tables->find_area_effect(effect_id);
		if (idx < 0 || idx >= 64) {
			return fail(StringName("invalid_value"));
		}
		if ((page->area_effects[addr.offset] & (1ULL << idx)) == 0) {
			return fail(StringName("missing_target"));
		}
		page->area_effects.write[addr.offset] &= ~(1ULL << idx);
		result->dirty_cells.append(coord->to_dict());
		finish();
		return result;
	}

	return fail(StringName("invalid_op"));
}
