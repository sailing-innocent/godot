#ifndef HEX_TERRAIN_DEF_H
#define HEX_TERRAIN_DEF_H

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "scene/resources/3d/shape_3d.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"

/**
 * @brief Definition of a single terrain type used by HexGridMap.
 *
 * This is the data-driven "material" of a cell: which mesh to draw,
 * which physics shape to use, and which gameplay flags it carries.
 */
class HexTerrainDef : public Resource {
	GDCLASS(HexTerrainDef, Resource)

	StringName terrain_id;
	Ref<Mesh> mesh;
	Ref<Material> material;
	Ref<Shape3D> physics_shape;
	uint32_t flags = 0;
	bool cast_shadows = true;

protected:
	static void _bind_methods();

public:
	void set_terrain_id(const StringName &p_id);
	StringName get_terrain_id() const;

	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh() const;

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

	void set_physics_shape(const Ref<Shape3D> &p_shape);
	Ref<Shape3D> get_physics_shape() const;

	void set_flags(uint32_t p_flags);
	uint32_t get_flags() const;

	void set_cast_shadows(bool p_enabled);
	bool get_cast_shadows() const;
};

#endif // HEX_TERRAIN_DEF_H
