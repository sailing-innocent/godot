#include "rl_bridge.h"

#include "core/object/class_db.h"
#include "modules/hat_factory_combat/action/battle_action.h"
#include "modules/hat_factory_combat/event/game_event.h"
#include "modules/hat_factory_combat/serialization/battle_snapshot.h"
#include "modules/hat_factory_combat/state/battle_state.h"

void RLBridge::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_endpoint_url", "endpoint_url"), &RLBridge::set_endpoint_url);
	ClassDB::bind_method(D_METHOD("get_endpoint_url"), &RLBridge::get_endpoint_url);
	ClassDB::bind_method(D_METHOD("set_timeout_seconds", "timeout_seconds"), &RLBridge::set_timeout_seconds);
	ClassDB::bind_method(D_METHOD("get_timeout_seconds"), &RLBridge::get_timeout_seconds);
	ClassDB::bind_method(D_METHOD("request_action", "state"), &RLBridge::request_action);
	ClassDB::bind_method(D_METHOD("send_observation", "state", "events"), &RLBridge::send_observation);
	ClassDB::bind_method(D_METHOD("request_action_sync", "state"), &RLBridge::request_action_sync);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "endpoint_url"), "set_endpoint_url", "get_endpoint_url");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "timeout_seconds"), "set_timeout_seconds", "get_timeout_seconds");
}

Ref<BattleAction> RLBridge::request_action(const Ref<BattleState> &p_state) {
	// Async placeholder: synchronous implementation is used for self-play.
	return request_action_sync(p_state);
}

void RLBridge::send_observation(const Ref<BattleState> &p_state, const TypedArray<GameEvent> &p_events) {
	// Placeholder: snapshot is serialized but not actually sent in this stub.
	if (p_state.is_null()) return;
	BattleSnapshot::state_to_json(p_state, false);
}

Ref<BattleAction> RLBridge::request_action_sync(const Ref<BattleState> &p_state) {
	if (p_state.is_null()) {
		return Ref<BattleAction>();
	}
	// Stub: return end_turn for current actor. Production connects to endpoint_url.
	return BattleAction::end_turn(p_state->get_current_actor());
}
