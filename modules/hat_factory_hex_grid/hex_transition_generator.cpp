#include "hex_transition_generator.h"

#include "hex_grid_map.h"
#include "hex_terrain_def.h"
#include "hex_terrain_library.h"
#include "core/math/basis.h"
#include "core/math/math_funcs.h"
#include "core/object/class_db.h"

void HexTransitionGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setup", "map", "library"), &HexTransitionGenerator::setup);
	ClassDB::bind_method(D_METHOD("generate_for_coord", "coord"), &HexTransitionGenerator::generate_for_coord);
	ClassDB::bind_method(D_METHOD("generate_all"), &HexTransitionGenerator::generate_all);
}

void HexTransitionGenerator::setup(HexGridMap *p_map, const Ref<HexTransitionLibrary> &p_library) {
	map = p_map;
	library = p_library;
}

TypedArray<HexTransitionInstanceData> HexTransitionGenerator::generate_for_coord(const Vector2i &p_coord) const {
	TypedArray<HexTransitionInstanceData> result;
	if (map == nullptr || library.is_null()) {
		return result;
	}
	if (!map->has_cell(p_coord)) {
		return result;
	}

	Ref<HexCellData> source = map->get_cell(p_coord);
	if (source.is_null()) {
		return result;
	}

	Ref<HexTerrainLibrary> terrain_library = map->get_terrain_library();
	float hex_size = map->get_hex_size();
	float cell_height_step = map->get_cell_height_step();

	for (int d = 0; d < 6; d++) {
		Vector2i neighbor_coord = map->get_neighbor(p_coord, d);
		if (!map->has_cell(neighbor_coord)) {
			continue;
		}
		Ref<HexCellData> target = map->get_cell(neighbor_coord);
		if (target.is_null()) {
			continue;
		}

		int height_delta = source->get_height() - target->get_height();

		Ref<HexTransitionDef> rule = library->find_rule(source, target, height_delta, terrain_library);
		if (rule.is_null() || rule->get_profile().is_null()) {
			continue;
		}

		Ref<HexTransitionProfile> profile = rule->get_profile();
		if (profile->get_mesh().is_null()) {
			continue;
		}

		Vector3 src_pos = map->map_to_local(p_coord, source->get_height());
		Vector3 tgt_pos = map->map_to_local(neighbor_coord, target->get_height());
		Vector3 delta = tgt_pos - src_pos;
		Vector3 flat_dir(delta.x, 0.0f, delta.z);

		Transform3D xform;
		xform.origin = src_pos + delta * 0.5f;
		xform.origin += profile->get_mesh_offset();

		if (!flat_dir.is_zero_approx()) {
			xform.basis = Basis::looking_at(flat_dir.normalized(), Vector3(0, 1, 0));
		}

		float edge_width = hex_size * 1.5f;
		float thickness = hex_size * 0.25f;
		float height_factor = MAX(1.0f, Math::abs((float)height_delta) * cell_height_step * profile->get_height_scale());

		Vector3 scale(edge_width, height_factor, thickness);
		scale *= profile->get_mesh_scale();
		xform.basis.scale(scale);

		Ref<HexTransitionInstanceData> instance;
		instance.instantiate();
		instance->set_source_coord(p_coord);
		instance->set_direction(d);
		instance->set_profile(profile);
		instance->set_transform(xform);
		instance->set_gameplay_flags(rule->get_gameplay_flags());
		instance->set_gameplay_state_id(rule->get_gameplay_state_id());

		result.append(instance);
	}

	return result;
}

TypedArray<HexTransitionInstanceData> HexTransitionGenerator::generate_all() const {
	TypedArray<HexTransitionInstanceData> result;
	if (map == nullptr) {
		return result;
	}
	TypedArray<Vector2i> used = map->get_used_cells();
	for (int i = 0; i < used.size(); i++) {
		TypedArray<HexTransitionInstanceData> cell_instances = generate_for_coord(used[i]);
		for (int j = 0; j < cell_instances.size(); j++) {
			result.append(cell_instances[j]);
		}
	}
	return result;
}
