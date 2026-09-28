#include "action_result.h"

#include "battle_action.h"
#include "event/game_event.h"
#include "state/battle_state.h"

#include "core/io/json.h"
#include "core/object/class_db.h"

void ActionResult::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_accepted", "accepted"), &ActionResult::set_accepted);
	ClassDB::bind_method(D_METHOD("get_accepted"), &ActionResult::get_accepted);
	ClassDB::bind_method(D_METHOD("set_reason", "reason"), &ActionResult::set_reason);
	ClassDB::bind_method(D_METHOD("get_reason"), &ActionResult::get_reason);
	ClassDB::bind_method(D_METHOD("set_next_state", "next_state"), &ActionResult::set_next_state);
	ClassDB::bind_method(D_METHOD("get_next_state"), &ActionResult::get_next_state);
	ClassDB::bind_method(D_METHOD("set_events", "events"), &ActionResult::set_events);
	ClassDB::bind_method(D_METHOD("get_events"), &ActionResult::get_events);
	ClassDB::bind_method(D_METHOD("append_event", "event"), &ActionResult::append_event);
	ClassDB::bind_method(D_METHOD("set_action", "action"), &ActionResult::set_action);
	ClassDB::bind_method(D_METHOD("get_action"), &ActionResult::get_action);

	ClassDB::bind_method(D_METHOD("to_dict"), &ActionResult::to_dict);
	ClassDB::bind_static_method("ActionResult", D_METHOD("from_dict", "dict"), &ActionResult::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &ActionResult::clone);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "accepted"), "set_accepted", "get_accepted");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "reason"), "set_reason", "get_reason");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "next_state", PROPERTY_HINT_RESOURCE_TYPE, "BattleState"), "set_next_state", "get_next_state");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "events", PROPERTY_HINT_ARRAY_TYPE, "GameEvent"), "set_events", "get_events");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "action", PROPERTY_HINT_RESOURCE_TYPE, "BattleAction"), "set_action", "get_action");
}

Dictionary ActionResult::to_dict() const {
	Dictionary d;
	d["accepted"] = accepted;
	d["reason"] = reason;
	if (next_state.is_valid()) {
		d["next_state"] = next_state->to_snapshot();
	}
	Array evs;
	for (int i = 0; i < events.size(); i++) {
		Ref<GameEvent> ev = events[i];
		if (ev.is_valid()) {
			evs.push_back(ev->to_dict());
		}
	}
	d["events"] = evs;
	if (action.is_valid()) {
		d["action"] = action->to_dict();
	}
	return d;
}

Ref<ActionResult> ActionResult::from_dict(const Dictionary &p_dict) {
	Ref<ActionResult> r;
	r.instantiate();
	if (p_dict.has("accepted")) r->accepted = p_dict["accepted"];
	if (p_dict.has("reason")) r->reason = p_dict["reason"];
	if (p_dict.has("next_state")) {
		r->next_state = BattleState::from_snapshot(p_dict["next_state"]);
	}
	if (p_dict.has("events")) {
		Array evs = p_dict["events"];
		for (int i = 0; i < evs.size(); i++) {
			Ref<GameEvent> ev = GameEvent::from_dict(evs[i]);
			r->events.push_back(ev);
		}
	}
	if (p_dict.has("action")) {
		r->action = BattleAction::from_dict(p_dict["action"]);
	}
	return r;
}

Ref<ActionResult> ActionResult::clone() const {
	Ref<ActionResult> r;
	r.instantiate();
	r->accepted = accepted;
	r->reason = reason;
	r->next_state = next_state.is_valid() ? next_state->clone() : Ref<BattleState>();
	r->events = events.duplicate();
	r->action = action.is_valid() ? action->clone() : Ref<BattleAction>();
	return r;
}
