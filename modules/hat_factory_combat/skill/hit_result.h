#ifndef HIT_RESULT_H
#define HIT_RESULT_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/math/vector2i.h"

class HitResult : public RefCounted {
	GDCLASS(HitResult, RefCounted)

	int target_id = 0;
	int damage = 0;
	int healing = 0;
	bool hit = false;
	StringName status_id;
	int status_stacks = 0;
	int status_turns = 0;
	String log;
	Vector2i displacement;
	bool has_displacement = false;
	StringName terrain_id;
	bool has_terrain_change = false;
	Vector2i terrain_coord;

protected:
	static void _bind_methods();

public:
	void set_target_id(int p_value) { target_id = p_value; }
	int get_target_id() const { return target_id; }

	void set_damage(int p_value) { damage = p_value; }
	int get_damage() const { return damage; }

	void set_healing(int p_value) { healing = p_value; }
	int get_healing() const { return healing; }

	void set_hit(bool p_value) { hit = p_value; }
	bool get_hit() const { return hit; }

	void set_status_id(const StringName &p_value) { status_id = p_value; }
	StringName get_status_id() const { return status_id; }

	void set_status_stacks(int p_value) { status_stacks = p_value; }
	int get_status_stacks() const { return status_stacks; }

	void set_status_turns(int p_value) { status_turns = p_value; }
	int get_status_turns() const { return status_turns; }

	void set_log(const String &p_value) { log = p_value; }
	String get_log() const { return log; }
	void set_displacement(const Vector2i &p_value) { displacement = p_value; has_displacement = true; }
	Vector2i get_displacement() const { return displacement; }
	bool get_has_displacement() const { return has_displacement; }
	void set_terrain_id(const StringName &p_value) { terrain_id = p_value; has_terrain_change = true; }
	StringName get_terrain_id() const { return terrain_id; }
	bool get_has_terrain_change() const { return has_terrain_change; }
	void set_terrain_coord(const Vector2i &p_value) { terrain_coord = p_value; }
	Vector2i get_terrain_coord() const { return terrain_coord; }
};

#endif // HIT_RESULT_H
