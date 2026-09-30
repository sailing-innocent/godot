/**************************************************************************/
/*  hex_terrain_table_set.cpp                                             */
/**************************************************************************/
#include "hex_terrain_table_set.h"

#include "core/object/class_db.h"

void HexTerrainTableSet::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_table_set_id", "id"), &HexTerrainTableSet::set_table_set_id);
	ClassDB::bind_method(D_METHOD("get_table_set_id"), &HexTerrainTableSet::get_table_set_id);
	ClassDB::bind_method(D_METHOD("set_base_terrains", "defs"), &HexTerrainTableSet::set_base_terrains);
	ClassDB::bind_method(D_METHOD("get_base_terrains"), &HexTerrainTableSet::get_base_terrains);
	ClassDB::bind_method(D_METHOD("set_surfaces", "defs"), &HexTerrainTableSet::set_surfaces);
	ClassDB::bind_method(D_METHOD("get_surfaces"), &HexTerrainTableSet::get_surfaces);
	ClassDB::bind_method(D_METHOD("set_objects", "defs"), &HexTerrainTableSet::set_objects);
	ClassDB::bind_method(D_METHOD("get_objects"), &HexTerrainTableSet::get_objects);
	ClassDB::bind_method(D_METHOD("set_edge_types", "defs"), &HexTerrainTableSet::set_edge_types);
	ClassDB::bind_method(D_METHOD("get_edge_types"), &HexTerrainTableSet::get_edge_types);
	ClassDB::bind_method(D_METHOD("set_area_effects", "defs"), &HexTerrainTableSet::set_area_effects);
	ClassDB::bind_method(D_METHOD("get_area_effects"), &HexTerrainTableSet::get_area_effects);
	ClassDB::bind_method(D_METHOD("set_movement_profiles", "defs"), &HexTerrainTableSet::set_movement_profiles);
	ClassDB::bind_method(D_METHOD("get_movement_profiles"), &HexTerrainTableSet::get_movement_profiles);
	ClassDB::bind_method(D_METHOD("set_verbs", "defs"), &HexTerrainTableSet::set_verbs);
	ClassDB::bind_method(D_METHOD("get_verbs"), &HexTerrainTableSet::get_verbs);
	ClassDB::bind_method(D_METHOD("set_reactions", "defs"), &HexTerrainTableSet::set_reactions);
	ClassDB::bind_method(D_METHOD("get_reactions"), &HexTerrainTableSet::get_reactions);
	ClassDB::bind_method(D_METHOD("compile"), &HexTerrainTableSet::compile);
	ClassDB::bind_method(D_METHOD("is_compiled"), &HexTerrainTableSet::is_compiled);
	ClassDB::bind_method(D_METHOD("get_compile_error"), &HexTerrainTableSet::get_compile_error);
	ClassDB::bind_method(D_METHOD("find_base_terrain", "id"), &HexTerrainTableSet::find_base_terrain);
	ClassDB::bind_method(D_METHOD("find_surface", "id"), &HexTerrainTableSet::find_surface);
	ClassDB::bind_method(D_METHOD("find_object", "id"), &HexTerrainTableSet::find_object);
	ClassDB::bind_method(D_METHOD("find_edge_type", "id"), &HexTerrainTableSet::find_edge_type);
	ClassDB::bind_method(D_METHOD("find_area_effect", "id"), &HexTerrainTableSet::find_area_effect);
	ClassDB::bind_method(D_METHOD("find_movement_profile", "id"), &HexTerrainTableSet::find_movement_profile);
	ClassDB::bind_method(D_METHOD("find_verb", "id"), &HexTerrainTableSet::find_verb);
	ClassDB::bind_method(D_METHOD("get_base_terrain", "index"), &HexTerrainTableSet::get_base_terrain);
	ClassDB::bind_method(D_METHOD("get_surface", "index"), &HexTerrainTableSet::get_surface);
	ClassDB::bind_method(D_METHOD("get_object", "index"), &HexTerrainTableSet::get_object);
	ClassDB::bind_method(D_METHOD("get_edge_type", "index"), &HexTerrainTableSet::get_edge_type);
	ClassDB::bind_method(D_METHOD("get_area_effect", "index"), &HexTerrainTableSet::get_area_effect);
	ClassDB::bind_method(D_METHOD("get_movement_profile", "index"), &HexTerrainTableSet::get_movement_profile);
	ClassDB::bind_method(D_METHOD("get_verb", "index"), &HexTerrainTableSet::get_verb);
	ClassDB::bind_method(D_METHOD("get_base_terrain_count"), &HexTerrainTableSet::get_base_terrain_count);
	ClassDB::bind_method(D_METHOD("get_surface_count"), &HexTerrainTableSet::get_surface_count);
	ClassDB::bind_method(D_METHOD("get_object_count"), &HexTerrainTableSet::get_object_count);
	ClassDB::bind_method(D_METHOD("get_edge_type_count"), &HexTerrainTableSet::get_edge_type_count);
	ClassDB::bind_method(D_METHOD("get_area_effect_count"), &HexTerrainTableSet::get_area_effect_count);
	ClassDB::bind_method(D_METHOD("get_movement_profile_count"), &HexTerrainTableSet::get_movement_profile_count);
	ClassDB::bind_method(D_METHOD("get_verb_count"), &HexTerrainTableSet::get_verb_count);
	ClassDB::bind_method(D_METHOD("get_reaction_count"), &HexTerrainTableSet::get_reaction_count);
	ClassDB::bind_method(D_METHOD("get_reaction", "index"), &HexTerrainTableSet::get_reaction);
	ClassDB::bind_method(D_METHOD("validate"), &HexTerrainTableSet::validate);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "table_set_id"), "set_table_set_id", "get_table_set_id");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "base_terrains"), "set_base_terrains", "get_base_terrains");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "surfaces"), "set_surfaces", "get_surfaces");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "objects"), "set_objects", "get_objects");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "edge_types"), "set_edge_types", "get_edge_types");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "area_effects"), "set_area_effects", "get_area_effects");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "movement_profiles"), "set_movement_profiles", "get_movement_profiles");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "verbs"), "set_verbs", "get_verbs");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "reactions", PROPERTY_HINT_ARRAY_TYPE, "HexReactionDef"), "set_reactions", "get_reactions");
}

void HexTerrainTableSet::set_table_set_id(const StringName &p_id) { table_set_id = p_id; }
StringName HexTerrainTableSet::get_table_set_id() const { return table_set_id; }
void HexTerrainTableSet::set_base_terrains(const Dictionary &p_defs) { base_terrains = p_defs; compiled = false; }
Dictionary HexTerrainTableSet::get_base_terrains() const { return base_terrains; }
void HexTerrainTableSet::set_surfaces(const Dictionary &p_defs) { surfaces = p_defs; compiled = false; }
Dictionary HexTerrainTableSet::get_surfaces() const { return surfaces; }
void HexTerrainTableSet::set_objects(const Dictionary &p_defs) { objects = p_defs; compiled = false; }
Dictionary HexTerrainTableSet::get_objects() const { return objects; }
void HexTerrainTableSet::set_edge_types(const Dictionary &p_defs) { edge_types = p_defs; compiled = false; }
Dictionary HexTerrainTableSet::get_edge_types() const { return edge_types; }
void HexTerrainTableSet::set_area_effects(const Dictionary &p_defs) { area_effects = p_defs; compiled = false; }
Dictionary HexTerrainTableSet::get_area_effects() const { return area_effects; }
void HexTerrainTableSet::set_movement_profiles(const Dictionary &p_defs) { movement_profiles = p_defs; compiled = false; }
Dictionary HexTerrainTableSet::get_movement_profiles() const { return movement_profiles; }
void HexTerrainTableSet::set_verbs(const Dictionary &p_defs) { verbs = p_defs; compiled = false; }
Dictionary HexTerrainTableSet::get_verbs() const { return verbs; }
void HexTerrainTableSet::set_reactions(const TypedArray<HexReactionDef> &p_defs) { reactions = p_defs; compiled = false; }
TypedArray<HexReactionDef> HexTerrainTableSet::get_reactions() const { return reactions; }

template <typename T>
bool HexTerrainTableSet::_compile_table(const Dictionary &p_src, TypedArray<T> &r_array, HashMap<StringName, int> &r_index, String &r_error, const char *p_label) {
	r_array.clear();
	r_index.clear();
	const LocalVector<Variant> keys = p_src.get_key_list();
	for (const Variant &key : keys) {
		Ref<T> def = p_src.get(key, Ref<T>());
		if (def.is_null()) {
			r_error = String(p_label) + " entry '" + String(key) + "' is not a valid def";
			return false;
		}
		const StringName id = def->get_def_id();
		if (id == StringName()) {
			r_error = String(p_label) + " entry '" + String(key) + "' missing def_id";
			return false;
		}
		if (r_index.has(id)) {
			r_error = String(p_label) + " duplicate def_id '" + String(id) + "'";
			return false;
		}
		r_index.insert(id, r_array.size());
		r_array.append(def);
	}
	return true;
}

bool HexTerrainTableSet::compile() {
	compiled = false;
	if (!_compile_table(base_terrains, c_bases, i_base, compile_error, "base_terrains")) return false;
	if (!_compile_table(surfaces, c_surfaces, i_surface, compile_error, "surfaces")) return false;
	if (!_compile_table(objects, c_objects, i_object, compile_error, "objects")) return false;
	if (!_compile_table(edge_types, c_edge_types, i_edge_type, compile_error, "edge_types")) return false;
	if (!_compile_table(area_effects, c_area_effects, i_area_effect, compile_error, "area_effects")) return false;
	if (!_compile_table(movement_profiles, c_profiles, i_profile, compile_error, "movement_profiles")) return false;
	if (!_compile_table(verbs, c_verbs, i_verb, compile_error, "verbs")) return false;
	compiled = true;
	return true;
}

bool HexTerrainTableSet::is_compiled() const { return compiled; }
String HexTerrainTableSet::get_compile_error() const { return compile_error; }

int HexTerrainTableSet::find_base_terrain(const StringName &p_id) const { const int *v = i_base.getptr(p_id); return v ? *v : -1; }
int HexTerrainTableSet::find_surface(const StringName &p_id) const { const int *v = i_surface.getptr(p_id); return v ? *v : -1; }
int HexTerrainTableSet::find_object(const StringName &p_id) const { const int *v = i_object.getptr(p_id); return v ? *v : -1; }
int HexTerrainTableSet::find_edge_type(const StringName &p_id) const { const int *v = i_edge_type.getptr(p_id); return v ? *v : -1; }
int HexTerrainTableSet::find_area_effect(const StringName &p_id) const { const int *v = i_area_effect.getptr(p_id); return v ? *v : -1; }
int HexTerrainTableSet::find_movement_profile(const StringName &p_id) const { const int *v = i_profile.getptr(p_id); return v ? *v : -1; }
int HexTerrainTableSet::find_verb(const StringName &p_id) const { const int *v = i_verb.getptr(p_id); return v ? *v : -1; }

template <typename T>
static Ref<T> _array_get(const TypedArray<T> &p_array, int p_index) {
	if (p_index < 0 || p_index >= p_array.size()) {
		return Ref<T>();
	}
	return p_array[p_index];
}

Ref<HexBaseTerrainDef> HexTerrainTableSet::get_base_terrain(int p_index) const { return _array_get(c_bases, p_index); }
Ref<HexSurfaceDef> HexTerrainTableSet::get_surface(int p_index) const { return _array_get(c_surfaces, p_index); }
Ref<HexObjectDef> HexTerrainTableSet::get_object(int p_index) const { return _array_get(c_objects, p_index); }
Ref<HexEdgeTypeDef> HexTerrainTableSet::get_edge_type(int p_index) const { return _array_get(c_edge_types, p_index); }
Ref<HexAreaEffectDef> HexTerrainTableSet::get_area_effect(int p_index) const { return _array_get(c_area_effects, p_index); }
Ref<HexMovementProfile> HexTerrainTableSet::get_movement_profile(int p_index) const { return _array_get(c_profiles, p_index); }
Ref<HexVerbDef> HexTerrainTableSet::get_verb(int p_index) const { return _array_get(c_verbs, p_index); }

int HexTerrainTableSet::get_base_terrain_count() const { return c_bases.size(); }
int HexTerrainTableSet::get_surface_count() const { return c_surfaces.size(); }
int HexTerrainTableSet::get_object_count() const { return c_objects.size(); }
int HexTerrainTableSet::get_edge_type_count() const { return c_edge_types.size(); }
int HexTerrainTableSet::get_area_effect_count() const { return c_area_effects.size(); }
int HexTerrainTableSet::get_movement_profile_count() const { return c_profiles.size(); }
int HexTerrainTableSet::get_verb_count() const { return c_verbs.size(); }
int HexTerrainTableSet::get_reaction_count() const { return reactions.size(); }
Ref<HexReactionDef> HexTerrainTableSet::get_reaction(int p_index) const { return _array_get(reactions, p_index); }

String HexTerrainTableSet::validate() const {
	if (!compiled && !const_cast<HexTerrainTableSet *>(this)->compile()) {
		return compile_error;
	}
	// Cross references.
	for (int i = 0; i < c_objects.size(); i++) {
		Ref<HexObjectDef> def = c_objects[i];
		if (def->get_destroyed_form() != StringName() && !i_object.has(def->get_destroyed_form())) {
			return "object '" + String(def->get_def_id()) + "' destroyed_form '" + String(def->get_destroyed_form()) + "' not found";
		}
	}
	for (int i = 0; i < c_edge_types.size(); i++) {
		Ref<HexEdgeTypeDef> def = c_edge_types[i];
		if (def->get_destroyed_form() != StringName() && !i_edge_type.has(def->get_destroyed_form())) {
			return "edge_type '" + String(def->get_def_id()) + "' destroyed_form '" + String(def->get_destroyed_form()) + "' not found";
		}
		if (def->get_action_type() == StringName()) {
			return "edge_type '" + String(def->get_def_id()) + "' missing action_type";
		}
	}
	for (int i = 0; i < reactions.size(); i++) {
		Ref<HexReactionDef> def = reactions[i];
		if (def->get_result_surface() != StringName() && !i_surface.has(def->get_result_surface())) {
			return "reaction '" + String(def->get_def_id()) + "' result_surface '" + String(def->get_result_surface()) + "' not found";
		}
		if (def->get_result_effect() != StringName() && !i_area_effect.has(def->get_result_effect())) {
			return "reaction '" + String(def->get_def_id()) + "' result_effect '" + String(def->get_result_effect()) + "' not found";
		}
	}
	for (int i = 0; i < c_profiles.size(); i++) {
		Ref<HexMovementProfile> profile = _array_get(c_profiles, i);
		String err = profile->validate();
		if (!err.is_empty()) {
			return err;
		}
	}
	for (int i = 0; i < c_verbs.size(); i++) {
		Ref<HexVerbDef> verb = _array_get(c_verbs, i);
		String err = verb->validate();
		if (!err.is_empty()) {
			return err;
		}
	}
	return String();
}
