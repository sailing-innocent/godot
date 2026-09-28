#include "battle_config.h"

#include "modules/hat_factory_hex_grid/hex_grid_map_data.h"
#include "state/rule_set.h"

#include "core/object/class_db.h"

void BattleConfig::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_session_id", "session_id"), &BattleConfig::set_session_id);
	ClassDB::bind_method(D_METHOD("get_session_id"), &BattleConfig::get_session_id);
	ClassDB::bind_method(D_METHOD("set_random_seed", "random_seed"), &BattleConfig::set_random_seed);
	ClassDB::bind_method(D_METHOD("get_random_seed"), &BattleConfig::get_random_seed);
	ClassDB::bind_method(D_METHOD("set_grid_data", "grid_data"), &BattleConfig::set_grid_data);
	ClassDB::bind_method(D_METHOD("get_grid_data"), &BattleConfig::get_grid_data);
	ClassDB::bind_method(D_METHOD("set_terrain_library", "terrain_library"), &BattleConfig::set_terrain_library);
	ClassDB::bind_method(D_METHOD("get_terrain_library"), &BattleConfig::get_terrain_library);
	ClassDB::bind_method(D_METHOD("set_rules", "rules"), &BattleConfig::set_rules);
	ClassDB::bind_method(D_METHOD("get_rules"), &BattleConfig::get_rules);
	ClassDB::bind_method(D_METHOD("set_entities_data", "entities_data"), &BattleConfig::set_entities_data);
	ClassDB::bind_method(D_METHOD("get_entities_data"), &BattleConfig::get_entities_data);
	ClassDB::bind_method(D_METHOD("set_max_rounds", "max_rounds"), &BattleConfig::set_max_rounds);
	ClassDB::bind_method(D_METHOD("get_max_rounds"), &BattleConfig::get_max_rounds);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "session_id"), "set_session_id", "get_session_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "random_seed"), "set_random_seed", "get_random_seed");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "grid_data", PROPERTY_HINT_RESOURCE_TYPE, "HexGridMapData"), "set_grid_data", "get_grid_data");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "terrain_library", PROPERTY_HINT_RESOURCE_TYPE, "HexTerrainLibrary"), "set_terrain_library", "get_terrain_library");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "rules", PROPERTY_HINT_RESOURCE_TYPE, "RuleSet"), "set_rules", "get_rules");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "entities_data"), "set_entities_data", "get_entities_data");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_rounds"), "set_max_rounds", "get_max_rounds");
}
