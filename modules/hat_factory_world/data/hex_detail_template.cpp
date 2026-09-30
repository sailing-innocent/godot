/**************************************************************************/
/*  hex_detail_template.cpp                                               */
/**************************************************************************/
#include "hex_detail_template.h"

#include "core/object/class_db.h"

/**************************************************************************/
/* HexDetailCellDef                                                       */
/**************************************************************************/

void HexDetailCellDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_coord", "coord"), &HexDetailCellDef::set_coord);
	ClassDB::bind_method(D_METHOD("get_coord"), &HexDetailCellDef::get_coord);
	ClassDB::bind_method(D_METHOD("set_base_terrain", "id"), &HexDetailCellDef::set_base_terrain);
	ClassDB::bind_method(D_METHOD("get_base_terrain"), &HexDetailCellDef::get_base_terrain);
	ClassDB::bind_method(D_METHOD("set_elevation", "elevation"), &HexDetailCellDef::set_elevation);
	ClassDB::bind_method(D_METHOD("get_elevation"), &HexDetailCellDef::get_elevation);
	ClassDB::bind_method(D_METHOD("set_surface_ids", "ids"), &HexDetailCellDef::set_surface_ids);
	ClassDB::bind_method(D_METHOD("get_surface_ids"), &HexDetailCellDef::get_surface_ids);
	ClassDB::bind_method(D_METHOD("set_object_id", "id"), &HexDetailCellDef::set_object_id);
	ClassDB::bind_method(D_METHOD("get_object_id"), &HexDetailCellDef::get_object_id);
	ClassDB::bind_method(D_METHOD("set_edge_type_ids", "ids"), &HexDetailCellDef::set_edge_type_ids);
	ClassDB::bind_method(D_METHOD("get_edge_type_ids"), &HexDetailCellDef::get_edge_type_ids);
	ClassDB::bind_method(D_METHOD("set_area_effect_ids", "ids"), &HexDetailCellDef::set_area_effect_ids);
	ClassDB::bind_method(D_METHOD("get_area_effect_ids"), &HexDetailCellDef::get_area_effect_ids);
	ClassDB::bind_method(D_METHOD("set_feature", "feature"), &HexDetailCellDef::set_feature);
	ClassDB::bind_method(D_METHOD("get_feature"), &HexDetailCellDef::get_feature);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexDetailCellDef::to_dict);
	ClassDB::bind_static_method("HexDetailCellDef", D_METHOD("from_dict", "dict"), &HexDetailCellDef::from_dict);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "coord", PROPERTY_HINT_RESOURCE_TYPE, "MicroCoord"), "set_coord", "get_coord");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "base_terrain"), "set_base_terrain", "get_base_terrain");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "elevation"), "set_elevation", "get_elevation");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "surface_ids"), "set_surface_ids", "get_surface_ids");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "object_id"), "set_object_id", "get_object_id");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "edge_type_ids"), "set_edge_type_ids", "get_edge_type_ids");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "area_effect_ids"), "set_area_effect_ids", "get_area_effect_ids");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "feature", PROPERTY_HINT_RESOURCE_TYPE, "FeatureId"), "set_feature", "get_feature");
}

void HexDetailCellDef::set_coord(const Ref<MicroCoord> &p_coord) { coord = p_coord; }
Ref<MicroCoord> HexDetailCellDef::get_coord() const { return coord; }
void HexDetailCellDef::set_base_terrain(const StringName &p_id) { base_terrain = p_id; }
StringName HexDetailCellDef::get_base_terrain() const { return base_terrain; }
void HexDetailCellDef::set_elevation(int p_elevation) { elevation = p_elevation; }
int HexDetailCellDef::get_elevation() const { return elevation; }
void HexDetailCellDef::set_surface_ids(const PackedStringArray &p_ids) { surface_ids = p_ids; }
PackedStringArray HexDetailCellDef::get_surface_ids() const { return surface_ids; }
void HexDetailCellDef::set_object_id(const StringName &p_id) { object_id = p_id; }
StringName HexDetailCellDef::get_object_id() const { return object_id; }
void HexDetailCellDef::set_edge_type_ids(const PackedStringArray &p_ids) { edge_type_ids = p_ids; }
PackedStringArray HexDetailCellDef::get_edge_type_ids() const { return edge_type_ids; }
void HexDetailCellDef::set_area_effect_ids(const PackedStringArray &p_ids) { area_effect_ids = p_ids; }
PackedStringArray HexDetailCellDef::get_area_effect_ids() const { return area_effect_ids; }
void HexDetailCellDef::set_feature(const Ref<FeatureId> &p_feature) { feature = p_feature; }
Ref<FeatureId> HexDetailCellDef::get_feature() const { return feature; }

Dictionary HexDetailCellDef::to_dict() const {
	Dictionary d;
	d[StringName("coord")] = coord.is_valid() ? coord->to_dict() : Dictionary();
	d[StringName("base_terrain")] = base_terrain;
	d[StringName("elevation")] = elevation;
	d[StringName("surface_ids")] = surface_ids;
	d[StringName("object_id")] = object_id;
	d[StringName("edge_type_ids")] = edge_type_ids;
	d[StringName("area_effect_ids")] = area_effect_ids;
	if (feature.is_valid()) {
		d[StringName("feature")] = feature->to_dict();
	} else {
		d[StringName("feature")] = Variant();
	}
	return d;
}

Ref<HexDetailCellDef> HexDetailCellDef::from_dict(const Dictionary &p_dict) {
	Ref<HexDetailCellDef> c;
	c.instantiate();
	c->set_coord(MicroCoord::from_dict(p_dict.get(StringName("coord"), Dictionary())));
	c->set_base_terrain(p_dict.get(StringName("base_terrain"), StringName()));
	c->set_elevation(p_dict.get(StringName("elevation"), 0));
	c->set_surface_ids(p_dict.get(StringName("surface_ids"), PackedStringArray()));
	c->set_object_id(p_dict.get(StringName("object_id"), StringName()));
	c->set_edge_type_ids(p_dict.get(StringName("edge_type_ids"), PackedStringArray()));
	c->set_area_effect_ids(p_dict.get(StringName("area_effect_ids"), PackedStringArray()));
	if (p_dict.has(StringName("feature")) && p_dict[StringName("feature")].get_type() == Variant::DICTIONARY) {
		c->set_feature(FeatureId::from_dict(p_dict[StringName("feature")]));
	}
	return c;
}

/**************************************************************************/
/* HexDetailTemplate                                                      */
/**************************************************************************/

void HexDetailTemplate::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_map_id", "id"), &HexDetailTemplate::set_map_id);
	ClassDB::bind_method(D_METHOD("get_map_id"), &HexDetailTemplate::get_map_id);
	ClassDB::bind_method(D_METHOD("set_template_version", "version"), &HexDetailTemplate::set_template_version);
	ClassDB::bind_method(D_METHOD("get_template_version"), &HexDetailTemplate::get_template_version);
	ClassDB::bind_method(D_METHOD("set_cells", "cells"), &HexDetailTemplate::set_cells);
	ClassDB::bind_method(D_METHOD("get_cells"), &HexDetailTemplate::get_cells);
	ClassDB::bind_method(D_METHOD("set_generator_profile", "profile"), &HexDetailTemplate::set_generator_profile);
	ClassDB::bind_method(D_METHOD("get_generator_profile"), &HexDetailTemplate::get_generator_profile);
	ClassDB::bind_method(D_METHOD("compile"), &HexDetailTemplate::compile);
	ClassDB::bind_method(D_METHOD("is_compiled"), &HexDetailTemplate::is_compiled);
	ClassDB::bind_method(D_METHOD("get_cell_count"), &HexDetailTemplate::get_cell_count);
	ClassDB::bind_method(D_METHOD("get_cell_at", "coord"), &HexDetailTemplate::get_cell_at);
	ClassDB::bind_method(D_METHOD("validate"), &HexDetailTemplate::validate);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "map_id"), "set_map_id", "get_map_id");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "template_version", PROPERTY_HINT_RESOURCE_TYPE, "TemplateVersion"), "set_template_version", "get_template_version");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cells", PROPERTY_HINT_ARRAY_TYPE, "HexDetailCellDef"), "set_cells", "get_cells");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "generator_profile"), "set_generator_profile", "get_generator_profile");
}

void HexDetailTemplate::set_map_id(const StringName &p_id) { map_id = p_id; compiled = false; }
StringName HexDetailTemplate::get_map_id() const { return map_id; }
void HexDetailTemplate::set_template_version(const Ref<TemplateVersion> &p_version) { template_version = p_version; }
Ref<TemplateVersion> HexDetailTemplate::get_template_version() const { return template_version; }
void HexDetailTemplate::set_cells(const TypedArray<HexDetailCellDef> &p_cells) { cells = p_cells; compiled = false; }
TypedArray<HexDetailCellDef> HexDetailTemplate::get_cells() const { return cells; }
void HexDetailTemplate::set_generator_profile(const Dictionary &p_profile) { generator_profile = p_profile; }
Dictionary HexDetailTemplate::get_generator_profile() const { return generator_profile; }

static uint64_t _pack_micro_coord(const Ref<MicroCoord> &p_coord) {
	uint64_t h = ((uint64_t)(uint32_t)p_coord->get_q() << 32) | (uint32_t)p_coord->get_r();
	h = h * 31 + (uint64_t)p_coord->get_detail_level();
	return h;
}

bool HexDetailTemplate::compile() {
	cell_index.clear();
	compiled = false;
	for (int i = 0; i < cells.size(); i++) {
		Ref<HexDetailCellDef> cell = cells[i];
		if (cell.is_null() || cell->get_coord().is_null()) {
			return false;
		}
		if (cell->get_edge_type_ids().size() > 6) {
			return false;
		}
		const uint64_t key = _pack_micro_coord(cell->get_coord());
		if (cell_index.has(key)) {
			return false;
		}
		cell_index.insert(key, i);
	}
	compiled = true;
	return true;
}

bool HexDetailTemplate::is_compiled() const { return compiled; }
int HexDetailTemplate::get_cell_count() const { return cells.size(); }

Ref<HexDetailCellDef> HexDetailTemplate::get_cell_at(const Ref<MicroCoord> &p_coord) const {
	if (p_coord.is_null()) {
		return Ref<HexDetailCellDef>();
	}
	const int *idx = cell_index.getptr(_pack_micro_coord(p_coord));
	if (idx == nullptr) {
		return Ref<HexDetailCellDef>();
	}
	return cells[*idx];
}

String HexDetailTemplate::validate() const {
	if (map_id == StringName()) {
		return "detail template missing map_id";
	}
	if (template_version.is_null()) {
		return "detail template missing template_version";
	}
	if (!compiled && !const_cast<HexDetailTemplate *>(this)->compile()) {
		return "duplicate/invalid micro coords or more than 6 edge ids";
	}
	return String();
}
