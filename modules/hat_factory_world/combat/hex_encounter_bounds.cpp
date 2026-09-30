/**************************************************************************/
/*  hex_encounter_bounds.cpp                                              */
/**************************************************************************/
#include "hex_encounter_bounds.h"

#include "core/object/class_db.h"

void HexEncounterBounds::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_bounds_kind", "kind"), &HexEncounterBounds::set_bounds_kind);
	ClassDB::bind_method(D_METHOD("get_bounds_kind"), &HexEncounterBounds::get_bounds_kind);
	ClassDB::bind_method(D_METHOD("set_center", "center"), &HexEncounterBounds::set_center);
	ClassDB::bind_method(D_METHOD("get_center"), &HexEncounterBounds::get_center);
	ClassDB::bind_method(D_METHOD("set_world_radius", "radius"), &HexEncounterBounds::set_world_radius);
	ClassDB::bind_method(D_METHOD("get_world_radius"), &HexEncounterBounds::get_world_radius);
	ClassDB::bind_method(D_METHOD("set_explicit_cells", "cells"), &HexEncounterBounds::set_explicit_cells);
	ClassDB::bind_method(D_METHOD("get_explicit_cells"), &HexEncounterBounds::get_explicit_cells);
	ClassDB::bind_method(D_METHOD("set_deployment_zone", "cells"), &HexEncounterBounds::set_deployment_zone);
	ClassDB::bind_method(D_METHOD("get_deployment_zone"), &HexEncounterBounds::get_deployment_zone);
	ClassDB::bind_method(D_METHOD("set_participants", "participants"), &HexEncounterBounds::set_participants);
	ClassDB::bind_method(D_METHOD("get_participants"), &HexEncounterBounds::get_participants);
	ClassDB::bind_method(D_METHOD("get_participant_count"), &HexEncounterBounds::get_participant_count);
	ClassDB::bind_method(D_METHOD("set_commit_strategy", "strategy"), &HexEncounterBounds::set_commit_strategy);
	ClassDB::bind_method(D_METHOD("get_commit_strategy"), &HexEncounterBounds::get_commit_strategy);
	ClassDB::bind_method(D_METHOD("set_defeat_strategy", "strategy"), &HexEncounterBounds::set_defeat_strategy);
	ClassDB::bind_method(D_METHOD("get_defeat_strategy"), &HexEncounterBounds::get_defeat_strategy);
	ClassDB::bind_method(D_METHOD("set_coverage_summary_threshold", "threshold"), &HexEncounterBounds::set_coverage_summary_threshold);
	ClassDB::bind_method(D_METHOD("get_coverage_summary_threshold"), &HexEncounterBounds::get_coverage_summary_threshold);
	ClassDB::bind_method(D_METHOD("validate"), &HexEncounterBounds::validate);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "bounds_kind"), "set_bounds_kind", "get_bounds_kind");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "center", PROPERTY_HINT_RESOURCE_TYPE, "WorldCellCoord"), "set_center", "get_center");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "world_radius"), "set_world_radius", "get_world_radius");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "explicit_cells", PROPERTY_HINT_ARRAY_TYPE, "WorldCellCoord"), "set_explicit_cells", "get_explicit_cells");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "deployment_zone", PROPERTY_HINT_ARRAY_TYPE, "WorldCellCoord"), "set_deployment_zone", "get_deployment_zone");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "participants"), "set_participants", "get_participants");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "commit_strategy"), "set_commit_strategy", "get_commit_strategy");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "defeat_strategy"), "set_defeat_strategy", "get_defeat_strategy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "coverage_summary_threshold"), "set_coverage_summary_threshold", "get_coverage_summary_threshold");
}

void HexEncounterBounds::set_bounds_kind(const StringName &p_kind) { bounds_kind = p_kind; }
StringName HexEncounterBounds::get_bounds_kind() const { return bounds_kind; }
void HexEncounterBounds::set_center(const Ref<WorldCellCoord> &p_center) { center = p_center; }
Ref<WorldCellCoord> HexEncounterBounds::get_center() const { return center; }
void HexEncounterBounds::set_world_radius(int p_radius) { world_radius = p_radius; }
int HexEncounterBounds::get_world_radius() const { return world_radius; }
void HexEncounterBounds::set_explicit_cells(const TypedArray<WorldCellCoord> &p_cells) { explicit_cells = p_cells; }
TypedArray<WorldCellCoord> HexEncounterBounds::get_explicit_cells() const { return explicit_cells; }
void HexEncounterBounds::set_deployment_zone(const TypedArray<WorldCellCoord> &p_cells) { deployment_zone = p_cells; }
TypedArray<WorldCellCoord> HexEncounterBounds::get_deployment_zone() const { return deployment_zone; }
void HexEncounterBounds::set_participants(const Dictionary &p_participants) { participants = p_participants; }
Dictionary HexEncounterBounds::get_participants() const { return participants; }
int HexEncounterBounds::get_participant_count() const { return participants.size(); }
void HexEncounterBounds::set_commit_strategy(const StringName &p_strategy) { commit_strategy = p_strategy; }
StringName HexEncounterBounds::get_commit_strategy() const { return commit_strategy; }
void HexEncounterBounds::set_defeat_strategy(const StringName &p_strategy) { defeat_strategy = p_strategy; }
StringName HexEncounterBounds::get_defeat_strategy() const { return defeat_strategy; }
void HexEncounterBounds::set_coverage_summary_threshold(double p_threshold) { coverage_summary_threshold = p_threshold; }
double HexEncounterBounds::get_coverage_summary_threshold() const { return coverage_summary_threshold; }

String HexEncounterBounds::validate() const {
	if (bounds_kind != StringName("radius") && bounds_kind != StringName("cells")) {
		return "bounds_kind must be 'radius' or 'cells'";
	}
	if (bounds_kind == StringName("radius") && center.is_null()) {
		return "radius bounds require a center world cell";
	}
	if (world_radius < 0) {
		return "world_radius must be >= 0";
	}
	if (bounds_kind == StringName("cells") && explicit_cells.is_empty()) {
		return "cells bounds require explicit_cells";
	}
	if (commit_strategy != StringName("commit_all") && commit_strategy != StringName("commit_on_victory") && commit_strategy != StringName("discard")) {
		return "unknown commit_strategy '" + String(commit_strategy) + "'";
	}
	if (defeat_strategy != StringName("rollback") && defeat_strategy != StringName("keep")) {
		return "unknown defeat_strategy '" + String(defeat_strategy) + "'";
	}
	return String();
}
