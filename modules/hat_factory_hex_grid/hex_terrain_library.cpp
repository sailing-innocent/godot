#include "hex_terrain_library.h"

#include "core/object/class_db.h"

void HexTerrainLibrary::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_terrains", "terrains"), &HexTerrainLibrary::set_terrains);
	ClassDB::bind_method(D_METHOD("get_terrains"), &HexTerrainLibrary::get_terrains);

	ClassDB::bind_method(D_METHOD("add_terrain", "terrain_id", "terrain"), &HexTerrainLibrary::add_terrain);
	ClassDB::bind_method(D_METHOD("remove_terrain", "terrain_id"), &HexTerrainLibrary::remove_terrain);
	ClassDB::bind_method(D_METHOD("has_terrain", "terrain_id"), &HexTerrainLibrary::has_terrain);
	ClassDB::bind_method(D_METHOD("get_terrain", "terrain_id"), &HexTerrainLibrary::get_terrain);
	ClassDB::bind_method(D_METHOD("get_terrain_ids"), &HexTerrainLibrary::get_terrain_ids);

	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "terrains"), "set_terrains", "get_terrains");

	ADD_SIGNAL(MethodInfo("terrains_changed"));
}

void HexTerrainLibrary::set_terrains(const Dictionary &p_terrains) {
	terrains = p_terrains;
	emit_signal(SNAME("terrains_changed"));
	emit_changed();
}

Dictionary HexTerrainLibrary::get_terrains() const {
	return terrains;
}

void HexTerrainLibrary::add_terrain(const StringName &p_id, const Ref<HexTerrainDef> &p_def) {
	terrains[p_id] = p_def;
	emit_signal(SNAME("terrains_changed"));
	emit_changed();
}

void HexTerrainLibrary::remove_terrain(const StringName &p_id) {
	terrains.erase(p_id);
	emit_signal(SNAME("terrains_changed"));
	emit_changed();
}

bool HexTerrainLibrary::has_terrain(const StringName &p_id) const {
	return terrains.has(p_id);
}

Ref<HexTerrainDef> HexTerrainLibrary::get_terrain(const StringName &p_id) const {
	return terrains.get(p_id, Variant());
}

TypedArray<StringName> HexTerrainLibrary::get_terrain_ids() const {
	TypedArray<StringName> ids;
	Array keys = terrains.keys();
	for (int i = 0; i < keys.size(); i++) {
		ids.push_back(keys[i]);
	}
	return ids;
}
