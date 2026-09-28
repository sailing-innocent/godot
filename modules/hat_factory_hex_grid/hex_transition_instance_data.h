#ifndef HEX_TRANSITION_INSTANCE_DATA_H
#define HEX_TRANSITION_INSTANCE_DATA_H

#include "hex_transition_profile.h"
#include "core/io/resource.h"
#include "core/math/transform_3d.h"
#include "core/math/vector2i.h"
#include "core/string/string_name.h"

/**
 * @brief Runtime data for a single placed transition instance.
 *
 * Can be serialized or inspected by gameplay systems to compute
 * movement costs, state effects, etc.
 */
class HexTransitionInstanceData : public Resource {
	GDCLASS(HexTransitionInstanceData, Resource)

	Vector2i source_coord;
	int direction = 0;
	Ref<HexTransitionProfile> profile;
	Transform3D transform;
	uint32_t gameplay_flags = 0;
	StringName gameplay_state_id;

protected:
	static void _bind_methods();

public:
	void set_source_coord(const Vector2i &p_coord);
	Vector2i get_source_coord() const;

	void set_direction(int p_direction);
	int get_direction() const;

	void set_profile(const Ref<HexTransitionProfile> &p_profile);
	Ref<HexTransitionProfile> get_profile() const;

	void set_transform(const Transform3D &p_transform);
	Transform3D get_transform() const;

	void set_gameplay_flags(uint32_t p_flags);
	uint32_t get_gameplay_flags() const;

	void set_gameplay_state_id(const StringName &p_id);
	StringName get_gameplay_state_id() const;
};

#endif // HEX_TRANSITION_INSTANCE_DATA_H
