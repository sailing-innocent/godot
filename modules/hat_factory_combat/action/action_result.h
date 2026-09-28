#ifndef ACTION_RESULT_H
#define ACTION_RESULT_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/typed_array.h"

#include "battle_action.h"
#include "modules/hat_factory_combat/event/game_event.h"
#include "modules/hat_factory_combat/state/battle_state.h"

class ActionResult : public RefCounted {
	GDCLASS(ActionResult, RefCounted)

	bool accepted = false;
	String reason;
	Ref<BattleState> next_state;
	TypedArray<GameEvent> events;
	Ref<BattleAction> action;

protected:
	static void _bind_methods();

public:
	void set_accepted(bool p_value) { accepted = p_value; }
	bool get_accepted() const { return accepted; }

	void set_reason(const String &p_value) { reason = p_value; }
	String get_reason() const { return reason; }

	void set_next_state(const Ref<BattleState> &p_value) { next_state = p_value; }
	Ref<BattleState> get_next_state() const { return next_state; }

	void set_events(const TypedArray<GameEvent> &p_value) { events = p_value; }
	TypedArray<GameEvent> get_events() const { return events; }
	void append_event(const Ref<GameEvent> &p_event) { if (p_event.is_valid()) events.push_back(p_event); }

	void set_action(const Ref<BattleAction> &p_value) { action = p_value; }
	Ref<BattleAction> get_action() const { return action; }

	Dictionary to_dict() const;
	static Ref<ActionResult> from_dict(const Dictionary &p_dict);
	Ref<ActionResult> clone() const;
};

#endif // ACTION_RESULT_H
