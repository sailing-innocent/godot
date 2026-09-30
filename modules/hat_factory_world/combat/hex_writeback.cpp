/**************************************************************************/
/*  hex_writeback.cpp                                                     */
/**************************************************************************/
#include "hex_writeback.h"

#include "../runtime/hex_command_resolver.h"
#include "../runtime/hex_world_run.h"

#include "core/object/class_db.h"

void HexWriteback::_bind_methods() {
	ClassDB::bind_static_method("HexWriteback", D_METHOD("commit", "run", "overlay", "victory"), &HexWriteback::commit);
}

Dictionary HexWriteback::commit(const Ref<HexWorldRun> &p_run, const Ref<HexEncounterOverlay> &p_overlay, bool p_victory) {
	Dictionary report;
	report[StringName("committed")] = 0;
	report[StringName("rejected")] = 0;
	report[StringName("skipped")] = true;
	report[StringName("summary_dirty")] = TypedArray<WorldCellCoord>();
	ERR_FAIL_COND_V(p_run.is_null() || p_overlay.is_null() || p_overlay->bounds.is_null(), report);

	const Ref<HexEncounterBounds> bounds = p_overlay->bounds;
	const StringName strategy = bounds->get_commit_strategy();
	bool should_commit = false;
	if (strategy == StringName("commit_all")) {
		should_commit = true;
	} else if (strategy == StringName("commit_on_victory")) {
		if (p_victory) {
			should_commit = true;
		} else {
			should_commit = bounds->get_defeat_strategy() == StringName("keep");
		}
	}
	if (!should_commit) {
		return report; // discard / rollback: world keeps its pre-battle state
	}
	report[StringName("skipped")] = false;

	const TypedArray<HexMapCommand> deltas = p_overlay->collect_deltas();
	TypedArray<WorldCellCoord> summary_dirty;
	int committed = 0;
	int rejected = 0;
	for (int i = 0; i < deltas.size(); i++) {
		Ref<HexMapCommand> cmd = deltas[i];
		// Re-stamp the committed version; the resolver re-validates everything.
		cmd->set_expected_version(p_run->get_global_seq());
		Ref<HexCommandResult> result = p_run->apply_command(cmd);
		if (result->accepted) {
			committed++;
		} else {
			rejected++;
			WARN_PRINT("HexWriteback: delta rejected (" + String(result->reason) + ") at commit");
		}
	}

	// Macro summary recompute (MAP-04): only world cells whose water coverage
	// crossed the declared threshold get a refreshed summary. Micro edits never
	// turn a whole world body of water into something else by themselves.
	Ref<HexScaleMapping> mapping = HexScaleMapping::create(p_run->get_world_template()->get_detail_descriptor());
	const int scale = p_run->get_world_template()->get_detail_descriptor()->get_scale();
	const double threshold = bounds->get_coverage_summary_threshold();
	const TypedArray<MicroCoord> battle = p_overlay->get_battle_cells();
	HashMap<uint64_t, bool> seen_world;
	for (int i = 0; i < battle.size(); i++) {
		const Ref<WorldCellCoord> wc = mapping->micro_cell_to_world_cell(battle[i]);
		const uint64_t key = (((uint64_t)(uint32_t)wc->get_q()) << 32) | (uint32_t)wc->get_r();
		if (seen_world.has(key)) {
			continue;
		}
		seen_world.insert(key, true);
		// Count water coverage before/after across the world cell's micro cells.
		const Ref<MicroCoord> anchor = mapping->world_cell_to_micro_anchor(wc);
		auto coverage = [&](const Ref<HexRegionSnapshot> &p_view) -> double {
			int water = 0;
			int total = 0;
			for (int dq = 0; dq < scale; dq++) {
				for (int dr = 0; dr < scale; dr++) {
					const Ref<MicroCoord> mc = MicroCoord::make(anchor->get_q() + dq, anchor->get_r() + dr, 1);
					const Ref<WorldCellCoord> owner = mapping->micro_cell_to_world_cell(mc);
					if (!owner->equals(wc) || !p_view->has_cell_data(mc)) {
						continue;
					}
					total++;
					Dictionary view = p_view->get_cell_view(mc);
					const PackedStringArray surfaces = view.get(StringName("surfaces"), PackedStringArray());
					for (int s = 0; s < surfaces.size(); s++) {
						const int idx = p_run->get_tables()->find_surface(surfaces[s]);
						if (idx >= 0 && p_run->get_tables()->get_surface(idx)->has_tag(StringName("water"))) {
							water++;
							break;
						}
					}
				}
			}
			return total > 0 ? (double)water / (double)total : 0.0;
		};
		const double before = coverage(p_overlay->base_view);
		const double after = coverage(p_run->get_committed_snapshot());
		const bool crossed = (before < threshold && after >= threshold) || (before >= threshold && after < threshold);
		if (crossed) {
			summary_dirty.append(wc);
			// v1 summary derivation: threshold crossing marks the world cell's
			// summary stale; the authoritative recompute heuristic is the
			// extension point declared in dual-map-v1-scope §留空清单.
		}
	}
	report[StringName("committed")] = committed;
	report[StringName("rejected")] = rejected;
	report[StringName("summary_dirty")] = summary_dirty;
	return report;
}
