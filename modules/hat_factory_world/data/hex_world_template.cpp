/**************************************************************************/
/*  hex_world_template.cpp                                                */
/**************************************************************************/
#include "hex_world_template.h"

#include "hex_detail_template.h"

#include "core/object/class_db.h"

/**************************************************************************/
/* HexWorldCellDef                                                        */
/**************************************************************************/

void HexWorldCellDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_coord", "coord"), &HexWorldCellDef::set_coord);
	ClassDB::bind_method(D_METHOD("get_coord"), &HexWorldCellDef::get_coord);
	ClassDB::bind_method(D_METHOD("set_biome", "biome"), &HexWorldCellDef::set_biome);
	ClassDB::bind_method(D_METHOD("get_biome"), &HexWorldCellDef::get_biome);
	ClassDB::bind_method(D_METHOD("set_region_summary", "summary"), &HexWorldCellDef::set_region_summary);
	ClassDB::bind_method(D_METHOD("get_region_summary"), &HexWorldCellDef::get_region_summary);
	ClassDB::bind_method(D_METHOD("set_tags", "tags"), &HexWorldCellDef::set_tags);
	ClassDB::bind_method(D_METHOD("get_tags"), &HexWorldCellDef::get_tags);
	ClassDB::bind_method(D_METHOD("has_tag", "tag"), &HexWorldCellDef::has_tag);
	ClassDB::bind_method(D_METHOD("set_chunk_id", "id"), &HexWorldCellDef::set_chunk_id);
	ClassDB::bind_method(D_METHOD("get_chunk_id"), &HexWorldCellDef::get_chunk_id);
	ClassDB::bind_method(D_METHOD("set_is_choke", "enabled"), &HexWorldCellDef::set_is_choke);
	ClassDB::bind_method(D_METHOD("get_is_choke"), &HexWorldCellDef::get_is_choke);
	ClassDB::bind_method(D_METHOD("set_generator_profile", "profile"), &HexWorldCellDef::set_generator_profile);
	ClassDB::bind_method(D_METHOD("get_generator_profile"), &HexWorldCellDef::get_generator_profile);
	ClassDB::bind_method(D_METHOD("set_detail_override", "template"), &HexWorldCellDef::set_detail_override);
	ClassDB::bind_method(D_METHOD("get_detail_override"), &HexWorldCellDef::get_detail_override);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexWorldCellDef::to_dict);
	ClassDB::bind_static_method("HexWorldCellDef", D_METHOD("from_dict", "dict"), &HexWorldCellDef::from_dict);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "coord", PROPERTY_HINT_RESOURCE_TYPE, "WorldCellCoord"), "set_coord", "get_coord");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "biome"), "set_biome", "get_biome");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "region_summary"), "set_region_summary", "get_region_summary");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "tags"), "set_tags", "get_tags");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "chunk_id"), "set_chunk_id", "get_chunk_id");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_choke"), "set_is_choke", "get_is_choke");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "generator_profile"), "set_generator_profile", "get_generator_profile");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "detail_override", PROPERTY_HINT_RESOURCE_TYPE, "HexDetailTemplate"), "set_detail_override", "get_detail_override");
}

void HexWorldCellDef::set_coord(const Ref<WorldCellCoord> &p_coord) { coord = p_coord; }
Ref<WorldCellCoord> HexWorldCellDef::get_coord() const { return coord; }
void HexWorldCellDef::set_biome(const StringName &p_biome) { biome = p_biome; }
StringName HexWorldCellDef::get_biome() const { return biome; }
void HexWorldCellDef::set_region_summary(const StringName &p_summary) { region_summary = p_summary; }
StringName HexWorldCellDef::get_region_summary() const { return region_summary; }
void HexWorldCellDef::set_tags(const PackedStringArray &p_tags) { tags = p_tags; }
PackedStringArray HexWorldCellDef::get_tags() const { return tags; }

bool HexWorldCellDef::has_tag(const StringName &p_tag) const {
	for (int i = 0; i < tags.size(); i++) {
		if (tags[i] == p_tag) {
			return true;
		}
	}
	return false;
}

void HexWorldCellDef::set_chunk_id(const StringName &p_id) { chunk_id = p_id; }
StringName HexWorldCellDef::get_chunk_id() const { return chunk_id; }
void HexWorldCellDef::set_is_choke(bool p_enabled) { is_choke = p_enabled; }
bool HexWorldCellDef::get_is_choke() const { return is_choke; }
void HexWorldCellDef::set_generator_profile(const StringName &p_profile) { generator_profile = p_profile; }
StringName HexWorldCellDef::get_generator_profile() const { return generator_profile; }
void HexWorldCellDef::set_detail_override(const Ref<HexDetailTemplate> &p_template) { detail_override = p_template; }
Ref<HexDetailTemplate> HexWorldCellDef::get_detail_override() const { return detail_override; }

Dictionary HexWorldCellDef::to_dict() const {
	Dictionary d;
	d[StringName("coord")] = coord.is_valid() ? coord->to_dict() : Dictionary();
	d[StringName("biome")] = biome;
	d[StringName("region_summary")] = region_summary;
	d[StringName("tags")] = tags;
	d[StringName("chunk_id")] = chunk_id;
	d[StringName("is_choke")] = is_choke;
	d[StringName("generator_profile")] = generator_profile;
	return d;
}

Ref<HexWorldCellDef> HexWorldCellDef::from_dict(const Dictionary &p_dict) {
	Ref<HexWorldCellDef> c;
	c.instantiate();
	c->set_coord(WorldCellCoord::from_dict(p_dict.get(StringName("coord"), Dictionary())));
	c->set_biome(p_dict.get(StringName("biome"), StringName()));
	c->set_region_summary(p_dict.get(StringName("region_summary"), StringName()));
	c->set_tags(p_dict.get(StringName("tags"), PackedStringArray()));
	c->set_chunk_id(p_dict.get(StringName("chunk_id"), StringName()));
	c->set_is_choke(p_dict.get(StringName("is_choke"), false));
	c->set_generator_profile(p_dict.get(StringName("generator_profile"), StringName()));
	return c;
}

/**************************************************************************/
/* HexWorldPortalDef                                                      */
/**************************************************************************/

void HexWorldPortalDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_edge", "edge"), &HexWorldPortalDef::set_edge);
	ClassDB::bind_method(D_METHOD("get_edge"), &HexWorldPortalDef::get_edge);
	ClassDB::bind_method(D_METHOD("set_kind", "kind"), &HexWorldPortalDef::set_kind);
	ClassDB::bind_method(D_METHOD("get_kind"), &HexWorldPortalDef::get_kind);
	ClassDB::bind_method(D_METHOD("set_required_key_id", "key"), &HexWorldPortalDef::set_required_key_id);
	ClassDB::bind_method(D_METHOD("get_required_key_id"), &HexWorldPortalDef::get_required_key_id);
	ClassDB::bind_method(D_METHOD("set_enabled_by_default", "enabled"), &HexWorldPortalDef::set_enabled_by_default);
	ClassDB::bind_method(D_METHOD("get_enabled_by_default"), &HexWorldPortalDef::get_enabled_by_default);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexWorldPortalDef::to_dict);
	ClassDB::bind_static_method("HexWorldPortalDef", D_METHOD("from_dict", "dict"), &HexWorldPortalDef::from_dict);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "edge", PROPERTY_HINT_RESOURCE_TYPE, "EdgeKey"), "set_edge", "get_edge");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "kind"), "set_kind", "get_kind");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "required_key_id"), "set_required_key_id", "get_required_key_id");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled_by_default"), "set_enabled_by_default", "get_enabled_by_default");
}

void HexWorldPortalDef::set_edge(const Ref<EdgeKey> &p_edge) { edge = p_edge; }
Ref<EdgeKey> HexWorldPortalDef::get_edge() const { return edge; }
void HexWorldPortalDef::set_kind(const StringName &p_kind) { kind = p_kind; }
StringName HexWorldPortalDef::get_kind() const { return kind; }
void HexWorldPortalDef::set_required_key_id(const StringName &p_key) { required_key_id = p_key; }
StringName HexWorldPortalDef::get_required_key_id() const { return required_key_id; }
void HexWorldPortalDef::set_enabled_by_default(bool p_enabled) { enabled_by_default = p_enabled; }
bool HexWorldPortalDef::get_enabled_by_default() const { return enabled_by_default; }

Dictionary HexWorldPortalDef::to_dict() const {
	Dictionary d;
	d[StringName("edge")] = edge.is_valid() ? edge->to_dict() : Dictionary();
	d[StringName("kind")] = kind;
	d[StringName("required_key_id")] = required_key_id;
	d[StringName("enabled_by_default")] = enabled_by_default;
	return d;
}

Ref<HexWorldPortalDef> HexWorldPortalDef::from_dict(const Dictionary &p_dict) {
	Ref<HexWorldPortalDef> p;
	p.instantiate();
	p->set_edge(EdgeKey::from_dict(p_dict.get(StringName("edge"), Dictionary())));
	p->set_kind(p_dict.get(StringName("kind"), StringName("open")));
	p->set_required_key_id(p_dict.get(StringName("required_key_id"), StringName()));
	p->set_enabled_by_default(p_dict.get(StringName("enabled_by_default"), true));
	return p;
}

/**************************************************************************/
/* HexWorldTemplate                                                       */
/**************************************************************************/

void HexWorldTemplate::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_map_id", "id"), &HexWorldTemplate::set_map_id);
	ClassDB::bind_method(D_METHOD("get_map_id"), &HexWorldTemplate::get_map_id);
	ClassDB::bind_method(D_METHOD("set_template_version", "version"), &HexWorldTemplate::set_template_version);
	ClassDB::bind_method(D_METHOD("get_template_version"), &HexWorldTemplate::get_template_version);
	ClassDB::bind_method(D_METHOD("set_world_hex_size", "size"), &HexWorldTemplate::set_world_hex_size);
	ClassDB::bind_method(D_METHOD("get_world_hex_size"), &HexWorldTemplate::get_world_hex_size);
	ClassDB::bind_method(D_METHOD("set_default_biome", "biome"), &HexWorldTemplate::set_default_biome);
	ClassDB::bind_method(D_METHOD("get_default_biome"), &HexWorldTemplate::get_default_biome);
	ClassDB::bind_method(D_METHOD("set_detail_descriptor", "descriptor"), &HexWorldTemplate::set_detail_descriptor);
	ClassDB::bind_method(D_METHOD("get_detail_descriptor"), &HexWorldTemplate::get_detail_descriptor);
	ClassDB::bind_method(D_METHOD("set_cells", "cells"), &HexWorldTemplate::set_cells);
	ClassDB::bind_method(D_METHOD("get_cells"), &HexWorldTemplate::get_cells);
	ClassDB::bind_method(D_METHOD("set_portals", "portals"), &HexWorldTemplate::set_portals);
	ClassDB::bind_method(D_METHOD("get_portals"), &HexWorldTemplate::get_portals);
	ClassDB::bind_method(D_METHOD("compile"), &HexWorldTemplate::compile);
	ClassDB::bind_method(D_METHOD("is_compiled"), &HexWorldTemplate::is_compiled);
	ClassDB::bind_method(D_METHOD("get_cell_count"), &HexWorldTemplate::get_cell_count);
	ClassDB::bind_method(D_METHOD("get_cell_at", "coord"), &HexWorldTemplate::get_cell_at);
	ClassDB::bind_method(D_METHOD("validate"), &HexWorldTemplate::validate);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "map_id"), "set_map_id", "get_map_id");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "template_version", PROPERTY_HINT_RESOURCE_TYPE, "TemplateVersion"), "set_template_version", "get_template_version");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "world_hex_size"), "set_world_hex_size", "get_world_hex_size");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "default_biome"), "set_default_biome", "get_default_biome");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "detail_descriptor", PROPERTY_HINT_RESOURCE_TYPE, "DetailMapDescriptor"), "set_detail_descriptor", "get_detail_descriptor");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cells", PROPERTY_HINT_ARRAY_TYPE, "HexWorldCellDef"), "set_cells", "get_cells");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "portals", PROPERTY_HINT_ARRAY_TYPE, "HexWorldPortalDef"), "set_portals", "get_portals");
}

void HexWorldTemplate::set_map_id(const StringName &p_id) { map_id = p_id; compiled = false; }
StringName HexWorldTemplate::get_map_id() const { return map_id; }
void HexWorldTemplate::set_template_version(const Ref<TemplateVersion> &p_version) { template_version = p_version; }
Ref<TemplateVersion> HexWorldTemplate::get_template_version() const { return template_version; }
void HexWorldTemplate::set_world_hex_size(double p_size) { world_hex_size = p_size; }
double HexWorldTemplate::get_world_hex_size() const { return world_hex_size; }
void HexWorldTemplate::set_default_biome(const StringName &p_biome) { default_biome = p_biome; }
StringName HexWorldTemplate::get_default_biome() const { return default_biome; }
void HexWorldTemplate::set_detail_descriptor(const Ref<DetailMapDescriptor> &p_descriptor) { detail_descriptor = p_descriptor; }
Ref<DetailMapDescriptor> HexWorldTemplate::get_detail_descriptor() const { return detail_descriptor; }
void HexWorldTemplate::set_cells(const TypedArray<HexWorldCellDef> &p_cells) { cells = p_cells; compiled = false; }
TypedArray<HexWorldCellDef> HexWorldTemplate::get_cells() const { return cells; }
void HexWorldTemplate::set_portals(const TypedArray<HexWorldPortalDef> &p_portals) { portals = p_portals; }
TypedArray<HexWorldPortalDef> HexWorldTemplate::get_portals() const { return portals; }

static uint64_t _pack_world_coord(const Ref<WorldCellCoord> &p_coord) {
	return ((uint64_t)(uint32_t)p_coord->get_q() << 32) | (uint32_t)p_coord->get_r();
}

bool HexWorldTemplate::compile() {
	cell_index.clear();
	compiled = false;
	for (int i = 0; i < cells.size(); i++) {
		Ref<HexWorldCellDef> cell = cells[i];
		if (cell.is_null() || cell->get_coord().is_null()) {
			return false;
		}
		const uint64_t key = _pack_world_coord(cell->get_coord());
		if (cell_index.has(key)) {
			return false;
		}
		cell_index.insert(key, i);
	}
	compiled = true;
	return true;
}

bool HexWorldTemplate::is_compiled() const { return compiled; }
int HexWorldTemplate::get_cell_count() const { return cells.size(); }

Ref<HexWorldCellDef> HexWorldTemplate::get_cell_at(const Ref<WorldCellCoord> &p_coord) const {
	if (p_coord.is_null()) {
		return Ref<HexWorldCellDef>();
	}
	const int *idx = cell_index.getptr(_pack_world_coord(p_coord));
	if (idx == nullptr) {
		return Ref<HexWorldCellDef>();
	}
	return cells[*idx];
}

String HexWorldTemplate::validate() const {
	if (map_id == StringName()) {
		return "world template missing map_id";
	}
	if (world_hex_size <= 0.0) {
		return "world_hex_size must be > 0";
	}
	if (template_version.is_null()) {
		return "world template missing template_version";
	}
	if (detail_descriptor.is_null()) {
		return "world template missing detail_descriptor";
	}
	String desc_err = detail_descriptor->validate();
	if (!desc_err.is_empty()) {
		return desc_err;
	}
	if (!compiled && !const_cast<HexWorldTemplate *>(this)->compile()) {
		return "duplicate or invalid world cell coords";
	}
	for (int i = 0; i < portals.size(); i++) {
		Ref<HexWorldPortalDef> portal = portals[i];
		if (portal.is_null() || portal->get_edge().is_null() || !portal->get_edge()->is_valid()) {
			return "portal[" + itos(i) + "] missing valid edge";
		}
		// Portal endpoints must reference declared cells.
		if (get_cell_at(portal->get_edge()->get_endpoint_a()).is_null() ||
				get_cell_at(portal->get_edge()->get_endpoint_b()).is_null()) {
			return "portal[" + itos(i) + "] references undeclared world cell";
		}
	}
	return String();
}
