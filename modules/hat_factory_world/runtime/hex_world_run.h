/**************************************************************************/
/*  hex_world_run.h                                                       */
/*  Hat Factory World module — the one writable authority (MAP-40/42/43).*/
/*                                                                        */
/*  Run = world template version + seed + versioned shard deltas + global */
/*  sequence + stable feature registry + event journal. Templates are     */
/*  immutable (Invariant 4); all mutations flow through the command       */
/*  resolver (Invariant 3).                                               */
/**************************************************************************/
#ifndef HEX_WORLD_RUN_H
#define HEX_WORLD_RUN_H

#include "../data/hex_detail_template.h"
#include "../data/hex_terrain_table_set.h"
#include "../data/hex_world_template.h"
#include "hex_event_journal.h"
#include "hex_map_command.h"
#include "hex_region_snapshot.h"
#include "hex_shard_types.h"

#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"

class HexCommandResult; // defined in hex_command_resolver.h

class HexWorldRun : public RefCounted {
	GDCLASS(HexWorldRun, RefCounted)
public:
	static constexpr int SCHEMA_VERSION = 1;
	static constexpr int GENERATOR_VERSION = 1;

private:
	Ref<HexWorldTemplate> world_template;
	TypedArray<HexDetailTemplate> detail_templates;
	Ref<HexTerrainTableSet> tables;
	int64_t seed = 0;
	int64_t global_seq = 0;
	int64_t next_feature_seq = 1;

	HashMap<uint64_t, Ref<hf_world::HexShardState>> base_shards; // immutable materialization
	Ref<HexRegionSnapshot> committed;
	Ref<HexEventJournal> journal;
	HashMap<int64_t, Dictionary> features; // stable registry: seq -> {kind, coord dict}

public:
	static Ref<HexWorldRun> create(const Ref<HexWorldTemplate> &p_world_template, const TypedArray<HexDetailTemplate> &p_detail_templates, const Ref<HexTerrainTableSet> &p_tables, int64_t p_seed);
	/** 最近一次 create 失败的精确原因（探针，供无控制台的设备端呈现）。 */
	static String get_last_create_error();

	void set_seed(int64_t p_seed);
	int64_t get_seed() const;
	Ref<HexWorldTemplate> get_world_template() const;
	Ref<HexTerrainTableSet> get_tables() const;
	Ref<HexEventJournal> get_journal() const;
	StringName get_map_id() const;
	Ref<TemplateVersion> get_template_version() const;
	int64_t get_global_seq() const;

	Ref<hf_world::HexShardState> get_base_shard_state(uint64_t p_key) const;
	Ref<HexRegionSnapshot> get_committed_snapshot() const;
	/** Battle snapshot: shallow structural-sharing clone of the committed view. */
	Ref<HexRegionSnapshot> new_battle_snapshot() const;

	/**
	 * Validates and applies a command against the committed snapshot.
	 * On acceptance: commits the next snapshot, bumps global_seq, appends the
	 * journal. On rejection: state untouched, reason in the result.
	 */
	Ref<HexCommandResult> apply_command(const Ref<HexMapCommand> &p_command);

	// Stable feature registry (MAP-06: IDs never change on stream/rebuild).
	int64_t alloc_feature_seq();
	bool register_feature(int64_t p_seq, const StringName &p_kind, const Ref<MicroCoord> &p_coord);
	bool move_feature(int64_t p_seq, const Ref<MicroCoord> &p_coord);
	bool unregister_feature(int64_t p_seq);
	Dictionary get_feature(int64_t p_seq) const;

	/** Save manifest contents (schema/generator/template/checksum/seed/seq). */
	Dictionary get_manifest_dict() const;

	// Restore path used by HexWorldSave (no public binding needed).
	void _restore_committed(const Ref<HexWorldRun> &p_self, const Ref<HexRegionSnapshot> &p_snapshot, int64_t p_global_seq, const Ref<HexEventJournal> &p_journal, const HashMap<int64_t, Dictionary> &p_features, int64_t p_next_feature_seq);
	static Ref<HexWorldRun> _create_for_load(const Ref<HexWorldTemplate> &p_world_template, const TypedArray<HexDetailTemplate> &p_detail_templates, const Ref<HexTerrainTableSet> &p_tables, int64_t p_seed);
	/** Friend-style hooks for HexWorldSave. */
	void _set_detail_templates(const TypedArray<HexDetailTemplate> &p_templates) { detail_templates = p_templates; }
	TypedArray<HexDetailTemplate> _get_detail_templates() const { return detail_templates; }
	HashMap<int64_t, Dictionary> _get_features() const {
		HashMap<int64_t, Dictionary> out;
		for (const KeyValue<int64_t, Dictionary> &kv : features) {
			out.insert(kv.key, kv.value);
		}
		return out;
	}
	int64_t _get_next_feature_seq() const { return next_feature_seq; }

protected:
	static void _bind_methods();
	bool _materialize_base(String &r_error);
};

#endif // HEX_WORLD_RUN_H
