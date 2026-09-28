#include "event_log.h"

#include "game_event.h"

#include "core/object/class_db.h"

void EventLog::_bind_methods() {
	ClassDB::bind_method(D_METHOD("append", "event"), &EventLog::append);
	ClassDB::bind_method(D_METHOD("clear"), &EventLog::clear);
	ClassDB::bind_method(D_METHOD("get_events_since", "version"), &EventLog::get_events_since);
	ClassDB::bind_method(D_METHOD("get_version"), &EventLog::get_version);
	ClassDB::bind_method(D_METHOD("set_events", "events"), &EventLog::set_events);
	ClassDB::bind_method(D_METHOD("get_events"), &EventLog::get_events);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "events", PROPERTY_HINT_ARRAY_TYPE, "GameEvent"), "set_events", "get_events");
}

void EventLog::append(const Ref<GameEvent> &p_event) {
	if (p_event.is_valid()) {
		events.push_back(p_event);
	}
}

void EventLog::clear() {
	events.clear();
}

TypedArray<GameEvent> EventLog::get_events_since(int p_version) const {
	TypedArray<GameEvent> result;
	for (int i = p_version; i < events.size(); i++) {
		result.push_back(events[i]);
	}
	return result;
}

int EventLog::get_version() const {
	return events.size();
}
