/**************************************************************************/
/*  hex_spatial_query.cpp                                                 */
/**************************************************************************/
#include "hex_spatial_query.h"

#include "core/math/math_funcs.h"
#include "core/object/class_db.h"

#include <queue>

void HexQueryStep::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_to", "coord"), &HexQueryStep::set_to);
	ClassDB::bind_method(D_METHOD("get_to"), &HexQueryStep::get_to);
	ClassDB::bind_method(D_METHOD("set_action", "action"), &HexQueryStep::set_action);
	ClassDB::bind_method(D_METHOD("get_action"), &HexQueryStep::get_action);
	ClassDB::bind_method(D_METHOD("set_cost", "cost"), &HexQueryStep::set_cost);
	ClassDB::bind_method(D_METHOD("get_cost"), &HexQueryStep::get_cost);
}

void HexPathResult::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_found"), &HexPathResult::get_found);
	ClassDB::bind_method(D_METHOD("get_reason"), &HexPathResult::get_reason);
	ClassDB::bind_method(D_METHOD("get_steps"), &HexPathResult::get_steps);
	ClassDB::bind_method(D_METHOD("get_total_cost"), &HexPathResult::get_total_cost);
	ClassDB::bind_method(D_METHOD("get_world_steps"), &HexPathResult::get_world_steps);
}

void HexSpatialQuery::_bind_methods() {
	ClassDB::bind_static_method("HexSpatialQuery", D_METHOD("find_path", "view", "from", "to", "profile", "occupancy", "max_cost"), &HexSpatialQuery::find_path, DEFVAL(Ref<HexOccupancySnapshot>()), DEFVAL(64));
	ClassDB::bind_static_method("HexSpatialQuery", D_METHOD("reachable_cells", "view", "from", "profile", "occupancy", "max_cost"), &HexSpatialQuery::reachable_cells, DEFVAL(Ref<HexOccupancySnapshot>()), DEFVAL(64));
	ClassDB::bind_static_method("HexSpatialQuery", D_METHOD("has_line_of_sight", "view", "from", "to"), &HexSpatialQuery::has_line_of_sight);
	ClassDB::bind_static_method("HexSpatialQuery", D_METHOD("visible_cells", "view", "from", "range"), &HexSpatialQuery::visible_cells);
	ClassDB::bind_static_method("HexSpatialQuery", D_METHOD("find_world_path", "template", "from", "to", "profile"), &HexSpatialQuery::find_world_path);
}

namespace {

static constexpr int DIRS[6][2] = { { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, 0 }, { -1, 1 }, { 0, 1 } };

struct CellInfo {
	bool exists = false;
	bool has_data = false;
	int elevation = 0;
	int object_idx = -1; // def idx or -1
	bool object_blocks = false; // occupancy == occupies_cell
	bool object_blocks_vision = false;
	uint64_t surfaces = 0;
	uint64_t effects = 0;
	int edges[6] = { 0, 0, 0, 0, 0, 0 };
	bool edge_enabled[6] = { true, true, true, true, true, true };
};

CellInfo read_cell(const Ref<HexRegionSnapshot> &p_view, int32_t p_q, int32_t p_r) {
	CellInfo info;
	const hf_world::MicroAddr addr = hf_world::addr_from_micro(p_q, p_r);
	Ref<hf_world::HexShardState> state = p_view->find_shard_state(hf_world::pack_shard_key(addr.shard_q, addr.shard_r));
	if (state.is_null()) {
		return info;
	}
	info.exists = true;
	if (addr.page >= state->pages.size() || state->pages[addr.page].is_null()) {
		return info;
	}
	Ref<hf_world::HexShardPage> page = state->pages[addr.page];
	if (page->base_terrain[addr.offset] == hf_world::NO_DATA) {
		return info;
	}
	info.has_data = true;
	info.elevation = page->elevation[addr.offset];
	info.surfaces = page->surfaces[addr.offset];
	info.effects = page->area_effects[addr.offset];
	const int32_t obj = page->object[addr.offset];
	if (obj > 0) {
		info.object_idx = obj - 1;
	}
	for (int d = 0; d < 6; d++) {
		info.edges[d] = page->edges[addr.offset * 6 + d];
		info.edge_enabled[d] = page->edges_enabled[addr.offset * 6 + d] != 0;
	}
	return info;
}

// Fills object flags that need the table set.
void resolve_object(const Ref<HexRegionSnapshot> &p_view, CellInfo &r_info) {
	if (r_info.object_idx < 0) {
		return;
	}
	Ref<HexObjectDef> def = p_view->tables->get_object(r_info.object_idx);
	if (def.is_valid()) {
		r_info.object_blocks = def->get_occupancy() == StringName("occupies_cell");
		r_info.object_blocks_vision = def->get_blocks_vision();
	}
}

struct StepEval {
	bool ok = false;
	StringName reason;
	StringName action = StringName("walk");
	int cost = 1;
};

StepEval evaluate_step(const Ref<HexRegionSnapshot> &p_view, const CellInfo &p_from, const CellInfo &p_to, int p_dir, const Ref<HexMovementProfile> &p_profile, const Ref<HexOccupancySnapshot> &p_occupancy, int32_t p_to_q, int32_t p_to_r, int32_t p_mover_side) {
	StepEval out;
	if (!p_to.exists) {
		out.reason = StringName("out_of_bounds");
		return out;
	}
	if (!p_to.has_data) {
		out.reason = StringName("pending_data");
		return out;
	}
	// Entry cost from composed tags (base + surfaces + effects + blocking object).
	const Ref<HexTerrainTableSet> tables = p_view->tables;
	// base idx:
	// (CellInfo lacks it; recover)
	PackedStringArray tags;
	{
		const hf_world::MicroAddr addr = hf_world::addr_from_micro(p_to_q, p_to_r);
		Ref<hf_world::HexShardPage> page = p_view->find_page(addr);
		if (page.is_valid()) {
			const int32_t base_idx = page->base_terrain[addr.offset];
			if (base_idx >= 0) {
				Ref<HexBaseTerrainDef> def = tables->get_base_terrain(base_idx);
				if (def.is_valid()) {
					tags.append_array(def->get_tags());
				}
			}
		}
	}
	for (int i = 0; i < 64; i++) {
		if (p_to.surfaces & (1ULL << i)) {
			Ref<HexSurfaceDef> def = tables->get_surface(i);
			if (def.is_valid()) {
				tags.append_array(def->get_tags());
			}
		}
		if (p_to.effects & (1ULL << i)) {
			Ref<HexAreaEffectDef> def = tables->get_area_effect(i);
			if (def.is_valid()) {
				tags.append_array(def->get_tags());
				if (def->get_blocks_entry()) {
					out.reason = StringName("zone_forbidden");
					return out;
				}
			}
		}
	}
	if (p_to.object_idx >= 0) {
		Ref<HexObjectDef> def = tables->get_object(p_to.object_idx);
		if (def.is_valid()) {
			if (def->get_occupancy() == StringName("occupies_cell")) {
				out.reason = StringName("occupied");
				return out;
			}
			tags.append_array(def->get_tags());
		}
	}
	const int cell_cost = p_profile->get_cost_for_tags(tags);
	if (cell_cost < 0) {
		// Bridge/ford crossing: an enabled, non-blocking walk-action edge
		// carries the mover over a surface the profile cannot enter (e.g.
		// deep water under a bridge). Without this, bridges over water were
		// unusable — the surface rejection fired before edge mediation.
		// Hop-type destroyed forms (gap) deliberately do NOT bypass: hopping
		// a narrow seam still lands in the blocking surface.
		const int crossing_edge = p_from.edges[p_dir];
		if (crossing_edge != 0 && p_from.edge_enabled[p_dir]) {
			Ref<HexEdgeTypeDef> crossing_def = tables->get_edge_type(crossing_edge - 1);
			if (crossing_def.is_valid() && !crossing_def->get_blocks_movement()
					&& crossing_def->get_action_type() == StringName("walk")
					&& (p_to.elevation - p_from.elevation) <= p_profile->get_max_climb_height()) {
				out.ok = true;
				out.action = StringName("walk");
				out.cost = 1; // v1 flat crossing cost; balance 数值【待定】
				return out;
			}
		}
		out.reason = StringName("deep_water");
		return out;
	}
	// Unit occupancy (MAP-14): friendly may be crossed, never stopped on; enemy blocks.
	if (p_occupancy.is_valid() && p_occupancy->is_unit_at(MicroCoord::make(p_to_q, p_to_r))) {
		const int32_t side = p_occupancy->get_unit_side(MicroCoord::make(p_to_q, p_to_r));
		const bool friendly = side > 0 && side == p_mover_side && p_profile->get_can_cross_friendly();
		if (!friendly) {
			out.reason = StringName("occupied");
			return out;
		}
	}
	// Height + edge mediation.
	const int delta = p_to.elevation - p_from.elevation;
	const int edge = p_from.edges[p_dir];
	if (edge != 0) {
		if (!p_from.edge_enabled[p_dir]) {
			out.reason = StringName("edge_blocked");
			return out;
		}
		Ref<HexEdgeTypeDef> def = tables->get_edge_type(edge - 1);
		const StringName action = def->get_action_type();
		if (def->get_blocks_movement() && action == StringName("walk")) {
			out.reason = StringName("edge_blocked");
			return out;
		}
		if (action == StringName("hop")) {
			if (!p_profile->can_use_edge_action(StringName("hop")) || delta > def->get_climb_height()) {
				out.reason = StringName("insufficient_capability");
				return out;
			}
			out.action = StringName("hop");
		} else if (action == StringName("fall") || action == StringName("glide")) {
			// A drop edge: feather-class profiles glide when the drop allows,
			// everyone else falls; climbing up a drop is always a height failure.
			if (delta >= 0) {
				out.reason = StringName("height_delta");
				return out;
			}
			if (p_profile->can_use_edge_action(StringName("glide")) && -delta <= p_profile->get_glide_max_drop()) {
				out.action = StringName("glide");
			} else if (p_profile->can_use_edge_action(StringName("fall"))) {
				out.action = StringName("fall");
			} else {
				out.reason = StringName("height_delta");
				return out;
			}
		} else if (action == StringName("wade")) {
			out.action = StringName("wade");
		} else if (action == StringName("break_through")) {
			if (!p_profile->can_use_edge_action(StringName("break_through"))) {
				out.reason = StringName("insufficient_capability");
				return out;
			}
			out.action = StringName("break_through");
		}
	} else {
		if (delta > p_profile->get_max_climb_height()) {
			out.reason = StringName("height_delta");
			return out;
		}
		if (delta < 0) {
			// Descending open ground: glide when the profile allows it
			// (feather-class), otherwise fall.
			if (p_profile->can_use_edge_action(StringName("glide")) && -delta <= p_profile->get_glide_max_drop()) {
				out.action = StringName("glide");
			} else if (p_profile->can_use_edge_action(StringName("fall"))) {
				out.action = StringName("fall");
			} else {
				out.reason = StringName("height_delta");
				return out;
			}
		}
	}
	out.ok = true;
	out.cost = cell_cost;
	return out;
}

struct QueueNode {
	int cost;
	int32_t q;
	int32_t r;
	bool operator<(const QueueNode &p_other) const {
		if (cost != p_other.cost) {
			return cost > p_other.cost; // min-heap via reversed compare
		}
		if (q != p_other.q) {
			return q > p_other.q;
		}
		return r > p_other.r;
	}
};

uint64_t pack_key(int32_t q, int32_t r) {
	return ((uint64_t)(uint32_t)q << 32) | (uint32_t)r;
}

} // namespace

Ref<HexPathResult> HexSpatialQuery::find_path(const Ref<HexRegionSnapshot> &p_view, const Ref<MicroCoord> &p_from, const Ref<MicroCoord> &p_to, const Ref<HexMovementProfile> &p_profile, const Ref<HexOccupancySnapshot> &p_occupancy, int p_max_cost) {
	Ref<HexPathResult> result;
	result.instantiate();
	ERR_FAIL_COND_V(p_view.is_null() || p_from.is_null() || p_to.is_null() || p_profile.is_null(), result);

	const CellInfo from_info = read_cell(p_view, p_from->get_q(), p_from->get_r());
	if (!from_info.exists || !from_info.has_data) {
		result->reason = from_info.exists ? StringName("pending_data") : StringName("out_of_bounds");
		return result;
	}
	if (p_from->equals(p_to)) {
		result->found = true;
		return result;
	}
	const CellInfo target_info = read_cell(p_view, p_to->get_q(), p_to->get_r());
	if (!target_info.exists) {
		result->reason = StringName("out_of_bounds");
		return result;
	}
	if (!target_info.has_data) {
		result->reason = StringName("pending_data");
		return result;
	}
	if (target_info.object_idx >= 0) {
		CellInfo ti = target_info;
		resolve_object(p_view, ti);
		if (ti.object_blocks) {
			result->reason = StringName("occupied");
			return result;
		}
	}
	if (p_occupancy.is_valid() && p_occupancy->is_unit_at(p_to)) {
		result->reason = StringName("occupied");
		return result;
	}

	// Dijkstra over micro cells.
	std::priority_queue<QueueNode> open;
	HashMap<uint64_t, int> dist;
	HashMap<uint64_t, uint64_t> parent;
	HashMap<uint64_t, StringName> parent_action;
	StringName best_failure;
	int best_failure_cost = INT32_MAX;

	open.push({ 0, p_from->get_q(), p_from->get_r() });
	dist.insert(pack_key(p_from->get_q(), p_from->get_r()), 0);

	while (!open.empty()) {
		const QueueNode cur = open.top();
		open.pop();
		const uint64_t cur_key = pack_key(cur.q, cur.r);
		const int known = dist.get(cur_key);
		if (cur.cost > known) {
			continue;
		}
		const CellInfo cur_info = read_cell(p_view, cur.q, cur.r);
		for (int d = 0; d < 6; d++) {
			const int32_t nq = cur.q + DIRS[d][0];
			const int32_t nr = cur.r + DIRS[d][1];
			const uint64_t nkey = pack_key(nq, nr);
			if (dist.has(nkey)) {
				continue;
			}
			const CellInfo next_info = read_cell(p_view, nq, nr);
			const StepEval eval = evaluate_step(p_view, cur_info, next_info, d, p_profile, p_occupancy, nq, nr, 1);
			if (!eval.ok) {
				if (nq == p_to->get_q() && nr == p_to->get_r()) {
					result->reason = eval.reason;
				}
				if (cur.cost < best_failure_cost) {
					best_failure_cost = cur.cost;
					best_failure = eval.reason;
				}
				continue;
			}
			const int new_cost = cur.cost + eval.cost;
			if (new_cost > p_max_cost) {
				continue;
			}
			dist.insert(nkey, new_cost);
			parent.insert(nkey, cur_key);
			parent_action.insert(nkey, eval.action);
			if (nq == p_to->get_q() && nr == p_to->get_r()) {
				result->found = true;
				result->total_cost = new_cost;
				// Reconstruct.
				Vector<uint64_t> chain;
				uint64_t k = nkey;
				while (k != pack_key(p_from->get_q(), p_from->get_r())) {
					chain.push_back(k);
					k = parent.get(k);
				}
				for (int i = chain.size() - 1; i >= 0; i--) {
					Ref<HexQueryStep> step;
					step.instantiate();
					step->set_to(MicroCoord::make((int32_t)(chain[i] >> 32), (int32_t)(uint32_t)chain[i], 1));
					step->set_action(parent_action.get(chain[i]));
					step->set_cost(dist.get(chain[i]));
					result->steps.append(step);
				}
				return result;
			}
			open.push({ new_cost, nq, nr });
		}
	}
	if (result->reason == StringName("reachable")) {
		result->reason = best_failure != StringName() ? best_failure : StringName("unreachable");
	}
	return result;
}

TypedArray<MicroCoord> HexSpatialQuery::reachable_cells(const Ref<HexRegionSnapshot> &p_view, const Ref<MicroCoord> &p_from, const Ref<HexMovementProfile> &p_profile, const Ref<HexOccupancySnapshot> &p_occupancy, int p_max_cost) {
	TypedArray<MicroCoord> out;
	ERR_FAIL_COND_V(p_view.is_null() || p_from.is_null() || p_profile.is_null(), out);
	const CellInfo from_info = read_cell(p_view, p_from->get_q(), p_from->get_r());
	if (!from_info.has_data) {
		return out;
	}
	std::priority_queue<QueueNode> open;
	HashMap<uint64_t, int> dist;
	open.push({ 0, p_from->get_q(), p_from->get_r() });
	dist.insert(pack_key(p_from->get_q(), p_from->get_r()), 0);
	while (!open.empty()) {
		const QueueNode cur = open.top();
		open.pop();
		const uint64_t cur_key = pack_key(cur.q, cur.r);
		if (cur.cost > dist.get(cur_key)) {
			continue;
		}
		out.append(MicroCoord::make(cur.q, cur.r, 1));
		const CellInfo cur_info = read_cell(p_view, cur.q, cur.r);
		for (int d = 0; d < 6; d++) {
			const int32_t nq = cur.q + DIRS[d][0];
			const int32_t nr = cur.r + DIRS[d][1];
			const uint64_t nkey = pack_key(nq, nr);
			if (dist.has(nkey)) {
				continue;
			}
			const CellInfo next_info = read_cell(p_view, nq, nr);
			const StepEval eval = evaluate_step(p_view, cur_info, next_info, d, p_profile, p_occupancy, nq, nr, 1);
			if (!eval.ok) {
				continue;
			}
			const int new_cost = cur.cost + eval.cost;
			if (new_cost > p_max_cost) {
				continue;
			}
			dist.insert(nkey, new_cost);
			open.push({ new_cost, nq, nr });
		}
	}
	return out;
}

bool HexSpatialQuery::has_line_of_sight(const Ref<HexRegionSnapshot> &p_view, const Ref<MicroCoord> &p_from, const Ref<MicroCoord> &p_to) {
	ERR_FAIL_COND_V(p_view.is_null() || p_from.is_null() || p_to.is_null(), false);
	if (p_from->equals(p_to)) {
		return true;
	}
	const CellInfo a = read_cell(p_view, p_from->get_q(), p_from->get_r());
	const CellInfo b = read_cell(p_view, p_to->get_q(), p_to->get_r());
	if (!a.has_data || !b.has_data) {
		return false;
	}
	// Hex line via cube lerp (standard algorithm).
	int32_t x0 = p_from->get_q();
	int32_t z0 = p_from->get_r();
	int32_t y0 = -x0 - z0;
	const int32_t x1 = p_to->get_q();
	const int32_t z1 = p_to->get_r();
	const int32_t y1 = -x1 - z1;
	const int n = MAX(MAX(Math::abs(x1 - x0), Math::abs(y1 - y0)), Math::abs(z1 - z0));
	const int max_elev = MAX(a.elevation, b.elevation);
	int32_t px = x0, pz = z0;
	for (int i = 1; i < n; i++) {
		const double t = (double)i / (double)n;
		const double xi = x0 + (x1 - x0) * t;
		const double yi = y0 + (y1 - y0) * t;
		const double zi = z0 + (z1 - z0) * t;
		const double rx = Math::round(xi);
		const double ry = Math::round(yi);
		const double rz = Math::round(zi);
		const double dx = Math::abs(rx - xi);
		const double dy = Math::abs(ry - yi);
		const double dz = Math::abs(rz - zi);
		int32_t cx, cy, cz;
		if (dx > dy && dx > dz) {
			cx = (int32_t)(-ry - rz);
			cy = (int32_t)ry;
			cz = (int32_t)rz;
		} else if (dy > dz) {
			cx = (int32_t)rx;
			cy = (int32_t)(-rx - rz);
			cz = (int32_t)rz;
		} else {
			cx = (int32_t)rx;
			cy = (int32_t)ry;
			cz = (int32_t)(-rx - ry);
		}
		// Crossed-edge vision blocker on the previous boundary.
		for (int d = 0; d < 6; d++) {
			if (px + DIRS[d][0] == cx && pz + DIRS[d][1] == cz) {
				const CellInfo prev = read_cell(p_view, px, pz);
				const int edge = prev.edges[d];
				if (edge != 0 && prev.edge_enabled[d]) {
					Ref<HexEdgeTypeDef> def = p_view->tables->get_edge_type(edge - 1);
					if (def.is_valid() && def->get_blocks_vision()) {
						return false;
					}
				}
				break;
			}
		}
		const CellInfo mid = read_cell(p_view, cx, cz);
		if (mid.has_data) {
			CellInfo mi = mid;
			resolve_object(p_view, mi);
			if (mi.elevation > max_elev) {
				return false;
			}
			if (mi.object_blocks_vision) {
				return false;
			}
			for (int s = 0; s < 64; s++) {
				if (mi.surfaces & (1ULL << s)) {
					Ref<HexSurfaceDef> def = p_view->tables->get_surface(s);
					if (def.is_valid() && def->get_blocks_vision()) {
						return false;
					}
				}
			}
		}
		px = cx;
		pz = cz;
	}
	return true;
}

TypedArray<MicroCoord> HexSpatialQuery::visible_cells(const Ref<HexRegionSnapshot> &p_view, const Ref<MicroCoord> &p_from, int p_range) {
	TypedArray<MicroCoord> out;
	ERR_FAIL_COND_V(p_view.is_null() || p_from.is_null(), out);
	const CellInfo from_info = read_cell(p_view, p_from->get_q(), p_from->get_r());
	if (!from_info.has_data || p_range <= 0) {
		return out;
	}
	int range = p_range;
	for (int i = 0; i < 64; i++) {
		if (from_info.effects & (1ULL << i)) {
			Ref<HexAreaEffectDef> def = p_view->tables->get_area_effect(i);
			if (def.is_valid() && def->get_vision_modifier() > 0.0) {
				range = (int)(range * def->get_vision_modifier());
			}
		}
	}
	// Spiral rings around the origin.
	for (int dq = -range; dq <= range; dq++) {
		for (int dr = -range; dr <= range; dr++) {
			const int ds = -dq - dr;
			const int hex_dist = MAX(MAX(Math::abs(dq), Math::abs(dr)), Math::abs(ds));
			if (hex_dist > range || (dq == 0 && dr == 0)) {
				continue;
			}
			if (has_line_of_sight(p_view, p_from, MicroCoord::make(p_from->get_q() + dq, p_from->get_r() + dr, 1))) {
				out.append(MicroCoord::make(p_from->get_q() + dq, p_from->get_r() + dr, 1));
			}
		}
	}
	return out;
}

Ref<HexPathResult> HexSpatialQuery::find_world_path(const Ref<HexWorldTemplate> &p_template, const Ref<WorldCellCoord> &p_from, const Ref<WorldCellCoord> &p_to, const Ref<HexMovementProfile> &p_profile) {
	Ref<HexPathResult> result;
	result.instantiate();
	ERR_FAIL_COND_V(p_template.is_null() || p_from.is_null() || p_to.is_null(), result);
	if (!p_template->is_compiled() && !p_template->compile()) {
		result->reason = StringName("invalid_template");
		return result;
	}
	if (p_template->get_cell_at(p_from).is_null() || p_template->get_cell_at(p_to).is_null()) {
		result->reason = StringName("out_of_bounds");
		return result;
	}
	// Portal lookup: normalized edge key -> portal def.
	HashMap<uint64_t, Ref<HexWorldPortalDef>> portals;
	const TypedArray<HexWorldPortalDef> portal_defs = p_template->get_portals();
	for (int i = 0; i < portal_defs.size(); i++) {
		Ref<HexWorldPortalDef> p = portal_defs[i];
		if (p.is_null() || p->get_edge().is_null()) {
			continue;
		}
		const Ref<WorldCellCoord> a = p->get_edge()->get_endpoint_a();
		const Ref<WorldCellCoord> b = p->get_edge()->get_endpoint_b();
		const uint64_t key = (((uint64_t)(uint32_t)a->get_q() << 48) | ((uint64_t)(uint32_t)a->get_r() << 32) |
				((uint64_t)(uint32_t)b->get_q() << 16) | (uint32_t)b->get_r());
		portals.insert(key, p);
	}
	auto portal_between = [&](int32_t aq, int32_t ar, int32_t bq, int32_t br) -> Ref<HexWorldPortalDef> {
		Ref<WorldCellCoord> a = WorldCellCoord::make(aq, ar);
		Ref<WorldCellCoord> b = WorldCellCoord::make(bq, br);
		if (a->compare_to(b) > 0) {
			const Ref<WorldCellCoord> t = a;
			a = b;
			b = t;
		}
		const uint64_t key = (((uint64_t)(uint32_t)a->get_q() << 48) | ((uint64_t)(uint32_t)a->get_r() << 32) |
				((uint64_t)(uint32_t)b->get_q() << 16) | (uint32_t)b->get_r());
		const Ref<HexWorldPortalDef> *p = portals.getptr(key);
		return p ? *p : Ref<HexWorldPortalDef>();
	};

	// Dijkstra over declared world cells; explicit portals gate passage.
	std::priority_queue<QueueNode> open;
	HashMap<uint64_t, int> dist;
	HashMap<uint64_t, uint64_t> parent;
	open.push({ 0, p_from->get_q(), p_from->get_r() });
	dist.insert(pack_key(p_from->get_q(), p_from->get_r()), 0);
	while (!open.empty()) {
		const QueueNode cur = open.top();
		open.pop();
		const uint64_t cur_key = pack_key(cur.q, cur.r);
		if (cur.cost > dist.get(cur_key)) {
			continue;
		}
		if (cur.q == p_to->get_q() && cur.r == p_to->get_r()) {
			result->found = true;
			result->total_cost = cur.cost;
			Vector<uint64_t> chain;
			uint64_t k = cur_key;
			const uint64_t start_key = pack_key(p_from->get_q(), p_from->get_r());
			while (k != start_key) {
				chain.push_back(k);
				k = parent.get(k);
			}
			for (int i = chain.size() - 1; i >= 0; i--) {
				Dictionary d;
				d[StringName("q")] = (int32_t)(chain[i] >> 32);
				d[StringName("r")] = (int32_t)(uint32_t)chain[i];
				result->world_steps.append(d);
			}
			return result;
		}
		for (int d = 0; d < 6; d++) {
			const int32_t nq = cur.q + DIRS[d][0];
			const int32_t nr = cur.r + DIRS[d][1];
			const uint64_t nkey = pack_key(nq, nr);
			if (dist.has(nkey) || p_template->get_cell_at(WorldCellCoord::make(nq, nr)).is_null()) {
				continue;
			}
			const Ref<HexWorldPortalDef> portal = portal_between(cur.q, cur.r, nq, nr);
			if (portal.is_valid() && !portal->get_enabled_by_default()) {
				continue; // closed shortcut / locked gate (keys are consumed at open time)
			}
			dist.insert(nkey, cur.cost + 1);
			parent.insert(nkey, cur_key);
			open.push({ cur.cost + 1, nq, nr });
		}
	}
	result->reason = StringName("unreachable");
	return result;
}
