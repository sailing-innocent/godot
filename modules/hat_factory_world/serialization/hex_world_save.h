/**************************************************************************/
/*  hex_world_save.h                                                      */
/*  Hat Factory World module — save/load (MAP-43).                        */
/*                                                                        */
/*  manifest(schema/generator/template/checksum/seed/last_seq) + per-shard */
/*  pages + global event journal. Writes go to a temp file then atomically */
/*  replace; crashes recover the last committed seq. Template schema        */
/*  incompatibility is refused (migration tools, Phase 7+).                 */
/**************************************************************************/
#ifndef HEX_WORLD_SAVE_H
#define HEX_WORLD_SAVE_H

#include "../runtime/hex_world_run.h"

#include "core/object/ref_counted.h"

class HexWorldSave : public RefCounted {
	GDCLASS(HexWorldSave, RefCounted)
public:
	static constexpr int SAVE_SCHEMA_VERSION = 1;

	/** Returns OK or an Error code. */
	static int save_run(const Ref<HexWorldRun> &p_run, const String &p_path);
	/**
	 * Loads a run. Returns null on missing file, corruption, checksum
	 * mismatch or template/schema incompatibility (error printed).
	 */
	static Ref<HexWorldRun> load_run(const String &p_path, const Ref<HexWorldTemplate> &p_world_template, const TypedArray<HexDetailTemplate> &p_detail_templates, const Ref<HexTerrainTableSet> &p_tables);

protected:
	static void _bind_methods();
};

#endif // HEX_WORLD_SAVE_H
