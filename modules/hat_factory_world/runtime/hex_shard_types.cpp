/**************************************************************************/
/*  hex_shard_types.cpp                                                   */
/**************************************************************************/
#include "hex_shard_types.h"

#include "core/templates/hashfuncs.h"

namespace hf_world {

void HexShardPage::init_empty(int32_t p_base_offset) {
	base_offset = p_base_offset;
	base_terrain.resize(PAGE_CELLS);
	elevation.resize(PAGE_CELLS);
	surfaces.resize(PAGE_CELLS);
	object.resize(PAGE_CELLS);
	object_durability.resize(PAGE_CELLS);
	edges.resize(PAGE_CELLS * EDGE_SLOTS);
	edges_enabled.resize(PAGE_CELLS * EDGE_SLOTS);
	area_effects.resize(PAGE_CELLS);
	base_terrain.fill(NO_DATA);
	elevation.fill(0);
	surfaces.fill(0);
	object.fill(0);
	object_durability.fill(-1);
	edges.fill(0);
	edges_enabled.fill(0);
	area_effects.fill(0);
}

bool HexShardPage::is_empty() const {
	for (int i = 0; i < PAGE_CELLS; i++) {
		if (base_terrain[i] != NO_DATA || object[i] != 0 || surfaces[i] != 0 || area_effects[i] != 0) {
			return false;
		}
	}
	for (int i = 0; i < PAGE_CELLS * EDGE_SLOTS; i++) {
		if (edges[i] != 0) {
			return false;
		}
	}
	return true;
}

Dictionary HexShardPage::to_dict() const {
	Dictionary d;
	d[StringName("base_offset")] = base_offset;
	d[StringName("base_terrain")] = base_terrain;
	d[StringName("elevation")] = elevation;
	d[StringName("surfaces")] = surfaces;
	d[StringName("object")] = object;
	d[StringName("object_durability")] = object_durability;
	d[StringName("edges")] = edges;
	d[StringName("edges_enabled")] = edges_enabled;
	d[StringName("area_effects")] = area_effects;
	return d;
}

Ref<HexShardPage> HexShardPage::from_dict(const Dictionary &p_dict) {
	Ref<HexShardPage> p;
	p.instantiate();
	p->base_offset = p_dict.get(StringName("base_offset"), 0);
	p->base_terrain = p_dict.get(StringName("base_terrain"), PackedInt32Array());
	p->elevation = p_dict.get(StringName("elevation"), PackedInt32Array());
	p->surfaces = p_dict.get(StringName("surfaces"), PackedInt64Array());
	p->object = p_dict.get(StringName("object"), PackedInt32Array());
	p->object_durability = p_dict.get(StringName("object_durability"), PackedInt32Array());
	p->edges = p_dict.get(StringName("edges"), PackedInt32Array());
	p->edges_enabled = p_dict.get(StringName("edges_enabled"), PackedInt32Array());
	p->area_effects = p_dict.get(StringName("area_effects"), PackedInt64Array());
	return p;
}

Ref<HexShardState> HexShardState::clone_for_write(int64_t p_new_version) const {
	Ref<HexShardState> s;
	s.instantiate();
	s->chunk_q = chunk_q;
	s->chunk_r = chunk_r;
	s->pages = pages; // share immutable pages; writers replace affected entries
	s->features = features.duplicate(); // COW the (small) feature registry
	s->version = p_new_version;
	return s;
}

Dictionary HexShardState::to_dict() const {
	Dictionary d;
	d[StringName("chunk_q")] = chunk_q;
	d[StringName("chunk_r")] = chunk_r;
	d[StringName("version")] = version;
	Dictionary pages_dict;
	for (int i = 0; i < pages.size(); i++) {
		if (pages[i].is_valid()) {
			pages_dict[i] = pages[i]->to_dict();
		}
	}
	d[StringName("pages")] = pages_dict;
	d[StringName("features")] = features;
	return d;
}

Ref<HexShardState> HexShardState::from_dict(const Dictionary &p_dict) {
	Ref<HexShardState> s;
	s.instantiate();
	s->chunk_q = p_dict.get(StringName("chunk_q"), 0);
	s->chunk_r = p_dict.get(StringName("chunk_r"), 0);
	s->version = p_dict.get(StringName("version"), (int64_t)0);
	Dictionary pages_dict = p_dict.get(StringName("pages"), Dictionary());
	const LocalVector<Variant> keys = pages_dict.get_key_list();
	s->pages.resize(PAGE_COUNT);
	for (const Variant &k : keys) {
		const int idx = (int)k;
		if (idx >= 0 && idx < PAGE_COUNT) {
			s->pages[idx] = HexShardPage::from_dict(pages_dict[k]);
		}
	}
	s->features = p_dict.get(StringName("features"), Dictionary());
	return s;
}

} // namespace hf_world
