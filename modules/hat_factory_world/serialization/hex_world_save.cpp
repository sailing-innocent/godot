/**************************************************************************/
/*  hex_world_save.cpp                                                    */
/**************************************************************************/
#include "hex_world_save.h"

#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/object/class_db.h"

void HexWorldSave::_bind_methods() {
	ClassDB::bind_static_method("HexWorldSave", D_METHOD("save_run", "run", "path"), &HexWorldSave::save_run);
	ClassDB::bind_static_method("HexWorldSave", D_METHOD("load_run", "path", "world_template", "detail_templates", "tables"), &HexWorldSave::load_run);
}

int HexWorldSave::save_run(const Ref<HexWorldRun> &p_run, const String &p_path) {
	ERR_FAIL_COND_V(p_run.is_null(), ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_path.is_empty(), ERR_INVALID_PARAMETER);

	Dictionary root;
	root[StringName("save_schema")] = SAVE_SCHEMA_VERSION;
	root[StringName("manifest")] = p_run->get_manifest_dict();
	root[StringName("snapshot")] = p_run->get_committed_snapshot()->to_dict();
	if (p_run->get_journal().is_valid()) {
		root[StringName("journal")] = p_run->get_journal()->to_dict();
	}
	Dictionary features;
	const HashMap<int64_t, Dictionary> feats = p_run->_get_features();
	for (const KeyValue<int64_t, Dictionary> &kv : feats) {
		features[kv.key] = kv.value;
	}
	root[StringName("features")] = features;
	root[StringName("next_feature_seq")] = p_run->_get_next_feature_seq();

	const String json = JSON::stringify(root, "  ");

	// Crash-safe: write temp, atomically replace the previous committed file.
	const String tmp_path = p_path + ".tmp";
	{
		Ref<FileAccess> f = FileAccess::open(tmp_path, FileAccess::WRITE);
		ERR_FAIL_COND_V(f.is_null(), FileAccess::get_open_error());
		f->store_string(json);
		f->flush();
		const Error close_err = f->get_error() == OK ? OK : ERR_FILE_CANT_WRITE;
		f.unref();
		ERR_FAIL_COND_V(close_err != OK, close_err);
	}

	Error rename_err = DirAccess::rename_absolute(tmp_path, p_path);
	if (rename_err != OK) {
		// Rename may fail on some platforms when the target exists; retry once.
		DirAccess::remove_absolute(p_path);
		rename_err = DirAccess::rename_absolute(tmp_path, p_path);
	}
	if (rename_err != OK) {
		// Keep the temp file so no committed data is lost.
		ERR_PRINT("HexWorldSave: atomic replace failed for " + p_path + " (temp kept)");
		return rename_err;
	}
	return OK;
}

Ref<HexWorldRun> HexWorldSave::load_run(const String &p_path, const Ref<HexWorldTemplate> &p_world_template, const TypedArray<HexDetailTemplate> &p_detail_templates, const Ref<HexTerrainTableSet> &p_tables) {
	String load_path = p_path;
	if (!FileAccess::exists(load_path)) {
		// Crash recovery: a previous save may have died between temp write
		// and atomic replace; the temp holds the last fully written payload.
		if (FileAccess::exists(p_path + ".tmp")) {
			load_path = p_path + ".tmp";
			WARN_PRINT("HexWorldSave: recovering from temp file " + load_path);
		} else {
			ERR_PRINT("HexWorldSave: save file not found: " + p_path);
			return Ref<HexWorldRun>();
		}
	}
	Ref<FileAccess> f = FileAccess::open(load_path, FileAccess::READ);
	ERR_FAIL_COND_V(f.is_null(), Ref<HexWorldRun>());
	const String json = f->get_as_text();
	f.unref();

	Ref<JSON> parser;
	parser.instantiate();
	const Error parse_err = parser->parse(json);
	if (parse_err != OK || parser->get_data().get_type() != Variant::DICTIONARY) {
		ERR_PRINT("HexWorldSave: corrupt save (JSON parse error at line " + itos(parser->get_error_line()) + ": " + parser->get_error_message() + "); keeping previous state unread");
		return Ref<HexWorldRun>();
	}
	Dictionary root = parser->get_data();
	const int save_schema = root.get(StringName("save_schema"), 0);
	if (save_schema != SAVE_SCHEMA_VERSION) {
		ERR_PRINT("HexWorldSave: incompatible save schema " + itos(save_schema) + " (want " + itos(SAVE_SCHEMA_VERSION) + "); refusing to load");
		return Ref<HexWorldRun>();
	}
	Dictionary manifest = root.get(StringName("manifest"), Dictionary());
	ERR_FAIL_COND_V_MSG(manifest.is_empty(), Ref<HexWorldRun>(), "HexWorldSave: manifest missing");
	// Template identity + version gate (no silent replay of old seeds).
	ERR_FAIL_COND_V_MSG(p_world_template.is_null(), Ref<HexWorldRun>(), "HexWorldSave: world template required for load");
	const StringName map_id = manifest.get(StringName("map_id"), StringName());
	if (map_id != p_world_template->get_map_id()) {
		ERR_PRINT("HexWorldSave: map_id mismatch '" + String(map_id) + "' != '" + String(p_world_template->get_map_id()) + "'");
		return Ref<HexWorldRun>();
	}
	Dictionary version_dict = manifest.get(StringName("template_version"), Dictionary());
	Ref<TemplateVersion> saved_version = TemplateVersion::from_dict(version_dict);
	Ref<TemplateVersion> current_version = p_world_template->get_template_version();
	if (current_version.is_null() || !saved_version->is_compatible_with(current_version)) {
		ERR_PRINT("HexWorldSave: template schema incompatible; migration required — refusing to load");
		return Ref<HexWorldRun>();
	}

	Ref<HexWorldRun> run = HexWorldRun::_create_for_load(p_world_template, p_detail_templates, p_tables, manifest.get(StringName("seed"), (int64_t)0));
	ERR_FAIL_COND_V(run.is_null(), Ref<HexWorldRun>());

	Ref<HexRegionSnapshot> snapshot = HexRegionSnapshot::from_dict(root.get(StringName("snapshot"), Dictionary()), p_tables);
	ERR_FAIL_COND_V(snapshot.is_null(), Ref<HexWorldRun>());
	const int64_t last_seq = manifest.get(StringName("last_seq"), (int64_t)0);
	if (snapshot->get_version() != last_seq) {
		ERR_PRINT("HexWorldSave: snapshot version != manifest last_seq; save is inconsistent — refusing to load");
		return Ref<HexWorldRun>();
	}

	Ref<HexEventJournal> journal = HexEventJournal::from_dict(root.get(StringName("journal"), Dictionary()));
	HashMap<int64_t, Dictionary> features;
	Dictionary feats = root.get(StringName("features"), Dictionary());
	const LocalVector<Variant> keys = feats.get_key_list();
	for (const Variant &k : keys) {
		features[(int64_t)k] = feats[k];
	}
	run->_restore_committed(run, snapshot, last_seq, journal, features, root.get(StringName("next_feature_seq"), (int64_t)1));

	// Corruption smoke check: dynamic checksum must match the manifest.
	const Dictionary recomputed = run->get_manifest_dict();
	if ((int64_t)recomputed.get(StringName("checksum"), (int64_t)-1) != (int64_t)manifest.get(StringName("checksum"), (int64_t)-2)) {
		ERR_PRINT("HexWorldSave: checksum mismatch; save corrupted — refusing to load");
		return Ref<HexWorldRun>();
	}
	return run;
}
