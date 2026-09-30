/**************************************************************************/
/*  hex_encounter_overlay.cpp                                             */
/**************************************************************************/
#include "hex_encounter_overlay.h"

#include "../runtime/hex_command_resolver.h"
#include "../runtime/hex_world_run.h"

#include "core/math/math_funcs.h"
#include "core/object/class_db.h"

static thread_local String g_last_expand_error;

void HexEncounterOverlay::_bind_methods() {
	ClassDB::bind_static_method("HexEncounterOverlay", D_METHOD("expand", "run", "bounds"), &HexEncounterOverlay::expand);
	ClassDB::bind_static_method("HexEncounterOverlay", D_METHOD("get_last_expand_error"), &HexEncounterOverlay::get_last_expand_error);
	ClassDB::bind_method(D_METHOD("get_world_cells"), &HexEncounterOverlay::get_world_cells);
	ClassDB::bind_method(D_METHOD("get_battle_cells"), &HexEncounterOverlay::get_battle_cells);
	ClassDB::bind_method(D_METHOD("contains_micro", "coord"), &HexEncounterOverlay::contains_micro);
	ClassDB::bind_method(D_METHOD("get_view"), &HexEncounterOverlay::get_view);
	ClassDB::bind_method(D_METHOD("get_start_seq"), &HexEncounterOverlay::get_start_seq);
	ClassDB::bind_method(D_METHOD("project_placements", "profile"), &HexEncounterOverlay::project_placements);
	ClassDB::bind_method(D_METHOD("get_placement_error"), &HexEncounterOverlay::get_placement_error);
	ClassDB::bind_method(D_METHOD("get_placements"), &HexEncounterOverlay::get_placements);
	ClassDB::bind_method(D_METHOD("get_placement", "entity"), &HexEncounterOverlay::get_placement);
	ClassDB::bind_method(D_METHOD("apply_battle_command", "command"), &HexEncounterOverlay::apply_battle_command);
	ClassDB::bind_method(D_METHOD("world_position_of", "coord"), &HexEncounterOverlay::world_position_of);
	ClassDB::bind_method(D_METHOD("collect_deltas"), &HexEncounterOverlay::collect_deltas);
}

String HexEncounterOverlay::get_last_expand_error() {
	return g_last_expand_error;
}

Ref<HexEncounterOverlay> HexEncounterOverlay::expand(const Ref<HexWorldRun> &p_run, const Ref<HexEncounterBounds> &p_bounds) {
	g_last_expand_error = String();
	auto fail = [&](const String &p_msg) {
		g_last_expand_error = p_msg;
		return Ref<HexEncounterOverlay>();
	};
	ERR_FAIL_COND_V_MSG(p_run.is_null() || p_bounds.is_null(), Ref<HexEncounterOverlay>(), "HexEncounterOverlay::expand: null run/bounds");
	const String verr = p_bounds->validate();
	if (!verr.is_empty()) {
		return fail(verr);
	}
	Ref<HexEncounterOverlay> eo;
	eo.instantiate();
	eo->bounds = p_bounds;
	eo->start_seq = p_run->get_global_seq();
	eo->base_view = p_run->new_battle_snapshot();
	eo->overlay = eo->base_view;

	// Arena readiness: every micro cell inside the bounds must be materialized.
	const TypedArray<MicroCoord> cells = eo->get_battle_cells();
	if (cells.is_empty()) {
		return fail("arena covers no micro cells");
	}
	for (int i = 0; i < cells.size(); i++) {
		if (!eo->overlay->has_cell_data(cells[i])) {
			return fail("arena_not_ready: micro cell missing authoritative data (no placeholder battles)");
		}
	}
	return eo;
}

TypedArray<WorldCellCoord> HexEncounterOverlay::get_world_cells() const {
	TypedArray<WorldCellCoord> out;
	if (bounds.is_null()) {
		return out;
	}
	if (bounds->get_bounds_kind() == StringName("cells")) {
		return bounds->get_explicit_cells();
	}
	const Ref<WorldCellCoord> c = bounds->get_center();
	const int radius = bounds->get_world_radius();
	for (int dq = -radius; dq <= radius; dq++) {
		for (int dr = -radius; dr <= radius; dr++) {
			const int ds = -dq - dr;
			if (MAX(MAX(Math::abs(dq), Math::abs(dr)), Math::abs(ds)) <= radius) {
				out.append(WorldCellCoord::make(c->get_q() + dq, c->get_r() + dr));
			}
		}
	}
	return out;
}

TypedArray<MicroCoord> HexEncounterOverlay::get_battle_cells() const {
	TypedArray<MicroCoord> out;
	ERR_FAIL_COND_V(overlay.is_null(), out);
	Ref<HexWorldRun> run = overlay->get_run();
	ERR_FAIL_COND_V(run.is_null() || run->get_world_template().is_null(), out);
	Ref<HexScaleMapping> mapping = HexScaleMapping::create(run->get_world_template()->get_detail_descriptor());
	const int scale = run->get_world_template()->get_detail_descriptor()->get_scale();
	const TypedArray<WorldCellCoord> wc = get_world_cells();
	for (int i = 0; i < wc.size(); i++) {
		const Ref<MicroCoord> anchor = mapping->world_cell_to_micro_anchor(wc[i]);
		// Voronoi ownership is not an axis-aligned square around the anchor;
		// scan a window and keep cells that truly belong to this world cell.
		for (int dq = -scale; dq <= scale; dq++) {
			for (int dr = -scale; dr <= scale; dr++) {
				const int32_t mq = anchor->get_q() + dq;
				const int32_t mr = anchor->get_r() + dr;
				Ref<WorldCellCoord> owner = mapping->micro_cell_to_world_cell(MicroCoord::make(mq, mr, 1));
				if (owner->equals(wc[i])) {
					out.append(MicroCoord::make(mq, mr, 1));
				}
			}
		}
	}
	return out;
}

bool HexEncounterOverlay::contains_micro(const Ref<MicroCoord> &p_coord) const {
	const TypedArray<MicroCoord> cells = get_battle_cells();
	for (int i = 0; i < cells.size(); i++) {
		Ref<MicroCoord> cell = cells[i];
		if (cell->equals(p_coord)) {
			return true;
		}
	}
	return false;
}

bool HexEncounterOverlay::project_placements(const Ref<HexMovementProfile> &p_profile) {
	ERR_FAIL_COND_V(overlay.is_null() || bounds.is_null(), false);
	placements.clear();
	placement_error = String();
	Ref<HexWorldRun> run = overlay->get_run();
	const Dictionary participants = bounds->get_participants();
	const LocalVector<Variant> ids = participants.get_key_list();
	// Stable processing order: entity id, lexicographic.
	Vector<StringName> ordered;
	for (const Variant &id : ids) {
		ordered.append(StringName(id));
	}
	struct SNameLess {
		bool operator()(const StringName &a, const StringName &b) const { return String(a).casecmp_to(String(b)) < 0; }
	};
	ordered.sort_custom<SNameLess>();

	const TypedArray<MicroCoord> arena = get_battle_cells();
	for (const StringName &id : ordered) {
		Dictionary pos = participants.get(id, Dictionary());
		const Vector2 world_pos(pos.get(StringName("x"), 0.0), pos.get(StringName("z"), 0.0));
		// Choose the nearest legal cell: distance to micro center, lex tie-break.
		Ref<MicroCoord> best;
		double best_dist = 1e30;
		for (int i = 0; i < arena.size(); i++) {
			const Ref<MicroCoord> cand = arena[i];
			if (placements.values().find(cand->to_dict()) >= 0) {
				continue; // uniqueness: one entity per cell
			}
			if (!overlay->has_cell_data(cand)) {
				continue;
			}
			// Legality: blocking objects reject the cell.
			Dictionary view = overlay->get_cell_view(cand);
			const StringName object_id = view.get(StringName("object"), StringName());
			if (object_id != StringName()) {
				const int oidx = run->get_tables()->find_object(object_id);
				if (oidx >= 0 && run->get_tables()->get_object(oidx)->get_occupancy() == StringName("occupies_cell")) {
					continue;
				}
			}
			const Vector3 c3 = world_position_of(cand);
			const Vector2 cpos(c3.x, c3.z);
			const double dist = cpos.distance_to(world_pos);
			if (dist < best_dist - 1e-9) {
				best_dist = dist;
				best = cand;
			} else if (Math::abs(dist - best_dist) <= 1e-9 && best.is_valid()) {
				if (cand->compare_to(best) < 0) {
					best = cand;
				}
			}
		}
		if (best.is_null()) {
			placement_error = "no legal placement for participant '" + String(id) + "' — refusing battle start (level layout error)";
			placements.clear();
			return false;
		}
		placements[id] = best->to_dict();
	}
	return true;
}

Ref<MicroCoord> HexEncounterOverlay::get_placement(const StringName &p_entity) const {
	const Variant *v = placements.getptr(p_entity);
	if (v == nullptr) {
		return Ref<MicroCoord>();
	}
	return MicroCoord::from_dict(*v);
}

Ref<HexCommandResult> HexEncounterOverlay::apply_battle_command(const Ref<HexMapCommand> &p_command) {
	Ref<HexCommandResult> result = HexCommandResolver::apply(overlay, p_command);
	if (result->accepted) {
		overlay = result->next_snapshot;
	}
	return result;
}

Vector3 HexEncounterOverlay::world_position_of(const Ref<MicroCoord> &p_coord) const {
	ERR_FAIL_COND_V(overlay.is_null() || p_coord.is_null(), Vector3());
	Ref<HexWorldRun> run = overlay->get_run();
	const double hex_size = run->get_world_template()->get_world_hex_size();
	Ref<HexScaleMapping> mapping = HexScaleMapping::create(run->get_world_template()->get_detail_descriptor());
	const Vector2 c = mapping->micro_cell_center(p_coord, hex_size);
	return Vector3(c.x, 0.0, c.y);
}

TypedArray<HexMapCommand> HexEncounterOverlay::collect_deltas() const {
	TypedArray<HexMapCommand> out;
	ERR_FAIL_COND_V(overlay.is_null() || base_view.is_null(), out);
	const TypedArray<MicroCoord> cells = get_battle_cells();
	for (int i = 0; i < cells.size(); i++) {
		const Ref<MicroCoord> coord = cells[i];
		Dictionary after = overlay->get_cell_view(coord);
		Dictionary before = base_view->get_cell_view(coord);
		if (!_is_cell_equal(before, after)) {
			_push_delta_commands(before, after, coord, out);
		}
	}
	return out;
}

bool HexEncounterOverlay::_is_cell_equal(const Dictionary &p_before, const Dictionary &p_after) {
	static const char *keys[] = { "base_terrain", "elevation", "surfaces", "object", "edges", "area_effects", nullptr };
	for (int k = 0; keys[k]; k++) {
		const StringName key(keys[k]);
		if (p_before.get(key, Variant()) != p_after.get(key, Variant())) {
			return false;
		}
	}
	return true;
}

void HexEncounterOverlay::_push_delta_commands(const Dictionary &p_before, const Dictionary &p_after, const Ref<MicroCoord> &p_coord, TypedArray<HexMapCommand> &r_out) {
	const StringName b_base = p_before.get(StringName("base_terrain"), StringName());
	const StringName a_base = p_after.get(StringName("base_terrain"), StringName());
	if (a_base != b_base && a_base != StringName()) {
		r_out.append(HexMapCommand::set_cell_base(p_coord, a_base));
	}
	const int b_elev = p_before.get(StringName("elevation"), 0);
	const int a_elev = p_after.get(StringName("elevation"), 0);
	if (a_elev != b_elev) {
		r_out.append(HexMapCommand::set_cell_elevation(p_coord, a_elev));
	}
	// Edges: set per changed slot.
	Array b_edges = p_before.get(StringName("edges"), Array());
	Array a_edges = p_after.get(StringName("edges"), Array());
	for (int d = 0; d < MIN(b_edges.size(), a_edges.size()); d++) {
		Dictionary be = b_edges[d];
		Dictionary ae = a_edges[d];
		const StringName bt = be.get(StringName("type"), StringName());
		const StringName at = ae.get(StringName("type"), StringName());
		const bool ben = be.get(StringName("enabled"), true);
		const bool aen = ae.get(StringName("enabled"), true);
		if (at != bt) {
			if (at == StringName()) {
				r_out.append(HexMapCommand::set_edge(p_coord, d, StringName("gap"), true)); // cleared edge -> open gap
			} else {
				r_out.append(HexMapCommand::set_edge(p_coord, d, at, aen));
			}
		} else if (aen != ben && at != StringName()) {
			r_out.append(HexMapCommand::set_edge_enabled(p_coord, d, aen));
		}
	}
	// Objects: place / destroy.
	const StringName b_obj = p_before.get(StringName("object"), StringName());
	const StringName a_obj = p_after.get(StringName("object"), StringName());
	if (a_obj != b_obj) {
		if (a_obj == StringName()) {
			r_out.append(HexMapCommand::destroy_object(p_coord));
		} else {
			// Durability loss on a surviving object is committed by damage;
			// fresh placements carry full durability from the table.
			if (b_obj == StringName()) {
				r_out.append(HexMapCommand::place_object(p_coord, a_obj));
			} else {
				r_out.append(HexMapCommand::destroy_object(p_coord));
				r_out.append(HexMapCommand::place_object(p_coord, a_obj));
			}
		}
	}
	// Area effects: apply added, remove missing.
	PackedStringArray b_fx = p_before.get(StringName("area_effects"), PackedStringArray());
	PackedStringArray a_fx = p_after.get(StringName("area_effects"), PackedStringArray());
	for (int i = 0; i < a_fx.size(); i++) {
		if (!b_fx.has(a_fx[i])) {
			r_out.append(HexMapCommand::apply_area_effect(p_coord, a_fx[i]));
		}
	}
	for (int i = 0; i < b_fx.size(); i++) {
		if (!a_fx.has(b_fx[i])) {
			r_out.append(HexMapCommand::remove_area_effect(p_coord, b_fx[i]));
		}
	}
}
