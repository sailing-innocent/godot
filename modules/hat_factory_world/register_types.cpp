/**************************************************************************/
/*  register_types.cpp                                                    */
/*  Hat Factory World module                                              */
/**************************************************************************/
#include "register_types.h"

#include "core/object/class_db.h"

#include "data/hex_detail_template.h"
#include "data/hex_movement_profile.h"
#include "data/hex_tag_modifier_table.h"
#include "data/hex_terrain_layer_defs.h"
#include "data/hex_terrain_table_set.h"
#include "data/hex_verb_def.h"
#include "data/hex_world_template.h"
#include "combat/hex_encounter_bounds.h"
#include "combat/hex_encounter_overlay.h"
#include "combat/hex_writeback.h"
#include "query/hex_occupancy.h"
#include "query/hex_spatial_query.h"
#include "runtime/hex_command_resolver.h"
#include "runtime/hex_event_journal.h"
#include "runtime/hex_map_command.h"
#include "runtime/hex_region_snapshot.h"
#include "runtime/hex_world_run.h"
#include "serialization/hex_world_save.h"
#include "world/hex_edge_contract.h"
#include "world/hex_scale_mapping.h"
#include "world/world_ids.h"

void initialize_hat_factory_world_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Identity layer (Phase 1).
		GDREGISTER_CLASS(MapId);
		GDREGISTER_CLASS(TemplateVersion);
		GDREGISTER_CLASS(WorldCellCoord);
		GDREGISTER_CLASS(MicroCoord);
		GDREGISTER_CLASS(FeatureId);
		GDREGISTER_CLASS(EdgeKey);
		// Scale mapping + boundary contracts (Phase 1).
		GDREGISTER_CLASS(DetailMapDescriptor);
		GDREGISTER_CLASS(HexScaleMapping);
		GDREGISTER_CLASS(HexEdgeContract);
		// Terrain tables + templates (Phase 2).
		GDREGISTER_CLASS(HexLayerDefBase);
		GDREGISTER_CLASS(HexBaseTerrainDef);
		GDREGISTER_CLASS(HexSurfaceDef);
		GDREGISTER_CLASS(HexObjectDef);
		GDREGISTER_CLASS(HexEdgeTypeDef);
		GDREGISTER_CLASS(HexAreaEffectDef);
		GDREGISTER_CLASS(HexReactionDef);
		GDREGISTER_CLASS(HexMovementProfile);
		GDREGISTER_CLASS(HexVerbDef);
		GDREGISTER_CLASS(HexTagModifierTable);
		GDREGISTER_CLASS(HexTerrainTableSet);
		GDREGISTER_CLASS(HexWorldCellDef);
		GDREGISTER_CLASS(HexWorldPortalDef);
		GDREGISTER_CLASS(HexWorldTemplate);
		GDREGISTER_CLASS(HexDetailCellDef);
		GDREGISTER_CLASS(HexDetailTemplate);
		// Runtime: commands, snapshots, run, resolver, journal (Phase 3).
		GDREGISTER_CLASS(HexMapCommand);
		GDREGISTER_CLASS(HexEventJournal);
		GDREGISTER_CLASS(HexRegionSnapshot);
		GDREGISTER_CLASS(HexWorldRun);
		GDREGISTER_CLASS(HexCommandResult);
		GDREGISTER_CLASS(HexCommandResolver);
		GDREGISTER_CLASS(HexWorldSave);
		// Unified spatial query (Phase 4).
		GDREGISTER_CLASS(HexOccupancySnapshot);
		GDREGISTER_CLASS(HexQueryStep);
		GDREGISTER_CLASS(HexPathResult);
		GDREGISTER_CLASS(HexSpatialQuery);
		// Battle expansion + writeback (Phase 5).
		GDREGISTER_CLASS(HexEncounterBounds);
		GDREGISTER_CLASS(HexEncounterOverlay);
		GDREGISTER_CLASS(HexWriteback);
	}
}

void uninitialize_hat_factory_world_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
