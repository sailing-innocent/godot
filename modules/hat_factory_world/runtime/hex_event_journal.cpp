/**************************************************************************/
/*  hex_event_journal.cpp                                                 */
/**************************************************************************/
#include "hex_event_journal.h"

#include "core/object/class_db.h"

void HexEventJournal::_bind_methods() {
	ClassDB::bind_method(D_METHOD("append", "event"), &HexEventJournal::append);
	ClassDB::bind_method(D_METHOD("get_event_count"), &HexEventJournal::get_event_count);
	ClassDB::bind_method(D_METHOD("get_event", "index"), &HexEventJournal::get_event);
	ClassDB::bind_method(D_METHOD("get_last_seq"), &HexEventJournal::get_last_seq);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexEventJournal::to_dict);
	ClassDB::bind_static_method("HexEventJournal", D_METHOD("from_dict", "dict"), &HexEventJournal::from_dict);
	ClassDB::bind_method(D_METHOD("clear"), &HexEventJournal::clear);
}

int64_t HexEventJournal::append(const Dictionary &p_event) {
	const StringName idem = p_event.get(StringName("idempotency_id"), StringName());
	if (idem != StringName()) {
		if (idempotency_keys.has(idem)) {
			return -1; // duplicate delivery: same result as the first apply
		}
		idempotency_keys.insert(idem);
	}
	Dictionary stamped = p_event;
	stamped[StringName("seq")] = next_seq;
	events.append(stamped);
	return next_seq++;
}

int HexEventJournal::get_event_count() const { return events.size(); }

Dictionary HexEventJournal::get_event(int p_index) const {
	if (p_index < 0 || p_index >= events.size()) {
		return Dictionary();
	}
	return events[p_index];
}

Dictionary HexEventJournal::to_dict() const {
	Dictionary d;
	d[StringName("next_seq")] = next_seq;
	d[StringName("events")] = events;
	return d;
}

Ref<HexEventJournal> HexEventJournal::from_dict(const Dictionary &p_dict) {
	Ref<HexEventJournal> j;
	j.instantiate();
	j->next_seq = p_dict.get(StringName("next_seq"), (int64_t)1);
	j->events = p_dict.get(StringName("events"), TypedArray<Dictionary>());
	for (int i = 0; i < j->events.size(); i++) {
		Dictionary ev = j->events[i];
		const StringName idem = ev.get(StringName("idempotency_id"), StringName());
		if (idem != StringName()) {
			j->idempotency_keys.insert(idem);
		}
	}
	return j;
}

void HexEventJournal::clear() {
	events.clear();
	idempotency_keys.clear();
	next_seq = 1;
}
