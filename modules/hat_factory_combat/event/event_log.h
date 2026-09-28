#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

#include "game_event.h"

class EventLog : public RefCounted {
	GDCLASS(EventLog, RefCounted)

	TypedArray<GameEvent> events;

protected:
	static void _bind_methods();

public:
	void append(const Ref<GameEvent> &p_event);
	void clear();
	TypedArray<GameEvent> get_events_since(int p_version) const;
	int get_version() const;

	void set_events(const TypedArray<GameEvent> &p_value) { events = p_value; }
	TypedArray<GameEvent> get_events() const { return events; }
};

#endif // EVENT_LOG_H
