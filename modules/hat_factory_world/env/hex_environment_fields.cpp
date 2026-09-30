/**************************************************************************/
/*  hex_environment_fields.cpp                                            */
/**************************************************************************/
#include "hex_environment_fields.h"

#include "core/object/class_db.h"

void HexEnvironmentFields::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_water_level", "level"), &HexEnvironmentFields::set_water_level);
	ClassDB::bind_method(D_METHOD("get_water_level"), &HexEnvironmentFields::get_water_level);
	ClassDB::bind_method(D_METHOD("set_sun_time", "hours"), &HexEnvironmentFields::set_sun_time);
	ClassDB::bind_method(D_METHOD("get_sun_time"), &HexEnvironmentFields::get_sun_time);
	ClassDB::bind_method(D_METHOD("set_wind", "speed", "direction"), &HexEnvironmentFields::set_wind);
	ClassDB::bind_method(D_METHOD("get_wind_speed"), &HexEnvironmentFields::get_wind_speed);
	ClassDB::bind_method(D_METHOD("get_wind_direction"), &HexEnvironmentFields::get_wind_direction);
	ClassDB::bind_method(D_METHOD("get_tick"), &HexEnvironmentFields::get_tick);
	ClassDB::bind_method(D_METHOD("sample", "world_pos"), &HexEnvironmentFields::sample);
	ClassDB::bind_method(D_METHOD("submit_rule_event", "kind", "magnitude", "region"), &HexEnvironmentFields::submit_rule_event, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("get_pending_event_count"), &HexEnvironmentFields::get_pending_event_count);
	ClassDB::bind_method(D_METHOD("advance_tick"), &HexEnvironmentFields::advance_tick);
	ClassDB::bind_method(D_METHOD("set_frozen", "frozen"), &HexEnvironmentFields::set_frozen);
	ClassDB::bind_method(D_METHOD("inject_battle_event", "kind", "magnitude"), &HexEnvironmentFields::inject_battle_event);
	ClassDB::bind_method(D_METHOD("has_battle_event"), &HexEnvironmentFields::has_battle_event);
	ClassDB::bind_method(D_METHOD("take_battle_event"), &HexEnvironmentFields::take_battle_event);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexEnvironmentFields::to_dict);
	ClassDB::bind_static_method("HexEnvironmentFields", D_METHOD("from_dict", "dict"), &HexEnvironmentFields::from_dict);
}

void HexEnvironmentFields::set_water_level(double p_level) { water_level = p_level; }
double HexEnvironmentFields::get_water_level() const { return water_level; }
void HexEnvironmentFields::set_sun_time(double p_hours) { sun_time = Math::fposmod(p_hours, 24.0); }
double HexEnvironmentFields::get_sun_time() const { return sun_time; }
void HexEnvironmentFields::set_wind(double p_speed, const Vector2 &p_direction) {
	wind_speed = p_speed;
	wind_direction = p_direction.length() > 1e-6 ? p_direction.normalized() : Vector2(1, 0);
}
double HexEnvironmentFields::get_wind_speed() const { return wind_speed; }
Vector2 HexEnvironmentFields::get_wind_direction() const { return wind_direction; }
int64_t HexEnvironmentFields::get_tick() const { return tick; }

Dictionary HexEnvironmentFields::sample(const Vector3 &p_world_pos) const {
	// v1 fields are global-low-frequency: position anchors the sample for
	// future per-region fields (MAP-47); values stay continuous across shards.
	Dictionary s;
	s[StringName("water_level")] = water_level;
	s[StringName("sun_time")] = sun_time;
	s[StringName("wind_speed")] = wind_speed;
	s[StringName("wind_direction")] = wind_direction;
	s[StringName("tick")] = tick;
	s[StringName("world_pos")] = p_world_pos;
	return s;
}

void HexEnvironmentFields::submit_rule_event(const StringName &p_kind, double p_magnitude, const Dictionary &p_region) {
	RuleEvent ev;
	ev.tick = tick + 1; // applies on the next advance
	ev.kind = p_kind;
	ev.magnitude = p_magnitude;
	ev.region = p_region;
	pending.push_back(ev);
}

int HexEnvironmentFields::get_pending_event_count() const { return pending.size(); }

bool HexEnvironmentFields::advance_tick() {
	if (frozen) {
		return false; // battle freeze: fields do not advance (D4)
	}
	tick++;
	// Deterministic settlement: tide moves water level, freeze lowers it on
	// ice worlds, etc. Same event sequence => same field values, always.
	for (int i = 0; i < pending.size(); i++) {
		const RuleEvent &ev = pending[i];
		if (ev.kind == StringName("tide")) {
			water_level = ev.magnitude;
		} else if (ev.kind == StringName("sun_time")) {
			set_sun_time(ev.magnitude);
		} else if (ev.kind == StringName("wind")) {
			wind_speed = ev.magnitude;
		}
		// "puddle"/"freeze" materialize through the map command resolver as
		// surface deltas — never through GPU or particle write-back.
	}
	pending.clear();
	return true;
}

void HexEnvironmentFields::set_frozen(bool p_frozen) { frozen = p_frozen; }

bool HexEnvironmentFields::inject_battle_event(const StringName &p_kind, double p_magnitude) {
	if (!frozen || battle_event_pending) {
		return false; // one explicit injectable event per battle (D4)
	}
	RuleEvent ev;
	ev.tick = tick + 1;
	ev.kind = p_kind;
	ev.magnitude = p_magnitude;
	battle_event = ev; // separate channel, not the rule queue
	battle_event_pending = true;
	return true;
}

Dictionary HexEnvironmentFields::take_battle_event() {
	battle_event_pending = false;
	Dictionary d;
	if (battle_event.kind != StringName()) {
		d[StringName("tick")] = battle_event.tick;
		d[StringName("kind")] = battle_event.kind;
		d[StringName("magnitude")] = battle_event.magnitude;
		battle_event = RuleEvent();
	}
	return d;
}

Dictionary HexEnvironmentFields::to_dict() const {
	Dictionary d;
	d[StringName("water_level")] = water_level;
	d[StringName("sun_time")] = sun_time;
	d[StringName("wind_speed")] = wind_speed;
	d[StringName("wind_direction")] = wind_direction;
	d[StringName("tick")] = tick;
	d[StringName("frozen")] = frozen;
	return d;
}

Ref<HexEnvironmentFields> HexEnvironmentFields::from_dict(const Dictionary &p_dict) {
	Ref<HexEnvironmentFields> f;
	f.instantiate();
	f->set_water_level(p_dict.get(StringName("water_level"), 0.0));
	f->set_sun_time(p_dict.get(StringName("sun_time"), 0.0));
	const Vector2 wd = p_dict.get(StringName("wind_direction"), Vector2(1, 0));
	f->set_wind(p_dict.get(StringName("wind_speed"), 0.0), wd);
	f->tick = p_dict.get(StringName("tick"), (int64_t)0);
	f->frozen = p_dict.get(StringName("frozen"), false);
	return f;
}
