#ifndef HEX_TERRAIN_DEF_H
#define HEX_TERRAIN_DEF_H

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/typed_array.h"
#include "scene/resources/3d/shape_3d.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"

/**
 * @brief Definition of a single terrain type used by HexGridMap.
 *
 * This is the data-driven "material" of a cell: which mesh to draw,
 * which physics shape to use, and which gameplay flags it carries.
 *
 * Extensions for the concept-to-hexmap pipeline:
 *  - variant_meshes / variant_materials: per-cell variant overrides.
 *  - fill_mesh / fill_material / fill_step: stacked height visualization.
 */
class HexTerrainDef : public Resource {
	GDCLASS(HexTerrainDef, Resource)

	StringName terrain_id;
	Ref<Mesh> mesh;
	Ref<Material> material;
	Ref<Shape3D> physics_shape;
	uint32_t flags = 0;
	bool cast_shadows = true;

	TypedArray<Mesh> variant_meshes;
	TypedArray<Material> variant_materials;

	Ref<Mesh> fill_mesh;
	Ref<Material> fill_material;
	int fill_step = 1;

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

	void set_variant_meshes(const TypedArray<Mesh> &p_meshes);
	TypedArray<Mesh> get_variant_meshes() const;

	void set_variant_materials(const TypedArray<Material> &p_materials);
	TypedArray<Material> get_variant_materials() const;

	void set_fill_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_fill_mesh() const;

	void set_fill_material(const Ref<Material> &p_material);
	Ref<Material> get_fill_material() const;

	void set_fill_step(int p_step);
	int get_fill_step() const;
};

#endif // HEX_TERRAIN_DEF_H
