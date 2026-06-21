/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/

#include "register_types.h"

#include "core/object/class_db.h"
#include "hex_cell_data.h"
#include "hex_cell_effect.h"
#include "hex_terrain_def.h"
#include "hex_terrain_library.h"
#include "hex_grid_map_data.h"
#include "hex_grid_map.h"

void initialize_hat_factory_hex_grid_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(HexCellEffect);
		GDREGISTER_CLASS(HexCellData);
		GDREGISTER_CLASS(HexTerrainDef);
		GDREGISTER_CLASS(HexTerrainLibrary);
		GDREGISTER_CLASS(HexGridMapData);
		GDREGISTER_CLASS(HexGridMap);
	}
}

void uninitialize_hat_factory_hex_grid_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
