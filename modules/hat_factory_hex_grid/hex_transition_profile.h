#ifndef HEX_TRANSITION_PROFILE_H
#define HEX_TRANSITION_PROFILE_H

#include "core/io/resource.h"
#include "core/math/vector3.h"
#include "core/string/string_name.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"

/**
 * @brief Visual profile for a hex transition instance.
 *
 * Describes which mesh/material to use and how to scale/offset it
 * when placed between two adjacent cells.
 */
class HexTransitionProfile : public Resource {
	GDCLASS(HexTransitionProfile, Resource)

	StringName profile_id;
	Ref<Mesh> mesh;
	Ref<Material> material;
	Vector3 mesh_offset;
	Vector3 mesh_scale = Vector3(1, 1, 1);
	float height_scale = 1.0f;
	bool cast_shadows = true;

protected:
	static void _bind_methods();

public:
	void set_profile_id(const StringName &p_id);
	StringName get_profile_id() const;

	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	void set_mesh_offset(const Vector3 &p_offset);
	Vector3 get_mesh_offset() const;

	void set_mesh_scale(const Vector3 &p_scale);
	Vector3 get_mesh_scale() const;

	void set_height_scale(float p_scale);
	float get_height_scale() const;

	void set_cast_shadows(bool p_enabled);
	bool get_cast_shadows() const;
};

#endif // HEX_TRANSITION_PROFILE_H
