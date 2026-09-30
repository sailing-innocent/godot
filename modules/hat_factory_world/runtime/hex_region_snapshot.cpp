/**************************************************************************/
/*  hex_region_snapshot.cpp                                               */
/**************************************************************************/
#include "hex_region_snapshot.h"

#include "hex_world_run.h"

#include "core/object/class_db.h"

void HexRegionSnapshot::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_run", "run"), &HexRegionSnapshot::set_run);
	ClassDB::bind_method(D_METHOD("get_run"), &HexRegionSnapshot::get_run);
	ClassDB::bind_method(D_METHOD("has_cell_data", "coord"), &HexRegionSnapshot::has_cell_data);
	ClassDB::bind_method(D_METHOD("get_base_terrain_at", "coord"), &HexRegionSnapshot::get_base_terrain_at);
	ClassDB::bind_method(D_METHOD("get_elevation_at", "coord"), &HexRegionSnapshot::get_elevation_at);
	ClassDB::bind_method(D_METHOD("get_cell_view", "coord"), &HexRegionSnapshot::get_cell_view);
	ClassDB::bind_method(D_METHOD("get_version"), &HexRegionSnapshot::get_version);
	ClassDB::bind_method(D_METHOD("get_shard_count"), &HexRegionSnapshot::get_shard_count);
	ClassDB::bind_method(D_METHOD("clone_shallow", "new_version"), &HexRegionSnapshot::clone_shallow);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexRegionSnapshot::to_dict);
}

void HexRegionSnapshot::set_run(const Ref<HexWorldRun> &p_run) { run = p_run; }
Ref<HexWorldRun> HexRegionSnapshot::get_run() const { return run; }

Ref<hf_world::HexShardState> HexRegionSnapshot::find_shard_state(uint64_t p_key) const {
	const Ref<hf_world::HexShardState> *s = shards.getptr(p_key);
	if (s != nullptr) {
		return *s;
	}
	if (run.is_valid()) {
		return run->get_base_shard_state(p_key);
	}
	return Ref<hf_world::HexShardState>();
}

Ref<hf_world::HexShardPage> HexRegionSnapshot::find_page(const hf_world::MicroAddr &p_addr) const {
	Ref<hf_world::HexShardState> state = find_shard_state(hf_world::pack_shard_key(p_addr.shard_q, p_addr.shard_r));
	if (state.is_null() || p_addr.page >= state->pages.size()) {
		return Ref<hf_world::HexShardPage>();
	}
	return state->pages[p_addr.page];
}

bool HexRegionSnapshot::has_cell_data(const Ref<MicroCoord> &p_coord) const {
	if (p_coord.is_null()) {
		return false;
	}
	const hf_world::MicroAddr addr = hf_world::addr_from_micro(p_coord->get_q(), p_coord->get_r());
	Ref<hf_world::HexShardPage> page = find_page(addr);
	if (page.is_null()) {
		return false;
	}
	return page->base_terrain[addr.offset] != hf_world::NO_DATA;
}

StringName HexRegionSnapshot::get_base_terrain_at(const Ref<MicroCoord> &p_coord) const {
	if (tables.is_null() || p_coord.is_null()) {
		return StringName();
	}
	const hf_world::MicroAddr addr = hf_world::addr_from_micro(p_coord->get_q(), p_coord->get_r());
	Ref<hf_world::HexShardPage> page = find_page(addr);
	if (page.is_null()) {
		return StringName();
	}
	const int32_t idx = page->base_terrain[addr.offset];
	if (idx == hf_world::NO_DATA) {
		return StringName();
	}
	Ref<HexBaseTerrainDef> def = tables->get_base_terrain(idx);
	return def.is_valid() ? def->get_def_id() : StringName();
}

int HexRegionSnapshot::get_elevation_at(const Ref<MicroCoord> &p_coord) const {
	if (p_coord.is_null()) {
		return 0;
	}
	const hf_world::MicroAddr addr = hf_world::addr_from_micro(p_coord->get_q(), p_coord->get_r());
	Ref<hf_world::HexShardPage> page = find_page(addr);
	return page.is_null() ? 0 : page->elevation[addr.offset];
}

Dictionary HexRegionSnapshot::get_cell_view(const Ref<MicroCoord> &p_coord) const {
	Dictionary d;
	if (p_coord.is_null() || tables.is_null()) {
		return d;
	}
	const hf_world::MicroAddr addr = hf_world::addr_from_micro(p_coord->get_q(), p_coord->get_r());
	Ref<hf_world::HexShardPage> page = find_page(addr);
	if (page.is_null()) {
		return d;
	}
	const int32_t base_idx = page->base_terrain[addr.offset];
	d[StringName("coord")] = p_coord->to_dict();
	d[StringName("has_data")] = base_idx != hf_world::NO_DATA;
	if (base_idx != hf_world::NO_DATA) {
		Ref<HexBaseTerrainDef> def = tables->get_base_terrain(base_idx);
		d[StringName("base_terrain")] = def.is_valid() ? def->get_def_id() : StringName();
	}
	d[StringName("elevation")] = page->elevation[addr.offset];
	const uint64_t surf_mask = page->surfaces[addr.offset];
	PackedStringArray surf_ids;
	for (int i = 0; i < 64; i++) {
		if (surf_mask & (1ULL << i)) {
			Ref<HexSurfaceDef> def = tables->get_surface(i);
			if (def.is_valid()) {
				surf_ids.append(def->get_def_id());
			}
		}
	}
	d[StringName("surfaces")] = surf_ids;
	const int32_t obj_idx = page->object[addr.offset];
	if (obj_idx > 0) {
		Ref<HexObjectDef> def = tables->get_object(obj_idx - 1);
		d[StringName("object")] = def.is_valid() ? def->get_def_id() : StringName();
		d[StringName("object_durability")] = page->object_durability[addr.offset];
	} else {
		d[StringName("object")] = StringName();
	}
	Array edges;
	for (int dir = 0; dir < hf_world::EDGE_SLOTS; dir++) {
		const int32_t e = page->edges[addr.offset * hf_world::EDGE_SLOTS + dir];
		Dictionary ed;
		if (e > 0) {
			Ref<HexEdgeTypeDef> def = tables->get_edge_type(e - 1);
			ed[StringName("type")] = def.is_valid() ? def->get_def_id() : StringName();
			ed[StringName("enabled")] = page->edges_enabled[addr.offset * hf_world::EDGE_SLOTS + dir] != 0;
		} else {
			ed[StringName("type")] = StringName();
			ed[StringName("enabled")] = true;
		}
		edges.append(ed);
	}
	d[StringName("edges")] = edges;
	const uint64_t fx_mask = page->area_effects[addr.offset];
	PackedStringArray fx_ids;
	for (int i = 0; i < 64; i++) {
		if (fx_mask & (1ULL << i)) {
			Ref<HexAreaEffectDef> def = tables->get_area_effect(i);
			if (def.is_valid()) {
				fx_ids.append(def->get_def_id());
			}
		}
	}
	d[StringName("area_effects")] = fx_ids;
	return d;
}

int64_t HexRegionSnapshot::get_version() const { return version; }
int HexRegionSnapshot::get_shard_count() const { return shards.size(); }

Ref<HexRegionSnapshot> HexRegionSnapshot::clone_shallow(int64_t p_new_version) const {
	Ref<HexRegionSnapshot> s;
	s.instantiate();
	s->run = run;
	s->tables = tables;
	s->version = p_new_version;
	for (const KeyValue<uint64_t, Ref<hf_world::HexShardState>> &kv : shards) {
		s->shards.insert(kv.key, kv.value);
	}
	return s;
}

void HexRegionSnapshot::put_shard_state(uint64_t p_key, const Ref<hf_world::HexShardState> &p_state) {
	shards.insert(p_key, p_state);
}

Dictionary HexRegionSnapshot::to_dict() const {
	Dictionary d;
	d[StringName("version")] = version;
	Dictionary shards_dict;
	for (const KeyValue<uint64_t, Ref<hf_world::HexShardState>> &kv : shards) {
		shards_dict[(int64_t)kv.key] = kv.value->to_dict();
	}
	d[StringName("shards")] = shards_dict;
	return d;
}

Ref<HexRegionSnapshot> HexRegionSnapshot::from_dict(const Dictionary &p_dict, const Ref<HexTerrainTableSet> &p_tables) {
	Ref<HexRegionSnapshot> s;
	s.instantiate();
	s->tables = p_tables;
	s->version = p_dict.get(StringName("version"), (int64_t)0);
	Dictionary shards_dict = p_dict.get(StringName("shards"), Dictionary());
	const LocalVector<Variant> keys = shards_dict.get_key_list();
	for (const Variant &k : keys) {
		s->shards.insert((uint64_t)(int64_t)k, hf_world::HexShardState::from_dict(shards_dict[k]));
	}
	return s;
}
