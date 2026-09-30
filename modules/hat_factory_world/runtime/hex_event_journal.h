/**************************************************************************/
/*  hex_event_journal.h                                                   */
/*  Hat Factory World module — sequenced event log (MAP-06/43).          */
/*                                                                        */
/*  Events are stamped with the world sequence number, logical tick and   */
/*  scale; cross-scale events also carry the world position and stable    */
/*  object ID. Duplicate idempotency IDs are dropped.                     */
/**************************************************************************/
#ifndef HEX_EVENT_JOURNAL_H
#define HEX_EVENT_JOURNAL_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/templates/hash_set.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

class HexEventJournal : public RefCounted {
	GDCLASS(HexEventJournal, RefCounted)
	TypedArray<Dictionary> events;
	HashSet<StringName> idempotency_keys;
	int64_t next_seq = 1;

protected:
	static void _bind_methods();

public:
	/**
	 * Appends an event. p_event should carry: scale, operation, coord (dict),
	 * payload (dict); optional: tick, world_pos, stable_id, idempotency_id.
	 * Returns the assigned world seq, or -1 when dropped (duplicate id).
	 */
	int64_t append(const Dictionary &p_event);
	int get_event_count() const;
	Dictionary get_event(int p_index) const;
	int64_t get_last_seq() const { return next_seq - 1; }

	Dictionary to_dict() const;
	static Ref<HexEventJournal> from_dict(const Dictionary &p_dict);
	void clear();
};

#endif // HEX_EVENT_JOURNAL_H
