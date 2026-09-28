#ifndef RL_BRIDGE_H
#define RL_BRIDGE_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/typed_array.h"

class BattleState;
class BattleAction;
class GameEvent;

class RLBridge : public RefCounted {
	GDCLASS(RLBridge, RefCounted)

	String endpoint_url;
	float timeout_seconds = 5.0f;

protected:
	static void _bind_methods();

public:
	void set_endpoint_url(const String &p_url) { endpoint_url = p_url; }
	String get_endpoint_url() const { return endpoint_url; }

	void set_timeout_seconds(float p_timeout) { timeout_seconds = p_timeout; }
	float get_timeout_seconds() const { return timeout_seconds; }

	Ref<BattleAction> request_action(const Ref<BattleState> &p_state);
	void send_observation(const Ref<BattleState> &p_state, const TypedArray<GameEvent> &p_events);
	Ref<BattleAction> request_action_sync(const Ref<BattleState> &p_state);
};

#endif // RL_BRIDGE_H
