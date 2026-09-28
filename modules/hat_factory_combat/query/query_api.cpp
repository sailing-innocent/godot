#include "query_api.h"

#include "core/math/math_funcs.h"
#include "core/templates/local_vector.h"
#include "state/battle_state.h"
#include "state/combat_entity.h"
#include "components/movement_component.h"
#include "components/transform_component.h"
#include "modules/hat_factory_hex_grid/hex_cell_data.h"
#include "modules/hat_factory_hex_grid/hex_grid_map_data.h"
#include "modules/hat_factory_hex_grid/hex_terrain_def.h"
#include "modules/hat_factory_hex_grid/hex_terrain_library.h"

#include "core/object/class_db.h"

namespace {
int qapi_cell_height(const Ref<HexGridMapData> &p_grid, const Vector2i &p_coord) {
	if (p_grid.is_null()) {
		return 0;
	}
	Ref<HexCellData> cell = p_grid->get_cell(p_coord);
	if (cell.is_null()) {
		return 0;
	}
	return cell->get_height();
}

int qapi_step_cost(const Ref<HexGridMapData> &p_grid, const Ref<HexTerrainLibrary> &p_lib, const Vector2i &p_coord, bool &r_blocks) {
	r_blocks = false;
	if (p_grid.is_null()) {
		return 1;
	}
	Ref<HexCellData> cell = p_grid->get_cell(p_coord);
	StringName terrain_id;
	if (cell.is_valid()) {
		terrain_id = cell->get_terrain_id();
	}
	if (p_lib.is_valid() && terrain_id != StringName()) {
		Ref<HexTerrainDef> def = p_lib->get_terrain(terrain_id);
		if (def.is_valid()) {
			if (def->get_blocks_movement()) {
				r_blocks = true;
				return 0;
			}
			return MAX(def->get_move_cost(), 1);
		}
	}
	return 1;
}
} //namespace

void QueryAPI::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_state", "state"), &QueryAPI::set_state);
	ClassDB::bind_method(D_METHOD("get_state"), &QueryAPI::get_state);
	ClassDB::bind_method(D_METHOD("has_entity", "entity_id"), &QueryAPI::has_entity);
	ClassDB::bind_method(D_METHOD("get_entity", "entity_id"), &QueryAPI::get_entity);
	ClassDB::bind_method(D_METHOD("get_entities_by_owner", "owner"), &QueryAPI::get_entities_by_owner);
	ClassDB::bind_method(D_METHOD("get_entity_at", "coord"), &QueryAPI::get_entity_at);
	ClassDB::bind_method(D_METHOD("get_reachable_cells", "actor_id", "range"), &QueryAPI::get_reachable_cells);
	ClassDB::bind_method(D_METHOD("can_move_to", "actor_id", "coord"), &QueryAPI::can_move_to);
	ClassDB::bind_static_method("QueryAPI", D_METHOD("hex_distance", "a", "b"), &QueryAPI::hex_distance);
	ClassDB::bind_static_method("QueryAPI", D_METHOD("get_neighbors", "coord"), &QueryAPI::get_neighbors);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "state", PROPERTY_HINT_RESOURCE_TYPE, "BattleState"), "set_state", "get_state");
}

int QueryAPI::hex_distance(const Vector2i &p_a, const Vector2i &p_b) {
	Vector2i d = p_a - p_b;
	return (Math::abs(d.x) + Math::abs(d.y) + Math::abs(d.x + d.y)) / 2;
}

TypedArray<Vector2i> QueryAPI::get_neighbors(const Vector2i &p_coord) {
	static const Vector2i DIRS[6] = {
		Vector2i(1, 0), Vector2i(-1, 0), Vector2i(0, 1),
		Vector2i(0, -1), Vector2i(1, -1), Vector2i(-1, 1)
	};
	TypedArray<Vector2i> result;
	for (int i = 0; i < 6; i++) {
		result.push_back(p_coord + DIRS[i]);
	}
	return result;
}

bool QueryAPI::has_entity(int p_id) const {
	return state.is_valid() && state->get_entity(p_id) != nullptr;
}

Ref<CombatEntity> QueryAPI::get_entity(int p_id) const {
	if (state.is_null()) {
		return Ref<CombatEntity>();
	}
	CombatEntity *entity = state->get_entity(p_id);
	if (entity == nullptr) {
		return Ref<CombatEntity>();
	}
	return Ref<CombatEntity>(entity);
}

TypedArray<int> QueryAPI::get_entities_by_owner(int p_owner) const {
	TypedArray<int> result;
	if (state.is_null()) {
		return result;
	}
	TypedArray<CombatEntity> ents = state->get_entities();
	for (int i = 0; i < ents.size(); i++) {
		Ref<CombatEntity> e = ents[i];
		if (e.is_valid() && e->get_owner() == p_owner) {
			result.push_back(e->get_entity_id());
		}
	}
	return result;
}

Ref<CombatEntity> QueryAPI::get_entity_at(const Vector2i &p_coord) const {
	if (state.is_null()) {
		return Ref<CombatEntity>();
	}
	TypedArray<CombatEntity> ents = state->get_entities();
	for (int i = 0; i < ents.size(); i++) {
		Ref<CombatEntity> e = ents[i];
		if (e.is_null() || !e->has_component(StringName("Transform"))) {
			continue;
		}
		Ref<TransformComponent> transform = e->get_component(StringName("Transform"));
		if (transform.is_valid() && transform->get_coord() == p_coord) {
			return e;
		}
	}
	return Ref<CombatEntity>();
}

TypedArray<Vector2i> QueryAPI::get_reachable_cells(int p_actor_id, int p_range) const {
	TypedArray<Vector2i> result;
	if (state.is_null() || p_range < 0) {
		return result;
	}
	CombatEntity *entity = state->get_entity(p_actor_id);
	if (entity == nullptr || !entity->has_component(StringName("Transform"))) {
		return result;
	}
	Ref<TransformComponent> transform = entity->get_component(StringName("Transform"));
	Vector2i origin = transform->get_coord();

	Ref<HexGridMapData> grid = state->get_grid_data();
	Ref<HexTerrainLibrary> terrain_lib = state->get_terrain_library();

	// Dijkstra over the hex graph. Each terrain contributes its own move cost
	// and may block movement outright; a height difference greater than one
	// level (in either direction) is impassable. Cells that do not exist in
	// the grid data or are occupied by another entity also block movement.
	HashMap<Vector2i, int> cost;
	LocalVector<Vector2i> frontier;
	cost[origin] = 0;
	frontier.push_back(origin);
	result.push_back(origin);

	while (!frontier.is_empty()) {
		// Extract the lowest-cost node from the frontier.
		int best_idx = 0;
		int best_cost = cost[frontier[0]];
		for (uint32_t i = 1; i < frontier.size(); i++) {
			int c = cost[frontier[i]];
			if (c < best_cost) {
				best_cost = c;
				best_idx = (int)i;
			}
		}
		Vector2i current = frontier[best_idx];
		frontier.remove_at(best_idx);
		int current_cost = cost[current];
		if (current_cost >= p_range) {
			continue;
		}
		int current_height = qapi_cell_height(grid, current);

		TypedArray<Vector2i> neighbors = get_neighbors(current);
		for (int n = 0; n < neighbors.size(); n++) {
			Vector2i coord = neighbors[n];
			if (grid.is_valid() && !grid->has_cell(coord)) {
				continue;
			}
			bool blocks = false;
			int step = qapi_step_cost(grid, terrain_lib, coord, blocks);
			if (blocks) {
				continue;
			}
			// Height gate: cannot climb or drop more than one level between cells.
			if (Math::abs(qapi_cell_height(grid, coord) - current_height) > 1) {
				continue;
			}
			// Occupied cells block movement (the actor's own cell is exempt).
			if (coord != origin) {
				Ref<CombatEntity> occupant = get_entity_at(coord);
				if (occupant.is_valid()) {
					continue;
				}
			}
			int new_cost = current_cost + step;
			if (new_cost > p_range) {
				continue;
			}
			if (cost.has(coord) && cost[coord] <= new_cost) {
				continue;
			}
			bool is_new = !cost.has(coord);
			cost[coord] = new_cost;
			if (is_new) {
				result.push_back(coord);
			}
			frontier.push_back(coord);
		}
	}
	return result;
}

bool QueryAPI::can_move_to(int p_actor_id, const Vector2i &p_coord) const {
	if (state.is_null()) {
		return false;
	}
	CombatEntity *entity = state->get_entity(p_actor_id);
	if (entity == nullptr) {
		return false;
	}
	int range = 0;
	if (entity->has_component(StringName("Movement"))) {
		Ref<MovementComponent> movement = entity->get_component(StringName("Movement"));
		if (movement.is_valid()) {
			range = movement->get_range();
		}
	}
	TypedArray<Vector2i> reachable = get_reachable_cells(p_actor_id, range);
	for (int i = 0; i < reachable.size(); i++) {
		if (reachable[i] == p_coord) {
			return true;
		}
	}
	return false;
}
