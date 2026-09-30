/**************************************************************************/
/*  hex_writeback.h                                                       */
/*  Hat Factory World module — battle delta commit (MAP-04, D3).          */
/*                                                                        */
/*  Strategy matrix (declared on HexEncounterBounds):                     */
/*    victory: commit_all / commit_on_victory -> commit; discard -> none  */
/*    defeat:  commit_all -> commit; commit_on_victory -> defeat_strategy */
/*             (rollback = discard, keep = commit); discard -> none       */
/*  After commit, macro summaries recompute only for portal/coverage      */
/*  changes (micro edits never rewrite whole world bodies of water).      */
/**************************************************************************/
#ifndef HEX_WRITEBACK_H
#define HEX_WRITEBACK_H

#include "hex_encounter_overlay.h"

#include "core/object/ref_counted.h"

class HexCommandResult; // defined in hex_command_resolver.h

class HexWriteback : public RefCounted {
	GDCLASS(HexWriteback, RefCounted)
public:
	struct Summary {
		int committed = 0;
		int rejected = 0;
		TypedArray<WorldCellCoord> summary_dirty_cells; // world cells whose macro summary changed
	};

	/**
	 * Commits overlay deltas to the run according to the declared strategy.
	 * Every delta is re-validated by the resolver; rejections are reported.
	 */
	static Dictionary commit(const Ref<HexWorldRun> &p_run, const Ref<HexEncounterOverlay> &p_overlay, bool p_victory);

protected:
	static void _bind_methods();
};

#endif // HEX_WRITEBACK_H
