#include "battle_snapshot.h"

#include "state/battle_state.h"

#include "core/io/json.h"
#include "core/object/class_db.h"

void BattleSnapshot::_bind_methods() {
	ClassDB::bind_static_method("BattleSnapshot", D_METHOD("state_to_json", "state", "pretty"), &BattleSnapshot::state_to_json, DEFVAL(true));
	ClassDB::bind_static_method("BattleSnapshot", D_METHOD("state_from_json", "json"), &BattleSnapshot::state_from_json);
	ClassDB::bind_static_method("BattleSnapshot", D_METHOD("state_to_dict", "state"), &BattleSnapshot::state_to_dict);
	ClassDB::bind_static_method("BattleSnapshot", D_METHOD("state_from_dict", "dict"), &BattleSnapshot::state_from_dict);
}

String BattleSnapshot::state_to_json(const Ref<BattleState> &p_state, bool p_pretty) {
	if (p_state.is_null()) {
		return String();
	}
	String indent = p_pretty ? "\t" : "";
	return JSON::stringify(p_state->to_snapshot(), indent, false, true);
}

Ref<BattleState> BattleSnapshot::state_from_json(const String &p_json) {
	return BattleState::from_json(p_json);
}

Dictionary BattleSnapshot::state_to_dict(const Ref<BattleState> &p_state) {
	if (p_state.is_null()) {
		return Dictionary();
	}
	return p_state->to_snapshot();
}

Ref<BattleState> BattleSnapshot::state_from_dict(const Dictionary &p_dict) {
	return BattleState::from_snapshot(p_dict);
}
