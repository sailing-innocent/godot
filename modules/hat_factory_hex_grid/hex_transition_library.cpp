#include "hex_transition_library.h"

#include "core/object/class_db.h"
#include "core/math/math_funcs.h"

void HexTransitionLibrary::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_definitions", "definitions"), &HexTransitionLibrary::set_definitions);
	ClassDB::bind_method(D_METHOD("get_definitions"), &HexTransitionLibrary::get_definitions);

	ClassDB::bind_method(D_METHOD("add_definition", "definition"), &HexTransitionLibrary::add_definition);
	ClassDB::bind_method(D_METHOD("remove_definition", "definition"), &HexTransitionLibrary::remove_definition);
	ClassDB::bind_method(D_METHOD("find_rule", "source", "target", "height_delta", "terrain_library"), &HexTransitionLibrary::find_rule);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "definitions", PROPERTY_HINT_ARRAY_TYPE, "HexTransitionDef"), "set_definitions", "get_definitions");

	ADD_SIGNAL(MethodInfo("definitions_changed"));
}

void HexTransitionLibrary::set_definitions(const TypedArray<HexTransitionDef> &p_definitions) {
	definitions = p_definitions;
	emit_signal(SNAME("definitions_changed"));
	emit_changed();
}

TypedArray<HexTransitionDef> HexTransitionLibrary::get_definitions() const {
	return definitions;
}

void HexTransitionLibrary::add_definition(const Ref<HexTransitionDef> &p_def) {
	if (p_def.is_null()) {
		return;
	}
	definitions.append(p_def);
	emit_signal(SNAME("definitions_changed"));
	emit_changed();
}

void HexTransitionLibrary::remove_definition(const Ref<HexTransitionDef> &p_def) {
	int idx = definitions.find(p_def);
	if (idx >= 0) {
		definitions.remove_at(idx);
		emit_signal(SNAME("definitions_changed"));
		emit_changed();
	}
}

bool HexTransitionLibrary::_tags_match(const PackedStringArray &p_rule_tags, const PackedStringArray &p_terrain_tags) {
	if (p_rule_tags.is_empty()) {
		return true;
	}
	for (int i = 0; i < p_rule_tags.size(); i++) {
		const String &tag = p_rule_tags[i];
		for (int j = 0; j < p_terrain_tags.size(); j++) {
			if (p_terrain_tags[j] == tag) {
				return true;
			}
		}
	}
	return false;
}

Ref<HexTransitionDef> HexTransitionLibrary::find_rule(const Ref<HexCellData> &p_source,
		const Ref<HexCellData> &p_target,
		int p_height_delta,
		const Ref<HexTerrainLibrary> &p_terrain_library) const {
	if (p_source.is_null() || p_target.is_null()) {
		return Ref<HexTransitionDef>();
	}

	StringName source_terrain_id = p_source->get_terrain_id();
	StringName target_terrain_id = p_target->get_terrain_id();

	PackedStringArray source_tags;
	PackedStringArray target_tags;
	if (p_terrain_library.is_valid()) {
		Ref<HexTerrainDef> src_def = p_terrain_library->get_terrain(source_terrain_id);
		if (src_def.is_valid()) {
			source_tags = src_def->get_transition_tags();
		}
		Ref<HexTerrainDef> tgt_def = p_terrain_library->get_terrain(target_terrain_id);
		if (tgt_def.is_valid()) {
			target_tags = tgt_def->get_transition_tags();
		}
	}

	Ref<HexTransitionDef> best;
	int best_priority = INT_MIN;

	for (int i = 0; i < definitions.size(); i++) {
		Ref<HexTransitionDef> def = definitions[i];
		if (def.is_null()) {
			continue;
		}

		// Height delta check.
		if (p_height_delta < def->get_min_height_delta() || p_height_delta > def->get_max_height_delta()) {
			continue;
		}

		// Terrain identity checks.
		if (def->get_require_same_terrain() && source_terrain_id != target_terrain_id) {
			continue;
		}
		if (def->get_require_different_terrain() && source_terrain_id == target_terrain_id) {
			continue;
		}

		// Exact terrain IDs.
		if (!def->get_source_terrain_id().is_empty() && def->get_source_terrain_id() != source_terrain_id) {
			continue;
		}
		if (!def->get_target_terrain_id().is_empty() && def->get_target_terrain_id() != target_terrain_id) {
			continue;
		}

		// Tag checks (only used when no exact ID was specified).
		if (def->get_source_terrain_id().is_empty() && !_tags_match(def->get_source_tags(), source_tags)) {
			continue;
		}
		if (def->get_target_terrain_id().is_empty() && !_tags_match(def->get_target_tags(), target_tags)) {
			continue;
		}

		// Probability.
		if (def->get_probability() < 1.0f && Math::randf() > def->get_probability()) {
			continue;
		}

		if (def->get_priority() > best_priority) {
			best = def;
			best_priority = def->get_priority();
		}
	}

	return best;
}
