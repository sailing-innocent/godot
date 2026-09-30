/**************************************************************************/
/*  hex_environment_fields.h                                              */
/*  Hat Factory World module — low-frequency global fields (MAP-47, D4).  */
/*                                                                        */
/*  World-anchored scalar fields (water level / sun time / wind) sampled  */
/*  per world position, plus deterministic rule events (tide / puddle /   */
/*  freeze) with versioned ticks. Battles freeze the fields; rule effects  */
/*  enter through one explicit injectable event per battle (replayable).   */
/*  GPU waves/particles/fluid must NEVER write back into these fields.     */
/**************************************************************************/
#ifndef HEX_ENVIRONMENT_FIELDS_H
#define HEX_ENVIRONMENT_FIELDS_H

#include "core/object/ref_counted.h"
#include "core/templates/vector.h"

class HexEnvironmentFields : public RefCounted {
	GDCLASS(HexEnvironmentFields, RefCounted)
public:
	struct RuleEvent {
		int64_t tick = 0;
		StringName kind; // "tide" | "puddle" | "freeze" | ...
		double magnitude = 0.0;
		Dictionary region; // optional world-space domain
	};

private:
	double water_level = 0.0; // world meters
	double sun_time = 0.0; // hours [0, 24)
	double wind_speed = 0.0;
	Vector2 wind_direction = Vector2(1, 0);
	int64_t tick = 0;
	bool frozen = false;
	Vector<RuleEvent> pending; // deterministic, replayable
	// D4 battle injection lives on its own channel: it must never be mixed
	// with queued rule events (take_battle_event returns THE injected event).
	RuleEvent battle_event;
	bool battle_event_pending = false;

public:
	void set_water_level(double p_level);
	double get_water_level() const;
	void set_sun_time(double p_hours);
	double get_sun_time() const;
	void set_wind(double p_speed, const Vector2 &p_direction);
	double get_wind_speed() const;
	Vector2 get_wind_direction() const;
	int64_t get_tick() const;

	Dictionary sample(const Vector3 &p_world_pos) const;

	/** Deterministic rule-event queue (same seed + same ticks => same fields). */
	void submit_rule_event(const StringName &p_kind, double p_magnitude, const Dictionary &p_region = Dictionary());
	int get_pending_event_count() const;
	/** Advances one logical tick; frozen fields reject advancement (D4). */
	bool advance_tick();
	bool is_frozen() const { return frozen; }
	void set_frozen(bool p_frozen);

	/**
	 * Battle integration (D4): while frozen, rule effects do not advance;
	 * the battle receives exactly one injectable event at an action point,
	 * which must be replayable from the journal.
	 */
	bool inject_battle_event(const StringName &p_kind, double p_magnitude);
	bool has_battle_event() const { return battle_event_pending; }
	/** Pending battle event as a Dictionary ({tick, kind, magnitude}). */
	Dictionary take_battle_event();

	Dictionary to_dict() const;
	static Ref<HexEnvironmentFields> from_dict(const Dictionary &p_dict);

protected:
	static void _bind_methods();
};

#endif // HEX_ENVIRONMENT_FIELDS_H
