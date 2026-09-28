#ifndef HEX_TRANSITION_DEF_H
#define HEX_TRANSITION_DEF_H

#include "hex_transition_profile.h"
#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/typed_array.h"

/**
 * @brief A single transition rule.
 *
 * Defines when a transition should appear between two adjacent cells
 * and which visual profile + gameplay metadata to attach.
 */
class HexTransitionDef : public Resource {
	GDCLASS(HexTransitionDef, Resource)

	PackedStringArray source_tags;
	PackedStringArray target_tags;
	StringName source_terrain_id;
	StringName target_terrain_id;

	int min_height_delta = 1;
	int max_height_delta = INT_MAX;
	bool require_same_terrain = false;
	bool require_different_terrain = false;
	float probability = 1.0f;
	int priority = 0;

	Ref<HexTransitionProfile> profile;
	uint32_t gameplay_flags = 0;
	StringName gameplay_state_id;

protected:
	static void _bind_methods();

public:
	void set_source_tags(const PackedStringArray &p_tags);
	PackedStringArray get_source_tags() const;

	void set_target_tags(const PackedStringArray &p_tags);
	PackedStringArray get_target_tags() const;

	void set_source_terrain_id(const StringName &p_id);
	StringName get_source_terrain_id() const;

	void set_target_terrain_id(const StringName &p_id);
	StringName get_target_terrain_id() const;

	void set_min_height_delta(int p_delta);
	int get_min_height_delta() const;

	void set_max_height_delta(int p_delta);
	int get_max_height_delta() const;

	void set_require_same_terrain(bool p_enabled);
	bool get_require_same_terrain() const;

	void set_require_different_terrain(bool p_enabled);
	bool get_require_different_terrain() const;

	void set_probability(float p_probability);
	float get_probability() const;

	void set_priority(int p_priority);
	int get_priority() const;

	void set_profile(const Ref<HexTransitionProfile> &p_profile);
	Ref<HexTransitionProfile> get_profile() const;

	void set_gameplay_flags(uint32_t p_flags);
	uint32_t get_gameplay_flags() const;

	void set_gameplay_state_id(const StringName &p_id);
	StringName get_gameplay_state_id() const;
};

#endif // HEX_TRANSITION_DEF_H
