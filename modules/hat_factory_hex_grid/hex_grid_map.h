#ifndef HEX_GRID_MAP_H
#define HEX_GRID_MAP_H

#include "hex_grid_map_data.h"
#include "hex_terrain_library.h"
#include "scene/3d/node_3d.h"
#include "core/math/aabb.h"
#include "core/math/vector2i.h"
#include "core/templates/rid.h"
#include "core/templates/hash_map.h"
#include "core/templates/hash_set.h"
#include "core/templates/local_vector.h"
#include "core/variant/typed_array.h"

/**
 * @brief 3D data-driven hexagonal grid map node.
 *
 * Heavily inspired by Godot's GridMap module:
 *  - cells are grouped into chunks (octants);
 *  - each chunk renders cells of the same terrain via a MultiMesh;
 *  - each chunk owns a single static physics body;
 *  - dirty chunks are rebuilt in a single deferred callback.
 *
 * Unlike GridMap, this node uses axial (q,r) hex coordinates and stores
 * its authoritative data inside a HexGridMapData resource.
 */
class HexGridMap : public Node3D {
	GDCLASS(HexGridMap, Node3D)

public:
	enum Orientation {
		ORIENTATION_POINTY_TOP,
		ORIENTATION_FLAT_TOP
	};

private:
	struct Chunk {
		bool dirty = false;
		RID static_body;
		HashSet<Vector2i> cells;

		struct MultimeshInstance {
			RID instance;
			RID multimesh;
			StringName terrain_id;
			int variant = 0;
			bool fill = false;
		};
		Vector<MultimeshInstance> multimesh_instances;
	};

	Ref<HexGridMapData> grid_data;
	Ref<HexTerrainLibrary> terrain_library;

	float hex_size = 2.0f;
	float cell_height_step = 1.0f;
	int chunk_size = 8;
	Orientation orientation = ORIENTATION_POINTY_TOP;

	uint32_t collision_layer = 1;
	uint32_t collision_mask = 1;
	real_t collision_priority = 1.0;

	bool center_grid = true;
	bool awaiting_update = false;
	bool _in_tree = false;
	Transform3D last_transform;
	Vector3 center_offset;

	Vector3 _map_to_local_no_center(const Vector2i &p_coord, int p_height) const;
	Vector3 _local_to_world_no_center(const Vector3 &p_local) const;

	HashMap<Vector2i, Ref<HexCellData>> cell_map;
	HashMap<Vector2i, Chunk *> chunk_map;
	HashSet<Vector2i> dirty_chunks;

	void _clear_internal();
	void _recreate_data();

	void _ensure_chunk(const Vector2i &p_chunk_key);
	void _cleanup_chunk(const Vector2i &p_chunk_key);

	void _add_cell_to_chunk(const Vector2i &p_cell);
	void _remove_cell_from_chunk(const Vector2i &p_cell);
	Vector2i _cell_to_chunk(const Vector2i &p_cell) const;

	void _queue_chunk_dirty(const Vector2i &p_chunk_key);
	void _queue_chunks_dirty();
	void _update_chunks_callback();
	bool _update_chunk(const Vector2i &p_chunk_key);

	void _chunk_enter_world(const Vector2i &p_chunk_key);
	void _chunk_exit_world(const Vector2i &p_chunk_key);
	void _update_chunk_transform(const Vector2i &p_chunk_key);

	void _grid_data_changed();
	void _terrain_library_changed();
	void _update_physics_bodies_collision_properties();

	static Vector2i _cube_round(float p_q, float p_r);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_grid_data(const Ref<HexGridMapData> &p_data);
	Ref<HexGridMapData> get_grid_data() const;

	void set_terrain_library(const Ref<HexTerrainLibrary> &p_lib);
	Ref<HexTerrainLibrary> get_terrain_library() const;

	void set_hex_size(float p_size);
	float get_hex_size() const;

	void set_cell_height_step(float p_step);
	float get_cell_height_step() const;

	void set_chunk_size(int p_size);
	int get_chunk_size() const;

	void set_orientation(Orientation p_orientation);
	Orientation get_orientation() const;

	void set_collision_layer(uint32_t p_layer);
	uint32_t get_collision_layer() const;

	void set_collision_mask(uint32_t p_mask);
	uint32_t get_collision_mask() const;

	void set_collision_priority(real_t p_priority);
	real_t get_collision_priority() const;

	void set_center_grid(bool p_center);
	bool get_center_grid() const;

	void set_cell(const Vector2i &p_coord, const StringName &p_terrain_id, int p_height = 0, uint32_t p_flags = 0);
	void set_cell_data(const Ref<HexCellData> &p_cell);
	void erase_cell(const Vector2i &p_coord);
	Ref<HexCellData> get_cell(const Vector2i &p_coord) const;
	bool has_cell(const Vector2i &p_coord) const;
	TypedArray<Vector2i> get_used_cells() const;
	void clear();

	Vector3 map_to_local(const Vector2i &p_coord, int p_height = 0) const;
	Vector2i local_to_map(const Vector3 &p_local) const;
	Vector2i get_neighbor(const Vector2i &p_coord, int p_direction) const;
	TypedArray<Vector2i> get_all_neighbors(const Vector2i &p_coord) const;
	TypedArray<Vector2i> get_cells_in_ring(const Vector2i &p_center, int p_radius) const;
	TypedArray<Vector2i> get_cells_in_disk(const Vector2i &p_center, int p_radius) const;
	TypedArray<Vector2i> get_cells_in_range(const Vector2i &p_center, int p_radius) const;

	bool has_terrain(const Vector2i &p_coord) const;
	AABB get_cell_world_bounds(const Vector2i &p_coord) const;

	void set_cells_bulk(const TypedArray<HexCellData> &p_cells);
	void erase_cells_bulk(const TypedArray<Vector2i> &p_coords);

	void refresh_cell(const Vector2i &p_coord);
	void refresh();

	HexGridMap();
	~HexGridMap();
};

VARIANT_ENUM_CAST(HexGridMap::Orientation)

#endif // HEX_GRID_MAP_H
