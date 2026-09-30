/**************************************************************************/
/*  hex_world_run.cpp                                                     */
/**************************************************************************/
#include "hex_world_run.h"

#include "../data/hex_detail_template.h"
#include "hex_command_resolver.h"

#include "core/object/class_db.h"
#include "core/templates/hashfuncs.h"

void HexWorldRun::_bind_methods() {
	ClassDB::bind_static_method("HexWorldRun", D_METHOD("create", "world_template", "detail_templates", "tables", "seed"), &HexWorldRun::create);
	ClassDB::bind_method(D_METHOD("get_seed"), &HexWorldRun::get_seed);
	ClassDB::bind_method(D_METHOD("get_world_template"), &HexWorldRun::get_world_template);
	ClassDB::bind_method(D_METHOD("get_tables"), &HexWorldRun::get_tables);
	ClassDB::bind_method(D_METHOD("get_journal"), &HexWorldRun::get_journal);
	ClassDB::bind_method(D_METHOD("get_map_id"), &HexWorldRun::get_map_id);
	ClassDB::bind_method(D_METHOD("get_template_version"), &HexWorldRun::get_template_version);
	ClassDB::bind_method(D_METHOD("get_global_seq"), &HexWorldRun::get_global_seq);
	ClassDB::bind_method(D_METHOD("get_base_shard_state", "key"), &HexWorldRun::get_base_shard_state);
	ClassDB::bind_method(D_METHOD("get_committed_snapshot"), &HexWorldRun::get_committed_snapshot);
	ClassDB::bind_method(D_METHOD("new_battle_snapshot"), &HexWorldRun::new_battle_snapshot);
	ClassDB::bind_method(D_METHOD("apply_command", "command"), &HexWorldRun::apply_command);
	ClassDB::bind_method(D_METHOD("alloc_feature_seq"), &HexWorldRun::alloc_feature_seq);
	ClassDB::bind_method(D_METHOD("register_feature", "seq", "kind", "coord"), &HexWorldRun::register_feature);
	ClassDB::bind_method(D_METHOD("move_feature", "seq", "coord"), &HexWorldRun::move_feature);
	ClassDB::bind_method(D_METHOD("unregister_feature", "seq"), &HexWorldRun::unregister_feature);
	ClassDB::bind_method(D_METHOD("get_feature", "seq"), &HexWorldRun::get_feature);
	ClassDB::bind_method(D_METHOD("get_manifest_dict"), &HexWorldRun::get_manifest_dict);
}

static Ref<hf_world::HexShardState> _ensure_building_shard(HashMap<uint64_t, Ref<hf_world::HexShardState>> &r_building, const hf_world::MicroAddr &p_addr) {
	const uint64_t key = hf_world::pack_shard_key(p_addr.shard_q, p_addr.shard_r);
	Ref<hf_world::HexShardState> *existing = r_building.getptr(key);
	if (existing != nullptr) {
		return *existing;
	}
	Ref<hf_world::HexShardState> s;
	s.instantiate();
	s->chunk_q = p_addr.shard_q;
	s->chunk_r = p_addr.shard_r;
	s->pages.resize(hf_world::PAGE_COUNT);
	s->version = 0;
	r_building.insert(key, s);
	return s;
}

static bool _fill_cell(const Ref<HexTerrainTableSet> &p_tables, const Ref<hf_world::HexShardState> &p_state, const Ref<HexDetailCellDef> &p_cell, String &r_error) {
	const Ref<MicroCoord> mc = p_cell->get_coord();
	const hf_world::MicroAddr addr = hf_world::addr_from_micro(mc->get_q(), mc->get_r());
	if (p_state->pages[addr.page].is_null()) {
		Ref<hf_world::HexShardPage> page;
		page.instantiate();
		page->init_empty(addr.page * hf_world::PAGE_CELLS);
		p_state->pages.write[addr.page] = page;
	}
	Ref<hf_world::HexShardPage> page = p_state->pages[addr.page];
	const int base_idx = p_tables->find_base_terrain(p_cell->get_base_terrain());
	if (base_idx < 0) {
		r_error = "detail cell (" + itos(mc->get_q()) + "," + itos(mc->get_r()) + ") references unknown base terrain '" + String(p_cell->get_base_terrain()) + "'";
		return false;
	}
	page->base_terrain.write[addr.offset] = base_idx;
	page->elevation.write[addr.offset] = p_cell->get_elevation();
	uint64_t surf_mask = 0;
	const PackedStringArray surf_ids = p_cell->get_surface_ids();
	for (int i = 0; i < surf_ids.size(); i++) {
		const int idx = p_tables->find_surface(surf_ids[i]);
		if (idx < 0) {
			r_error = "detail cell references unknown surface '" + String(surf_ids[i]) + "'";
			return false;
		}
		surf_mask |= (1ULL << idx);
	}
	page->surfaces.write[addr.offset] = surf_mask;
	const StringName obj_id = p_cell->get_object_id();
	if (obj_id != StringName()) {
		const int idx = p_tables->find_object(obj_id);
		if (idx < 0) {
			r_error = "detail cell references unknown object '" + String(obj_id) + "'";
			return false;
		}
		Ref<HexObjectDef> obj_def = p_tables->get_object(idx);
		page->object.write[addr.offset] = idx + 1;
		page->object_durability.write[addr.offset] = obj_def->is_destructible() ? obj_def->get_durability() : -1;
	}
	const PackedStringArray edge_ids = p_cell->get_edge_type_ids();
	for (int d = 0; d < edge_ids.size() && d < hf_world::EDGE_SLOTS; d++) {
		if (edge_ids[d] == StringName()) {
			continue;
		}
		const int idx = p_tables->find_edge_type(edge_ids[d]);
		if (idx < 0) {
			r_error = "detail cell references unknown edge type '" + String(edge_ids[d]) + "'";
			return false;
		}
		Ref<HexEdgeTypeDef> edge_def = p_tables->get_edge_type(idx);
		const int slot = addr.offset * hf_world::EDGE_SLOTS + d;
		page->edges.write[slot] = idx + 1;
		page->edges_enabled.write[slot] = edge_def->get_enabled_by_default() ? 1 : 0;
		page->edges_durability.write[slot] = edge_def->is_destructible() ? edge_def->get_durability() : -1;
	}
	uint64_t fx_mask = 0;
	const PackedStringArray fx_ids = p_cell->get_area_effect_ids();
	for (int i = 0; i < fx_ids.size(); i++) {
		const int idx = p_tables->find_area_effect(fx_ids[i]);
		if (idx < 0) {
			r_error = "detail cell references unknown area effect '" + String(fx_ids[i]) + "'";
			return false;
		}
		fx_mask |= (1ULL << idx);
	}
	page->area_effects.write[addr.offset] = fx_mask;
	return true;
}

bool HexWorldRun::_materialize_base(String &r_error) {
	if (tables.is_null() || !tables->compile()) {
		r_error = tables.is_null() ? "missing tables" : tables->get_compile_error();
		return false;
	}
	HashMap<uint64_t, Ref<hf_world::HexShardState>> building;
	// 1) Generic detail templates (designer baselines).
	for (int t = 0; t < detail_templates.size(); t++) {
		Ref<HexDetailTemplate> dt = detail_templates[t];
		if (dt.is_null()) {
			r_error = "null detail template at index " + itos(t);
			return false;
		}
		if (dt->get_map_id() != world_template->get_map_id()) {
			r_error = "detail template map_id '" + String(dt->get_map_id()) + "' != world map_id '" + String(world_template->get_map_id()) + "'";
			return false;
		}
		const TypedArray<HexDetailCellDef> cells = dt->get_cells();
		for (int i = 0; i < cells.size(); i++) {
			Ref<HexDetailCellDef> cell = cells[i];
			if (cell.is_null() || cell->get_coord().is_null()) {
				r_error = "detail template has null cell at index " + itos(i);
				return false;
			}
			const hf_world::MicroAddr addr = hf_world::addr_from_micro(cell->get_coord()->get_q(), cell->get_coord()->get_r());
			Ref<hf_world::HexShardState> state = _ensure_building_shard(building, addr);
			if (!_fill_cell(tables, state, cell, r_error)) {
				return false;
			}
		}
	}
	// 2) World-cell detail overrides (designer overrides win, MAP-42).
	const TypedArray<HexWorldCellDef> world_cells = world_template->get_cells();
	for (int i = 0; i < world_cells.size(); i++) {
		Ref<HexWorldCellDef> wc = world_cells[i];
		if (wc.is_null() || wc->get_detail_override().is_null()) {
			continue;
		}
		const TypedArray<HexDetailCellDef> cells = wc->get_detail_override()->get_cells();
		for (int j = 0; j < cells.size(); j++) {
			Ref<HexDetailCellDef> cell = cells[j];
			if (cell.is_null() || cell->get_coord().is_null()) {
				r_error = "world cell detail override has null cell";
				return false;
			}
			const hf_world::MicroAddr addr = hf_world::addr_from_micro(cell->get_coord()->get_q(), cell->get_coord()->get_r());
			Ref<hf_world::HexShardState> state = _ensure_building_shard(building, addr);
			if (!_fill_cell(tables, state, cell, r_error)) {
				return false;
			}
		}
	}
	base_shards = building;
	committed.instantiate();
	committed->tables = tables;
	committed->version = 0;
	return true;
}

Ref<HexWorldRun> HexWorldRun::create(const Ref<HexWorldTemplate> &p_world_template, const TypedArray<HexDetailTemplate> &p_detail_templates, const Ref<HexTerrainTableSet> &p_tables, int64_t p_seed) {
	ERR_FAIL_COND_V(p_world_template.is_null(), Ref<HexWorldRun>());
	const String verr = p_world_template->validate();
	ERR_FAIL_COND_V_MSG(!verr.is_empty(), Ref<HexWorldRun>(), "HexWorldRun::create: world template invalid: " + verr);
	Ref<HexWorldRun> run;
	run.instantiate();
	run->world_template = p_world_template;
	run->detail_templates = p_detail_templates;
	run->tables = p_tables;
	run->seed = p_seed;
	run->journal.instantiate();
	String err;
	ERR_FAIL_COND_V_MSG(!run->_materialize_base(err), Ref<HexWorldRun>(), "HexWorldRun::create: materialize failed: " + err);
	run->committed->set_run(run);
	return run;
}

Ref<HexWorldRun> HexWorldRun::_create_for_load(const Ref<HexWorldTemplate> &p_world_template, const TypedArray<HexDetailTemplate> &p_detail_templates, const Ref<HexTerrainTableSet> &p_tables, int64_t p_seed) {
	// Same as create; the committed view is replaced by _restore_committed.
	return create(p_world_template, p_detail_templates, p_tables, p_seed);
}

void HexWorldRun::_restore_committed(const Ref<HexWorldRun> &p_self, const Ref<HexRegionSnapshot> &p_snapshot, int64_t p_global_seq, const Ref<HexEventJournal> &p_journal, const HashMap<int64_t, Dictionary> &p_features, int64_t p_next_feature_seq) {
	committed = p_snapshot;
	committed->set_run(p_self);
	global_seq = p_global_seq;
	journal = p_journal;
	features = p_features;
	next_feature_seq = p_next_feature_seq;
}

void HexWorldRun::set_seed(int64_t p_seed) { seed = p_seed; }
int64_t HexWorldRun::get_seed() const { return seed; }
Ref<HexWorldTemplate> HexWorldRun::get_world_template() const { return world_template; }
Ref<HexTerrainTableSet> HexWorldRun::get_tables() const { return tables; }
Ref<HexEventJournal> HexWorldRun::get_journal() const { return journal; }
StringName HexWorldRun::get_map_id() const { return world_template.is_valid() ? world_template->get_map_id() : StringName(); }
Ref<TemplateVersion> HexWorldRun::get_template_version() const { return world_template.is_valid() ? world_template->get_template_version() : Ref<TemplateVersion>(); }
int64_t HexWorldRun::get_global_seq() const { return global_seq; }

Ref<hf_world::HexShardState> HexWorldRun::get_base_shard_state(uint64_t p_key) const {
	const Ref<hf_world::HexShardState> *s = base_shards.getptr(p_key);
	return s ? *s : Ref<hf_world::HexShardState>();
}

Ref<HexRegionSnapshot> HexWorldRun::get_committed_snapshot() const { return committed; }

Ref<HexRegionSnapshot> HexWorldRun::new_battle_snapshot() const {
	ERR_FAIL_COND_V(committed.is_null(), Ref<HexRegionSnapshot>());
	return committed->clone_shallow(global_seq);
}

Ref<HexCommandResult> HexWorldRun::apply_command(const Ref<HexMapCommand> &p_command) {
	ERR_FAIL_COND_V(committed.is_null(), Ref<HexCommandResult>());
	Ref<HexCommandResult> result = HexCommandResolver::apply(committed, p_command);
	if (!result->accepted) {
		return result; // atomic rejection: no state, seq, item or AP change
	}
	committed = result->next_snapshot;
	global_seq = result->new_version;
	// Stable feature registry upkeep from the committed event.
	for (int i = 0; i < result->events.size(); i++) {
		Dictionary ev = result->events[i];
		Dictionary payload = ev.get(StringName("payload"), Dictionary());
		const StringName op = ev.get(StringName("operation"), StringName());
		if (op == StringName("place_object") && payload.has(StringName("feature_kind"))) {
			int64_t seq = payload.get(StringName("feature_seq"), (int64_t)0);
			if (seq <= 0) {
				seq = alloc_feature_seq();
			}
			Dictionary coord = ev.get(StringName("coord"), Dictionary());
			register_feature(seq, payload.get(StringName("feature_kind"), StringName()), MicroCoord::from_dict(coord));
		} else if (op == StringName("move_object") && payload.has(StringName("feature_seq"))) {
			Dictionary to = payload.get(StringName("to_coord"), Dictionary());
			move_feature(payload.get(StringName("feature_seq"), (int64_t)0), MicroCoord::from_dict(to));
		} else if ((op == StringName("destroy_object") || op == StringName("damage_object")) && payload.has(StringName("feature_seq"))) {
			Dictionary current = get_feature(payload.get(StringName("feature_seq"), (int64_t)0));
			const StringName bound = current.get(StringName("object"), StringName());
			if (!current.is_empty() && bound != StringName()) {
				current[StringName("object")] = StringName();
				features[payload.get(StringName("feature_seq"), (int64_t)0)] = current;
			}
		}
		if (journal.is_valid()) {
			journal->append(ev);
		}
	}
	return result;
}

int64_t HexWorldRun::alloc_feature_seq() { return next_feature_seq++; }

bool HexWorldRun::register_feature(int64_t p_seq, const StringName &p_kind, const Ref<MicroCoord> &p_coord) {
	if (p_seq <= 0) {
		return false;
	}
	Dictionary rec;
	rec[StringName("kind")] = p_kind;
	rec[StringName("coord")] = p_coord.is_valid() ? p_coord->to_dict() : Dictionary();
	rec[StringName("object")] = p_kind;
	features[p_seq] = rec;
	return true;
}

bool HexWorldRun::move_feature(int64_t p_seq, const Ref<MicroCoord> &p_coord) {
	Dictionary *rec = features.getptr(p_seq);
	if (rec == nullptr) {
		return false;
	}
	(*rec)[StringName("coord")] = p_coord.is_valid() ? p_coord->to_dict() : Dictionary();
	return true;
}

bool HexWorldRun::unregister_feature(int64_t p_seq) { return features.erase(p_seq); }

Dictionary HexWorldRun::get_feature(int64_t p_seq) const {
	const Dictionary *rec = features.getptr(p_seq);
	return rec ? *rec : Dictionary();
}

Dictionary HexWorldRun::get_manifest_dict() const {
	Dictionary m;
	m[StringName("schema_version")] = SCHEMA_VERSION;
	m[StringName("generator_version")] = GENERATOR_VERSION;
	m[StringName("map_id")] = get_map_id();
	if (world_template.is_valid() && world_template->get_template_version().is_valid()) {
		m[StringName("template_version")] = world_template->get_template_version()->to_dict();
	}
	m[StringName("seed")] = seed;
	m[StringName("last_seq")] = global_seq;
	// Order-independent combine: HashMap iteration order is not stable across
	// save/load instances, so per-shard contributions are XOR-mixed.
	uint32_t acc = hash_murmur3_one_64((uint64_t)global_seq);
	acc = hash_murmur3_one_64((uint64_t)seed, acc);
	acc = hash_murmur3_one_32(get_map_id().hash(), acc);
	for (const KeyValue<uint64_t, Ref<hf_world::HexShardState>> &kv : committed->shards) {
		uint32_t s = hash_murmur3_one_32((uint32_t)kv.key);
		s = hash_murmur3_one_64((uint64_t)kv.value->version, s);
		s = hash_murmur3_one_32((uint32_t)kv.value->features.size(), s);
		acc ^= s;
	}
	m[StringName("checksum")] = (int64_t)hash_fmix32(acc);
	return m;
}
