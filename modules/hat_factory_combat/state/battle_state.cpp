#include "battle_state.h"

#include "combat_entity.h"
#include "components/combat_attr_component.h"
#include "components/combat_component.h"
#include "components/identity_component.h"
#include "components/movement_component.h"
#include "components/skill_component.h"
#include "components/stats_component.h"
#include "components/status_component.h"
#include "components/transform_component.h"
#include "components/turn_component.h"
#include "components/vision_component.h"
#include "event/game_event.h"
#include "modules/hat_factory_hex_grid/hex_grid_map_data.h"
#include "modules/hat_factory_hex_grid/hex_terrain_library.h"
#include "rule_set.h"
#include "serialization/deterministic_random.h"

#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/variant/typed_array.h"

void BattleState::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_session_id", "session_id"), &BattleState::set_session_id);
	ClassDB::bind_method(D_METHOD("get_session_id"), &BattleState::get_session_id);
	ClassDB::bind_method(D_METHOD("set_random_seed", "random_seed"), &BattleState::set_random_seed);
	ClassDB::bind_method(D_METHOD("get_random_seed"), &BattleState::get_random_seed);
	ClassDB::bind_method(D_METHOD("set_random_index", "random_index"), &BattleState::set_random_index);
	ClassDB::bind_method(D_METHOD("get_random_index"), &BattleState::get_random_index);
	ClassDB::bind_method(D_METHOD("set_round", "round"), &BattleState::set_round);
	ClassDB::bind_method(D_METHOD("get_round"), &BattleState::get_round);
	ClassDB::bind_method(D_METHOD("set_phase", "phase"), &BattleState::set_phase);
	ClassDB::bind_method(D_METHOD("get_phase"), &BattleState::get_phase);
	ClassDB::bind_method(D_METHOD("set_turn_index", "turn_index"), &BattleState::set_turn_index);
	ClassDB::bind_method(D_METHOD("get_turn_index"), &BattleState::get_turn_index);
	ClassDB::bind_method(D_METHOD("set_current_actor", "current_actor"), &BattleState::set_current_actor);
	ClassDB::bind_method(D_METHOD("get_current_actor"), &BattleState::get_current_actor);

	ClassDB::bind_method(D_METHOD("set_grid_data", "grid_data"), &BattleState::set_grid_data);
	ClassDB::bind_method(D_METHOD("get_grid_data"), &BattleState::get_grid_data);
	ClassDB::bind_method(D_METHOD("set_terrain_library", "terrain_library"), &BattleState::set_terrain_library);
	ClassDB::bind_method(D_METHOD("get_terrain_library"), &BattleState::get_terrain_library);
	ClassDB::bind_method(D_METHOD("set_rules", "rules"), &BattleState::set_rules);
	ClassDB::bind_method(D_METHOD("get_rules"), &BattleState::get_rules);

	ClassDB::bind_method(D_METHOD("set_entities", "entities"), &BattleState::set_entities);
	ClassDB::bind_method(D_METHOD("get_entities"), &BattleState::get_entities);
	ClassDB::bind_method(D_METHOD("set_turn_order", "turn_order"), &BattleState::set_turn_order);
	ClassDB::bind_method(D_METHOD("get_turn_order"), &BattleState::get_turn_order);
	ClassDB::bind_method(D_METHOD("set_event_log", "event_log"), &BattleState::set_event_log);
	ClassDB::bind_method(D_METHOD("get_event_log"), &BattleState::get_event_log);
	ClassDB::bind_method(D_METHOD("append_event", "event"), &BattleState::append_event);
	ClassDB::bind_method(D_METHOD("set_rng", "rng"), &BattleState::set_rng);
	ClassDB::bind_method(D_METHOD("get_rng"), &BattleState::get_rng);

	ClassDB::bind_method(D_METHOD("clone"), &BattleState::clone);
	ClassDB::bind_method(D_METHOD("get_entity", "entity_id"), &BattleState::get_entity);
	ClassDB::bind_method(D_METHOD("set_entity", "entity_id", "entity"), &BattleState::set_entity);
	ClassDB::bind_method(D_METHOD("remove_entity", "entity_id"), &BattleState::remove_entity);
	ClassDB::bind_method(D_METHOD("next_random_int", "min", "max"), &BattleState::next_random_int);
	ClassDB::bind_method(D_METHOD("next_random_float"), &BattleState::next_random_float);
	ClassDB::bind_method(D_METHOD("get_events_since", "version"), &BattleState::get_events_since);
	ClassDB::bind_method(D_METHOD("get_event_log_version"), &BattleState::get_event_log_version);
	ClassDB::bind_method(D_METHOD("to_snapshot"), &BattleState::to_snapshot);
	ClassDB::bind_static_method("BattleState", D_METHOD("from_snapshot", "snapshot"), &BattleState::from_snapshot);
	ClassDB::bind_method(D_METHOD("to_json"), &BattleState::to_json);
	ClassDB::bind_static_method("BattleState", D_METHOD("from_json", "json"), &BattleState::from_json);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "session_id"), "set_session_id", "get_session_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "random_seed"), "set_random_seed", "get_random_seed");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "random_index"), "set_random_index", "get_random_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "round"), "set_round", "get_round");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "phase"), "set_phase", "get_phase");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "turn_index"), "set_turn_index", "get_turn_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "current_actor"), "set_current_actor", "get_current_actor");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "grid_data", PROPERTY_HINT_RESOURCE_TYPE, "HexGridMapData"), "set_grid_data", "get_grid_data");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "terrain_library", PROPERTY_HINT_RESOURCE_TYPE, "HexTerrainLibrary"), "set_terrain_library", "get_terrain_library");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "rules", PROPERTY_HINT_RESOURCE_TYPE, "RuleSet"), "set_rules", "get_rules");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "entities", PROPERTY_HINT_ARRAY_TYPE, "CombatEntity"), "set_entities", "get_entities");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "turn_order", PROPERTY_HINT_ARRAY_TYPE, "int"), "set_turn_order", "get_turn_order");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "event_log", PROPERTY_HINT_ARRAY_TYPE, "GameEvent"), "set_event_log", "get_event_log");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "rng", PROPERTY_HINT_RESOURCE_TYPE, "DeterministicRandom"), "set_rng", "get_rng");
}

Ref<CombatEntity> BattleState::_clone_entity_from_dict(const Dictionary &p_dict) {
	return CombatEntity::from_dict(p_dict);
}

void BattleState::set_entities(const TypedArray<CombatEntity> &p_value) {
	entities = p_value;
	entity_map.clear();
	for (int i = 0; i < entities.size(); i++) {
		Ref<CombatEntity> e = entities[i];
		if (e.is_valid()) {
			entity_map[e->get_entity_id()] = e;
		}
	}
}

Ref<BattleState> BattleState::clone() const {
	Ref<BattleState> s;
	s.instantiate();
	s->session_id = session_id;
	s->random_seed = random_seed;
	s->random_index = random_index;
	s->round = round;
	s->phase = phase;
	s->turn_index = turn_index;
	s->current_actor = current_actor;
	s->grid_data = grid_data.is_valid() ? grid_data->clone() : Ref<HexGridMapData>();
	s->terrain_library = terrain_library.is_valid() ? Ref<HexTerrainLibrary>(terrain_library->duplicate()) : Ref<HexTerrainLibrary>();
	s->rules = rules.is_valid() ? rules->clone() : Ref<RuleSet>();
	s->turn_order = turn_order.duplicate();
	s->event_log = event_log.duplicate();
	s->rng = rng.is_valid() ? rng->clone() : Ref<DeterministicRandom>();

	TypedArray<CombatEntity> cloned_entities;
	for (int i = 0; i < entities.size(); i++) {
		Ref<CombatEntity> e = entities[i];
		if (e.is_valid()) {
			Ref<CombatEntity> clone = e->clone();
			cloned_entities.push_back(clone);
			s->entity_map[clone->get_entity_id()] = clone;
		}
	}
	s->entities = cloned_entities;
	return s;
}

CombatEntity *BattleState::get_entity(int p_id) const {
	const Ref<CombatEntity> *ptr = entity_map.getptr(p_id);
	if (ptr == nullptr) {
		return nullptr;
	}
	return ptr->ptr();
}

void BattleState::set_entity(int p_id, const Ref<CombatEntity> &p_entity) {
	if (p_entity.is_null()) {
		remove_entity(p_id);
		return;
	}
	if (p_entity->get_entity_id() != p_id) {
		p_entity->set_entity_id(p_id);
	}
	// Replace existing entry in array.
	for (int i = 0; i < entities.size(); i++) {
		Ref<CombatEntity> e = entities[i];
		if (e.is_valid() && e->get_entity_id() == p_id) {
			entities[i] = p_entity;
			entity_map[p_id] = p_entity;
			return;
		}
	}
	entities.push_back(p_entity);
	entity_map[p_id] = p_entity;
}

void BattleState::remove_entity(int p_id) {
	entity_map.erase(p_id);
	for (int i = entities.size() - 1; i >= 0; i--) {
		Ref<CombatEntity> e = entities[i];
		if (e.is_valid() && e->get_entity_id() == p_id) {
			entities.remove_at(i);
		}
	}
}

int BattleState::next_random_int(int p_min, int p_max) {
	if (rng.is_null()) {
		rng.instantiate();
		rng->set_seed(random_seed);
	}
	rng->set_index(random_index);
	int value = rng->next_int(p_min, p_max);
	random_index = rng->get_index();
	return value;
}

float BattleState::next_random_float() {
	if (rng.is_null()) {
		rng.instantiate();
		rng->set_seed(random_seed);
	}
	rng->set_index(random_index);
	float value = rng->next_float();
	random_index = rng->get_index();
	return value;
}

TypedArray<GameEvent> BattleState::get_events_since(int p_version) const {
	TypedArray<GameEvent> result;
	for (int i = p_version; i < event_log.size(); i++) {
		result.push_back(event_log[i]);
	}
	return result;
}

int BattleState::get_event_log_version() const {
	return event_log.size();
}

Dictionary BattleState::to_snapshot() const {
	Dictionary d;
	d["version"] = "1.0.0";
	d["session_id"] = session_id;
	d["random_seed"] = random_seed;
	d["random_index"] = random_index;
	d["round"] = round;
	d["phase"] = phase;
	d["turn_index"] = turn_index;
	d["current_actor"] = current_actor;

	if (grid_data.is_valid()) {
		Dictionary gd;
		gd["cells"] = grid_data->get_cells();
		gd["default_terrain"] = grid_data->get_default_terrain();
		d["grid_data"] = gd;
	}
	if (terrain_library.is_valid()) {
		Dictionary tl;
		tl["terrains"] = terrain_library->get_terrains();
		d["terrain_library"] = tl;
	}
	if (rules.is_valid()) {
		d["rules"] = rules->to_dict();
	}

	Dictionary entity_dict;
	for (int i = 0; i < entities.size(); i++) {
		Ref<CombatEntity> e = entities[i];
		if (e.is_valid()) {
			entity_dict[itos(e->get_entity_id())] = e->to_dict();
		}
	}
	d["entities"] = entity_dict;

	Array order;
	for (int i = 0; i < turn_order.size(); i++) {
		order.push_back(turn_order[i]);
	}
	d["turn_order"] = order;

	Array events;
	for (int i = 0; i < event_log.size(); i++) {
		Ref<GameEvent> ev = event_log[i];
		if (ev.is_valid()) {
			events.push_back(ev->to_dict());
		} else {
			events.push_back(Dictionary());
		}
	}
	d["event_log"] = events;

	if (rng.is_valid()) {
		d["rng"] = rng->to_dict();
	}

	return d;
}

Ref<BattleState> BattleState::from_snapshot(const Dictionary &p_snapshot) {
	Ref<BattleState> s;
	s.instantiate();

	if (p_snapshot.has("session_id")) s->session_id = p_snapshot["session_id"];
	if (p_snapshot.has("random_seed")) s->random_seed = p_snapshot["random_seed"];
	if (p_snapshot.has("random_index")) s->random_index = p_snapshot["random_index"];
	if (p_snapshot.has("round")) s->round = p_snapshot["round"];
	if (p_snapshot.has("phase")) s->phase = p_snapshot["phase"];
	if (p_snapshot.has("turn_index")) s->turn_index = p_snapshot["turn_index"];
	if (p_snapshot.has("current_actor")) s->current_actor = p_snapshot["current_actor"];

	if (p_snapshot.has("grid_data")) {
		Dictionary gd = p_snapshot["grid_data"];
		s->grid_data = HexGridMapData::from_dict(gd);
	}
	if (p_snapshot.has("terrain_library")) {
		Dictionary tl = p_snapshot["terrain_library"];
		Ref<HexTerrainLibrary> lib;
		lib.instantiate();
		if (tl.has("terrains")) lib->set_terrains(tl["terrains"]);
		s->terrain_library = lib;
	}
	if (p_snapshot.has("rules")) {
		s->rules = RuleSet::from_dict(p_snapshot["rules"]);
	}

	if (p_snapshot.has("entities")) {
		Dictionary entity_dict = p_snapshot["entities"];
		Array keys = entity_dict.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			Dictionary edict = entity_dict[key];
			Ref<CombatEntity> e = _clone_entity_from_dict(edict);
			s->entities.push_back(e);
			s->entity_map[e->get_entity_id()] = e;
		}
	}

	if (p_snapshot.has("turn_order")) {
		Array order = p_snapshot["turn_order"];
		for (int i = 0; i < order.size(); i++) {
			s->turn_order.push_back(order[i]);
		}
	}

	if (p_snapshot.has("event_log")) {
		Array events = p_snapshot["event_log"];
		for (int i = 0; i < events.size(); i++) {
			Dictionary evdict = events[i];
			Ref<GameEvent> ev = GameEvent::from_dict(evdict);
			s->event_log.push_back(ev);
		}
	}

	if (p_snapshot.has("rng")) {
		s->rng = DeterministicRandom::from_dict(p_snapshot["rng"]);
	} else {
		s->rng.instantiate();
		s->rng->set_seed(s->random_seed);
		s->rng->set_index(s->random_index);
	}

	return s;
}

String BattleState::to_json() const {
	return JSON::stringify(to_snapshot(), "\t", false, true);
}

Ref<BattleState> BattleState::from_json(const String &p_json) {
	Variant parsed = JSON::parse_string(p_json);
	if (parsed.get_type() != Variant::DICTIONARY) {
		return Ref<BattleState>();
	}
	return from_snapshot(parsed);
}
