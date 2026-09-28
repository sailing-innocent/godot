#ifndef BATTLE_CONFIG_H
#define BATTLE_CONFIG_H

#include "core/io/resource.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"

#include "modules/hat_factory_hex_grid/hex_grid_map_data.h"
#include "modules/hat_factory_hex_grid/hex_terrain_library.h"
#include "modules/hat_factory_combat/state/rule_set.h"

class BattleConfig : public Resource {
	GDCLASS(BattleConfig, Resource)

	String session_id;
	int random_seed = 0;
	Ref<HexGridMapData> grid_data;
	Ref<HexTerrainLibrary> terrain_library;
	Ref<RuleSet> rules;
	Array entities_data;
	int max_rounds = 99;

protected:
	static void _bind_methods();

public:
	void set_session_id(const String &p_value) { session_id = p_value; }
	String get_session_id() const { return session_id; }

	void set_random_seed(int p_value) { random_seed = p_value; }
	int get_random_seed() const { return random_seed; }

	void set_grid_data(const Ref<HexGridMapData> &p_value) { grid_data = p_value; }
	Ref<HexGridMapData> get_grid_data() const { return grid_data; }

	void set_terrain_library(const Ref<HexTerrainLibrary> &p_value) { terrain_library = p_value; }
	Ref<HexTerrainLibrary> get_terrain_library() const { return terrain_library; }

	void set_rules(const Ref<RuleSet> &p_value) { rules = p_value; }
	Ref<RuleSet> get_rules() const { return rules; }

	void set_entities_data(const Array &p_value) { entities_data = p_value; }
	Array get_entities_data() const { return entities_data; }

	void set_max_rounds(int p_value) { max_rounds = p_value; }
	int get_max_rounds() const { return max_rounds; }
};

#endif // BATTLE_CONFIG_H
