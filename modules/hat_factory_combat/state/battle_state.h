#ifndef BATTLE_STATE_H
#define BATTLE_STATE_H

#include "core/math/vector2i.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

#include "combat_entity.h"
#include "modules/hat_factory_combat/event/game_event.h"
#include "modules/hat_factory_hex_grid/hex_grid_map_data.h"
#include "modules/hat_factory_hex_grid/hex_terrain_library.h"
#include "modules/hat_factory_combat/serialization/deterministic_random.h"
#include "rule_set.h"

class BattleState : public RefCounted {
	GDCLASS(BattleState, RefCounted)

	String session_id;
	int random_seed = 0;
	int random_index = 0;
	int round = 1;
	StringName phase = StringName("init");
	int turn_index = 0;
	int current_actor = 0;

	Ref<HexGridMapData> grid_data;
	Ref<HexTerrainLibrary> terrain_library;
	Ref<RuleSet> rules;

	TypedArray<CombatEntity> entities;
	HashMap<int, Ref<CombatEntity>> entity_map;
	TypedArray<int> turn_order;
	TypedArray<GameEvent> event_log;

	Ref<DeterministicRandom> rng;

protected:
	static void _bind_methods();

	static Ref<CombatEntity> _clone_entity_from_dict(const Dictionary &p_dict);

public:
	void set_session_id(const String &p_value) { session_id = p_value; }
	String get_session_id() const { return session_id; }

	void set_random_seed(int p_value) { random_seed = p_value; }
	int get_random_seed() const { return random_seed; }

	void set_random_index(int p_value) { random_index = p_value; }
	int get_random_index() const { return random_index; }

	void set_round(int p_value) { round = p_value; }
	int get_round() const { return round; }

	void set_phase(const StringName &p_value) { phase = p_value; }
	StringName get_phase() const { return phase; }

	void set_turn_index(int p_value) { turn_index = p_value; }
	int get_turn_index() const { return turn_index; }

	void set_current_actor(int p_value) { current_actor = p_value; }
	int get_current_actor() const { return current_actor; }

	void set_grid_data(const Ref<HexGridMapData> &p_value) { grid_data = p_value; }
	Ref<HexGridMapData> get_grid_data() const { return grid_data; }

	void set_terrain_library(const Ref<HexTerrainLibrary> &p_value) { terrain_library = p_value; }
	Ref<HexTerrainLibrary> get_terrain_library() const { return terrain_library; }

	void set_rules(const Ref<RuleSet> &p_value) { rules = p_value; }
	Ref<RuleSet> get_rules() const { return rules; }

	void set_entities(const TypedArray<CombatEntity> &p_value);
	TypedArray<CombatEntity> get_entities() const { return entities; }

	void set_turn_order(const TypedArray<int> &p_value) { turn_order = p_value; }
	TypedArray<int> get_turn_order() const { return turn_order; }

	void set_event_log(const TypedArray<GameEvent> &p_value) { event_log = p_value; }
	TypedArray<GameEvent> get_event_log() const { return event_log; }
	TypedArray<GameEvent> &get_event_log_ref() { return event_log; }
	const TypedArray<GameEvent> &get_event_log_ref() const { return event_log; }
	void append_event(const Ref<GameEvent> &p_event) { if (p_event.is_valid()) event_log.push_back(p_event); }

	void set_rng(const Ref<DeterministicRandom> &p_value) { rng = p_value; }
	Ref<DeterministicRandom> get_rng() const { return rng; }

	Ref<BattleState> clone() const;

	CombatEntity *get_entity(int p_id) const;
	void set_entity(int p_id, const Ref<CombatEntity> &p_entity);
	void remove_entity(int p_id);

	int next_random_int(int p_min, int p_max);
	float next_random_float();

	TypedArray<GameEvent> get_events_since(int p_version) const;
	int get_event_log_version() const;

	Dictionary to_snapshot() const;
	static Ref<BattleState> from_snapshot(const Dictionary &p_snapshot);

	String to_json() const;
	static Ref<BattleState> from_json(const String &p_json);
};

#endif // BATTLE_STATE_H
