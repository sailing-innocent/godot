#include "hex_grid_map.h"

#include "core/core_string_names.h"
#include "core/math/math_funcs.h"
#include "core/object/callable_mp.h"
#include "core/object/class_db.h"
#include "core/os/memory.h"
#include "scene/main/scene_tree.h"
#include "servers/rendering/rendering_server.h"
#include "servers/rendering/rendering_server_enums.h"

#ifndef PHYSICS_3D_DISABLED
#include "servers/physics_3d/physics_server_3d.h"
#endif

static const Vector2i HEX_NEIGHBORS_POINTY[6] = {
	Vector2i(1, 0),
	Vector2i(1, -1),
	Vector2i(0, -1),
	Vector2i(-1, 0),
	Vector2i(-1, 1),
	Vector2i(0, 1)
};

static const Vector2i HEX_NEIGHBORS_FLAT[6] = {
	Vector2i(1, 0),
	Vector2i(1, -1),
	Vector2i(0, -1),
	Vector2i(-1, 0),
	Vector2i(-1, 1),
	Vector2i(0, 1)
};

Vector2i HexGridMap::_cube_round(float p_q, float p_r) {
	float s = -p_q - p_r;
	int rq = (int)Math::round(p_q);
	int rr = (int)Math::round(p_r);
	int rs = (int)Math::round(s);

	float dq = Math::abs((float)rq - p_q);
	float dr = Math::abs((float)rr - p_r);
	float ds = Math::abs((float)rs - s);

	if (dq > dr && dq > ds) {
		rq = -rr - rs;
	} else if (dr > ds) {
		rr = -rq - rs;
	}
	return Vector2i(rq, rr);
}

void HexGridMap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_grid_data", "grid_data"), &HexGridMap::set_grid_data);
	ClassDB::bind_method(D_METHOD("get_grid_data"), &HexGridMap::get_grid_data);

	ClassDB::bind_method(D_METHOD("set_terrain_library", "terrain_library"), &HexGridMap::set_terrain_library);
	ClassDB::bind_method(D_METHOD("get_terrain_library"), &HexGridMap::get_terrain_library);

	ClassDB::bind_method(D_METHOD("set_hex_size", "hex_size"), &HexGridMap::set_hex_size);
	ClassDB::bind_method(D_METHOD("get_hex_size"), &HexGridMap::get_hex_size);

	ClassDB::bind_method(D_METHOD("set_cell_height_step", "cell_height_step"), &HexGridMap::set_cell_height_step);
	ClassDB::bind_method(D_METHOD("get_cell_height_step"), &HexGridMap::get_cell_height_step);

	ClassDB::bind_method(D_METHOD("set_chunk_size", "chunk_size"), &HexGridMap::set_chunk_size);
	ClassDB::bind_method(D_METHOD("get_chunk_size"), &HexGridMap::get_chunk_size);

	ClassDB::bind_method(D_METHOD("set_orientation", "orientation"), &HexGridMap::set_orientation);
	ClassDB::bind_method(D_METHOD("get_orientation"), &HexGridMap::get_orientation);

	ClassDB::bind_method(D_METHOD("set_collision_layer", "collision_layer"), &HexGridMap::set_collision_layer);
	ClassDB::bind_method(D_METHOD("get_collision_layer"), &HexGridMap::get_collision_layer);

	ClassDB::bind_method(D_METHOD("set_collision_mask", "collision_mask"), &HexGridMap::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &HexGridMap::get_collision_mask);

	ClassDB::bind_method(D_METHOD("set_collision_priority", "collision_priority"), &HexGridMap::set_collision_priority);
	ClassDB::bind_method(D_METHOD("get_collision_priority"), &HexGridMap::get_collision_priority);

	ClassDB::bind_method(D_METHOD("set_center_grid", "center_grid"), &HexGridMap::set_center_grid);
	ClassDB::bind_method(D_METHOD("get_center_grid"), &HexGridMap::get_center_grid);

	ClassDB::bind_method(D_METHOD("set_cell", "coord", "terrain_id", "height", "flags"), &HexGridMap::set_cell, DEFVAL(0), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("set_cell_data", "cell"), &HexGridMap::set_cell_data);
	ClassDB::bind_method(D_METHOD("erase_cell", "coord"), &HexGridMap::erase_cell);
	ClassDB::bind_method(D_METHOD("get_cell", "coord"), &HexGridMap::get_cell);
	ClassDB::bind_method(D_METHOD("has_cell", "coord"), &HexGridMap::has_cell);
	ClassDB::bind_method(D_METHOD("get_used_cells"), &HexGridMap::get_used_cells);
	ClassDB::bind_method(D_METHOD("clear"), &HexGridMap::clear);

	ClassDB::bind_method(D_METHOD("map_to_local", "coord", "height"), &HexGridMap::map_to_local, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("local_to_map", "local_position"), &HexGridMap::local_to_map);
	ClassDB::bind_method(D_METHOD("get_neighbor", "coord", "direction"), &HexGridMap::get_neighbor);
	ClassDB::bind_method(D_METHOD("get_cells_in_range", "center", "radius"), &HexGridMap::get_cells_in_range);

	ClassDB::bind_method(D_METHOD("refresh_cell", "coord"), &HexGridMap::refresh_cell);
	ClassDB::bind_method(D_METHOD("refresh"), &HexGridMap::refresh);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "grid_data", PROPERTY_HINT_RESOURCE_TYPE, "HexGridMapData"), "set_grid_data", "get_grid_data");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "terrain_library", PROPERTY_HINT_RESOURCE_TYPE, "HexTerrainLibrary"), "set_terrain_library", "get_terrain_library");

	ADD_GROUP("Hex", "hex_");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "hex_size"), "set_hex_size", "get_hex_size");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cell_height_step"), "set_cell_height_step", "get_cell_height_step");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "chunk_size"), "set_chunk_size", "get_chunk_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "orientation", PROPERTY_HINT_ENUM, "Pointy Top,Flat Top"), "set_orientation", "get_orientation");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "center_grid"), "set_center_grid", "get_center_grid");

	ADD_GROUP("Collision", "collision_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_layer", "get_collision_layer");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "collision_priority"), "set_collision_priority", "get_collision_priority");

	ADD_SIGNAL(MethodInfo("cell_changed", PropertyInfo(Variant::VECTOR2I, "coord")));

	BIND_ENUM_CONSTANT(ORIENTATION_POINTY_TOP);
	BIND_ENUM_CONSTANT(ORIENTATION_FLAT_TOP);
}

void HexGridMap::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_in_tree = true;
			last_transform = get_global_transform();
			for (KeyValue<Vector2i, Chunk *> &E : chunk_map) {
				_chunk_enter_world(E.key);
			}
			_queue_chunks_dirty();
		} break;

		case NOTIFICATION_EXIT_TREE: {
			_in_tree = false;
			for (KeyValue<Vector2i, Chunk *> &E : chunk_map) {
				_chunk_exit_world(E.key);
			}
		} break;

		case NOTIFICATION_TRANSFORM_CHANGED: {
			if (_in_tree) {
				Transform3D gt = get_global_transform();
				if (gt != last_transform) {
					for (KeyValue<Vector2i, Chunk *> &E : chunk_map) {
						_update_chunk_transform(E.key);
					}
					last_transform = gt;
				}
			}
		} break;
	}
}

void HexGridMap::_clear_internal() {
	for (KeyValue<Vector2i, Chunk *> &E : chunk_map) {
		_cleanup_chunk(E.key);
	}
	chunk_map.clear();
	cell_map.clear();
	dirty_chunks.clear();
	awaiting_update = false;
}

void HexGridMap::_recreate_data() {
	_clear_internal();

	if (grid_data.is_valid()) {
		TypedArray<Vector2i> used = grid_data->get_used_cells();
		for (int i = 0; i < used.size(); i++) {
			Vector2i coord = used[i];
			Ref<HexCellData> cell = grid_data->get_cell(coord);
				if (cell.is_valid()) {
					cell_map[coord] = cell;
					_add_cell_to_chunk(coord);
				}
			}
		}

		if (center_grid && !cell_map.is_empty()) {
			Vector3 avg;
			for (const KeyValue<Vector2i, Ref<HexCellData>> &E : cell_map) {
				avg += _map_to_local_no_center(E.key, 0);
			}
			center_offset = avg / (float)cell_map.size();
		} else {
			center_offset = Vector3();
		}

		_queue_chunks_dirty();
	}

Vector2i HexGridMap::_cell_to_chunk(const Vector2i &p_cell) const {
	int q = (int)Math::floor((double)p_cell.x / (double)chunk_size);
	int r = (int)Math::floor((double)p_cell.y / (double)chunk_size);
	return Vector2i(q, r);
}

void HexGridMap::_ensure_chunk(const Vector2i &p_key) {
	if (chunk_map.has(p_key)) {
		return;
	}

	Chunk *chunk = memnew(Chunk);
	chunk->dirty = true;

#ifndef PHYSICS_3D_DISABLED
	chunk->static_body = PhysicsServer3D::get_singleton()->body_create();
	PhysicsServer3D::get_singleton()->body_set_mode(chunk->static_body, PhysicsServer3D::BODY_MODE_STATIC);
	PhysicsServer3D::get_singleton()->body_attach_object_instance_id(chunk->static_body, get_instance_id());
#endif

	chunk_map[p_key] = chunk;

	if (_in_tree) {
		_chunk_enter_world(p_key);
	}

	_update_physics_bodies_collision_properties();
}

void HexGridMap::_cleanup_chunk(const Vector2i &p_key) {
	if (!chunk_map.has(p_key)) {
		return;
	}

	Chunk *chunk = chunk_map[p_key];

	if (_in_tree) {
		_chunk_exit_world(p_key);
	}

	for (int i = 0; i < chunk->multimesh_instances.size(); i++) {
		const Chunk::MultimeshInstance &mmi = chunk->multimesh_instances[i];
		if (mmi.instance.is_valid()) {
			RS::get_singleton()->free_rid(mmi.instance);
		}
		if (mmi.multimesh.is_valid()) {
			RS::get_singleton()->free_rid(mmi.multimesh);
		}
	}
	chunk->multimesh_instances.clear();

#ifndef PHYSICS_3D_DISABLED
	if (chunk->static_body.is_valid()) {
		PhysicsServer3D::get_singleton()->free_rid(chunk->static_body);
	}
#endif

	memdelete(chunk);
	chunk_map.erase(p_key);
}

void HexGridMap::_add_cell_to_chunk(const Vector2i &p_cell) {
	Vector2i ck = _cell_to_chunk(p_cell);
	_ensure_chunk(ck);
	Chunk *chunk = chunk_map[ck];
	chunk->cells.insert(p_cell);
}

void HexGridMap::_remove_cell_from_chunk(const Vector2i &p_cell) {
	Vector2i ck = _cell_to_chunk(p_cell);
	if (!chunk_map.has(ck)) {
		return;
	}
	Chunk *chunk = chunk_map[ck];
	chunk->cells.erase(p_cell);
	chunk->dirty = true;
}

void HexGridMap::_queue_chunk_dirty(const Vector2i &p_chunk_key) {
	if (chunk_map.has(p_chunk_key)) {
		chunk_map[p_chunk_key]->dirty = true;
	}
	dirty_chunks.insert(p_chunk_key);

	if (!awaiting_update) {
		callable_mp(this, &HexGridMap::_update_chunks_callback).call_deferred();
		awaiting_update = true;
	}
}

void HexGridMap::_queue_chunks_dirty() {
	for (KeyValue<Vector2i, Chunk *> &E : chunk_map) {
		E.value->dirty = true;
		dirty_chunks.insert(E.key);
	}
	if (!awaiting_update) {
		callable_mp(this, &HexGridMap::_update_chunks_callback).call_deferred();
		awaiting_update = true;
	}
}

void HexGridMap::_update_chunks_callback() {
	if (!awaiting_update) {
		return;
	}

	LocalVector<Vector2i> to_remove;
	for (KeyValue<Vector2i, Chunk *> &E : chunk_map) {
		if (E.value->dirty) {
			bool empty = _update_chunk(E.key);
			if (empty) {
				to_remove.push_back(E.key);
			}
		}
	}

	for (int i = 0; i < to_remove.size(); i++) {
		_cleanup_chunk(to_remove[i]);
	}

	dirty_chunks.clear();
	awaiting_update = false;
}

bool HexGridMap::_update_chunk(const Vector2i &p_key) {
	if (!chunk_map.has(p_key)) {
		return false;
	}

	Chunk *chunk = chunk_map[p_key];
	if (!chunk->dirty) {
		return false;
	}

#ifndef PHYSICS_3D_DISABLED
	PhysicsServer3D::get_singleton()->body_clear_shapes(chunk->static_body);
#endif

	for (int i = 0; i < chunk->multimesh_instances.size(); i++) {
		const Chunk::MultimeshInstance &mmi = chunk->multimesh_instances[i];
		if (mmi.instance.is_valid()) {
			RS::get_singleton()->free_rid(mmi.instance);
		}
		if (mmi.multimesh.is_valid()) {
			RS::get_singleton()->free_rid(mmi.multimesh);
		}
	}
	chunk->multimesh_instances.clear();

	if (chunk->cells.is_empty()) {
		chunk->dirty = false;
		return true;
	}

	HashMap<StringName, LocalVector<Transform3D>> groups;

	for (const Vector2i &cell_coord : chunk->cells) {
		ERR_CONTINUE(!cell_map.has(cell_coord));
		Ref<HexCellData> cell = cell_map[cell_coord];
		StringName terrain_id = cell->get_terrain_id();

		if (terrain_library.is_null() || !terrain_library->has_terrain(terrain_id)) {
			continue;
		}

		Ref<HexTerrainDef> def = terrain_library->get_terrain(terrain_id);
		if (def.is_null() || def->get_mesh().is_null()) {
			continue;
		}

		Transform3D xform(Basis(), map_to_local(cell_coord, cell->get_height()));

		if (!groups.has(terrain_id)) {
			groups[terrain_id] = LocalVector<Transform3D>();
		}
		groups[terrain_id].push_back(xform);

#ifndef PHYSICS_3D_DISABLED
		if (def->get_physics_shape().is_valid()) {
			PhysicsServer3D::get_singleton()->body_add_shape(chunk->static_body, def->get_physics_shape()->get_rid(), xform);
		}
#endif
	}

	RID scenario;
	if (_in_tree && is_inside_world()) {
		scenario = get_world_3d()->get_scenario();
	}

	for (KeyValue<StringName, LocalVector<Transform3D>> &E : groups) {
		Ref<HexTerrainDef> def = terrain_library->get_terrain(E.key);
		if (def.is_null() || def->get_mesh().is_null()) {
			continue;
		}

		int instance_count = E.value.size();
		if (instance_count == 0) {
			continue;
		}

		RID mm = RS::get_singleton()->multimesh_create();
		RS::get_singleton()->multimesh_allocate_data(mm, instance_count, RSE::MULTIMESH_TRANSFORM_3D);
		RS::get_singleton()->multimesh_set_mesh(mm, def->get_mesh()->get_rid());

		for (int i = 0; i < instance_count; i++) {
			RS::get_singleton()->multimesh_instance_set_transform(mm, i, E.value[i]);
		}

		RID inst = RS::get_singleton()->instance_create();
		RS::get_singleton()->instance_set_base(inst, mm);

		if (scenario.is_valid()) {
			RS::get_singleton()->instance_set_scenario(inst, scenario);
			RS::get_singleton()->instance_set_transform(inst, get_global_transform());
		}

		if (def->get_material().is_valid()) {
			RS::get_singleton()->instance_geometry_set_material_override(inst, def->get_material()->get_rid());
		}

		RSE::ShadowCastingSetting shadow = def->get_cast_shadows() ? RSE::SHADOW_CASTING_SETTING_ON : RSE::SHADOW_CASTING_SETTING_OFF;
		RS::get_singleton()->instance_geometry_set_cast_shadows_setting(inst, shadow);

		Chunk::MultimeshInstance mmi;
		mmi.instance = inst;
		mmi.multimesh = mm;
		mmi.terrain_id = E.key;
		chunk->multimesh_instances.push_back(mmi);
	}

	chunk->dirty = false;
	return false;
}

void HexGridMap::_chunk_enter_world(const Vector2i &p_key) {
	if (!chunk_map.has(p_key)) {
		return;
	}
	Chunk *chunk = chunk_map[p_key];
	RID scenario = get_world_3d()->get_scenario();
	Transform3D gt = get_global_transform();

	for (int i = 0; i < chunk->multimesh_instances.size(); i++) {
		const Chunk::MultimeshInstance &mmi = chunk->multimesh_instances[i];
		RS::get_singleton()->instance_set_scenario(mmi.instance, scenario);
		RS::get_singleton()->instance_set_transform(mmi.instance, gt);
	}

#ifndef PHYSICS_3D_DISABLED
	PhysicsServer3D::get_singleton()->body_set_state(chunk->static_body, PhysicsServer3D::BODY_STATE_TRANSFORM, gt);
	PhysicsServer3D::get_singleton()->body_set_space(chunk->static_body, get_world_3d()->get_space());
#endif
}

void HexGridMap::_chunk_exit_world(const Vector2i &p_key) {
	if (!chunk_map.has(p_key)) {
		return;
	}
	Chunk *chunk = chunk_map[p_key];

	for (int i = 0; i < chunk->multimesh_instances.size(); i++) {
		const Chunk::MultimeshInstance &mmi = chunk->multimesh_instances[i];
		RS::get_singleton()->instance_set_scenario(mmi.instance, RID());
	}

#ifndef PHYSICS_3D_DISABLED
	PhysicsServer3D::get_singleton()->body_set_space(chunk->static_body, RID());
#endif
}

void HexGridMap::_update_chunk_transform(const Vector2i &p_key) {
	if (!chunk_map.has(p_key)) {
		return;
	}
	Chunk *chunk = chunk_map[p_key];
	Transform3D gt = get_global_transform();

	for (int i = 0; i < chunk->multimesh_instances.size(); i++) {
		const Chunk::MultimeshInstance &mmi = chunk->multimesh_instances[i];
		RS::get_singleton()->instance_set_transform(mmi.instance, gt);
	}

#ifndef PHYSICS_3D_DISABLED
	PhysicsServer3D::get_singleton()->body_set_state(chunk->static_body, PhysicsServer3D::BODY_STATE_TRANSFORM, gt);
	if (_in_tree) {
		PhysicsServer3D::get_singleton()->body_set_space(chunk->static_body, get_world_3d()->get_space());
	}
#endif
}

void HexGridMap::_grid_data_changed() {
	_recreate_data();
	emit_signal(CoreStringName(changed));
}

void HexGridMap::_terrain_library_changed() {
	_queue_chunks_dirty();
	emit_signal(CoreStringName(changed));
}

void HexGridMap::_update_physics_bodies_collision_properties() {
#ifndef PHYSICS_3D_DISABLED
	for (KeyValue<Vector2i, Chunk *> &E : chunk_map) {
		PhysicsServer3D::get_singleton()->body_set_collision_layer(E.value->static_body, collision_layer);
		PhysicsServer3D::get_singleton()->body_set_collision_mask(E.value->static_body, collision_mask);
		PhysicsServer3D::get_singleton()->body_set_collision_priority(E.value->static_body, collision_priority);
	}
#endif
}

void HexGridMap::set_grid_data(const Ref<HexGridMapData> &p_data) {
	if (grid_data == p_data) {
		return;
	}
	if (grid_data.is_valid()) {
		grid_data->disconnect_changed(callable_mp(this, &HexGridMap::_grid_data_changed));
	}
	grid_data = p_data;
	if (grid_data.is_valid()) {
		grid_data->connect_changed(callable_mp(this, &HexGridMap::_grid_data_changed));
	}
	_recreate_data();
	emit_signal(CoreStringName(changed));
}

Ref<HexGridMapData> HexGridMap::get_grid_data() const {
	return grid_data;
}

void HexGridMap::set_terrain_library(const Ref<HexTerrainLibrary> &p_lib) {
	if (terrain_library == p_lib) {
		return;
	}
	if (terrain_library.is_valid()) {
		terrain_library->disconnect_changed(callable_mp(this, &HexGridMap::_terrain_library_changed));
	}
	terrain_library = p_lib;
	if (terrain_library.is_valid()) {
		terrain_library->connect_changed(callable_mp(this, &HexGridMap::_terrain_library_changed));
	}
	_queue_chunks_dirty();
	emit_signal(CoreStringName(changed));
}

Ref<HexTerrainLibrary> HexGridMap::get_terrain_library() const {
	return terrain_library;
}

void HexGridMap::set_hex_size(float p_size) {
	if (p_size < 0.001f) {
		p_size = 0.001f;
	}
	if (hex_size == p_size) {
		return;
	}
	hex_size = p_size;
	_recreate_data();
	emit_signal(CoreStringName(changed));
}

float HexGridMap::get_hex_size() const {
	return hex_size;
}

void HexGridMap::set_cell_height_step(float p_step) {
	if (cell_height_step == p_step) {
		return;
	}
	cell_height_step = p_step;
	_recreate_data();
	emit_signal(CoreStringName(changed));
}

float HexGridMap::get_cell_height_step() const {
	return cell_height_step;
}

void HexGridMap::set_chunk_size(int p_size) {
	if (p_size < 1) {
		p_size = 1;
	}
	if (chunk_size == p_size) {
		return;
	}
	chunk_size = p_size;
	_recreate_data();
	emit_signal(CoreStringName(changed));
}

int HexGridMap::get_chunk_size() const {
	return chunk_size;
}

void HexGridMap::set_orientation(Orientation p_orientation) {
	if (orientation == p_orientation) {
		return;
	}
	orientation = p_orientation;
	_recreate_data();
	emit_signal(CoreStringName(changed));
}

HexGridMap::Orientation HexGridMap::get_orientation() const {
	return orientation;
}

void HexGridMap::set_collision_layer(uint32_t p_layer) {
	if (collision_layer == p_layer) {
		return;
	}
	collision_layer = p_layer;
	_update_physics_bodies_collision_properties();
	emit_signal(CoreStringName(changed));
}

uint32_t HexGridMap::get_collision_layer() const {
	return collision_layer;
}

void HexGridMap::set_collision_mask(uint32_t p_mask) {
	if (collision_mask == p_mask) {
		return;
	}
	collision_mask = p_mask;
	_update_physics_bodies_collision_properties();
	emit_signal(CoreStringName(changed));
}

uint32_t HexGridMap::get_collision_mask() const {
	return collision_mask;
}

void HexGridMap::set_collision_priority(real_t p_priority) {
	if (collision_priority == p_priority) {
		return;
	}
	collision_priority = p_priority;
	_update_physics_bodies_collision_properties();
	emit_signal(CoreStringName(changed));
}

real_t HexGridMap::get_collision_priority() const {
	return collision_priority;
}

void HexGridMap::set_center_grid(bool p_center) {
	if (center_grid == p_center) {
		return;
	}
	center_grid = p_center;
	_recreate_data();
	emit_signal(CoreStringName(changed));
}

bool HexGridMap::get_center_grid() const {
	return center_grid;
}

void HexGridMap::set_cell(const Vector2i &p_coord, const StringName &p_terrain_id, int p_height, uint32_t p_flags) {
	Ref<HexCellData> cell;
	if (cell_map.has(p_coord)) {
		cell = cell_map[p_coord];
	} else {
		cell.instantiate();
		cell->set_coord(p_coord);
	}
	cell->set_terrain_id(p_terrain_id);
	cell->set_height(p_height);
	cell->set_flags(p_flags);
	set_cell_data(cell);
}

void HexGridMap::set_cell_data(const Ref<HexCellData> &p_cell) {
	ERR_FAIL_COND(p_cell.is_null());
	Vector2i coord = p_cell->get_coord();

	if (cell_map.has(coord)) {
		_remove_cell_from_chunk(coord);
	}
	cell_map[coord] = p_cell;
	_add_cell_to_chunk(coord);

	Vector2i ck = _cell_to_chunk(coord);
	_queue_chunk_dirty(ck);
	emit_signal(SNAME("cell_changed"), coord);
}

void HexGridMap::erase_cell(const Vector2i &p_coord) {
	if (!cell_map.has(p_coord)) {
		return;
	}
	_remove_cell_from_chunk(p_coord);
	cell_map.erase(p_coord);

	Vector2i ck = _cell_to_chunk(p_coord);
	_queue_chunk_dirty(ck);
	emit_signal(SNAME("cell_changed"), p_coord);
}

Ref<HexCellData> HexGridMap::get_cell(const Vector2i &p_coord) const {
	if (!cell_map.has(p_coord)) {
		return Ref<HexCellData>();
	}
	return cell_map[p_coord];
}

bool HexGridMap::has_cell(const Vector2i &p_coord) const {
	return cell_map.has(p_coord);
}

TypedArray<Vector2i> HexGridMap::get_used_cells() const {
	TypedArray<Vector2i> used;
	for (KeyValue<Vector2i, Ref<HexCellData>> E : cell_map) {
		used.push_back(E.key);
	}
	return used;
}

void HexGridMap::clear() {
	_clear_internal();
	emit_signal(CoreStringName(changed));
}

Vector3 HexGridMap::_map_to_local_no_center(const Vector2i &p_coord, int p_height) const {
	float x = 0.0f;
	float z = 0.0f;
	float s = hex_size;
	float sqrt3 = Math::sqrt(3.0);

	if (orientation == ORIENTATION_POINTY_TOP) {
		x = s * (sqrt3 * p_coord.x + sqrt3 * 0.5f * p_coord.y);
		z = s * (1.5f * p_coord.y);
	} else {
		x = s * (1.5f * p_coord.x);
		z = s * (sqrt3 * 0.5f * p_coord.x + sqrt3 * p_coord.y);
	}

	float y = (float)p_height * cell_height_step;
	return Vector3(x, y, z);
}

Vector3 HexGridMap::map_to_local(const Vector2i &p_coord, int p_height) const {
	return _map_to_local_no_center(p_coord, p_height) - center_offset;
}

Vector3 HexGridMap::_local_to_world_no_center(const Vector3 &p_local) const {
	return p_local + center_offset;
}

Vector2i HexGridMap::local_to_map(const Vector3 &p_local) const {
	Vector3 world = _local_to_world_no_center(p_local);
	float x = world.x / hex_size;
	float z = world.z / hex_size;
	float q = 0.0f;
	float r = 0.0f;
	float sqrt3 = Math::sqrt(3.0);

	if (orientation == ORIENTATION_POINTY_TOP) {
		q = (sqrt3 / 3.0f * x - 1.0f / 3.0f * z);
		r = (2.0f / 3.0f * z);
	} else {
		q = (2.0f / 3.0f * x);
		r = (-1.0f / 3.0f * x + sqrt3 / 3.0f * z);
	}

	return _cube_round(q, r);
}

Vector2i HexGridMap::get_neighbor(const Vector2i &p_coord, int p_direction) const {
	p_direction = Math::posmod(p_direction, 6);
	if (orientation == ORIENTATION_POINTY_TOP) {
		return p_coord + HEX_NEIGHBORS_POINTY[p_direction];
	} else {
		return p_coord + HEX_NEIGHBORS_FLAT[p_direction];
	}
}

TypedArray<Vector2i> HexGridMap::get_cells_in_range(const Vector2i &p_center, int p_radius) const {
	TypedArray<Vector2i> result;
	for (int q = -p_radius; q <= p_radius; q++) {
		int r1 = (-p_radius > -q - p_radius) ? -p_radius : -q - p_radius;
		int r2 = (p_radius < -q + p_radius) ? p_radius : -q + p_radius;
		for (int r = r1; r <= r2; r++) {
			result.push_back(p_center + Vector2i(q, r));
		}
	}
	return result;
}

void HexGridMap::refresh_cell(const Vector2i &p_coord) {
	if (grid_data.is_valid() && grid_data->has_cell(p_coord)) {
		Ref<HexCellData> data_cell = grid_data->get_cell(p_coord);
		if (data_cell.is_valid()) {
			if (cell_map.has(p_coord)) {
				_remove_cell_from_chunk(p_coord);
			}
			cell_map[p_coord] = data_cell;
			_add_cell_to_chunk(p_coord);
			_queue_chunk_dirty(_cell_to_chunk(p_coord));
			emit_signal(SNAME("cell_changed"), p_coord);
			return;
		}
	}

	if (cell_map.has(p_coord)) {
		erase_cell(p_coord);
	}
}

void HexGridMap::refresh() {
	_queue_chunks_dirty();
}

HexGridMap::HexGridMap() {
	set_notify_transform(true);
}

HexGridMap::~HexGridMap() {
	_clear_internal();
}
