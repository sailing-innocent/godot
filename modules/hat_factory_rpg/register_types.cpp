#include "register_types.h"

#include "core/object/class_db.h"

#include "item/item_def.h"
#include "item/item_instance.h"
#include "item/item_effect.h"
#include "item/enchantment.h"

#include "inventory/inventory_slot.h"
#include "inventory/backpack.h"
#include "inventory/item_action_resolver.h"

#include "equipment/equipment_set.h"
#include "equipment/affix_def.h"

#include "economy/currency_wallet.h"
#include "economy/shop.h"
#include "economy/shop_catalog.h"
#include "economy/craft_station.h"

#include "serialization/rpg_snapshot.h"

void initialize_hat_factory_rpg_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(ItemDef);
		GDREGISTER_CLASS(ItemInstance);
		GDREGISTER_CLASS(ItemEffect);
		GDREGISTER_CLASS(Enchantment);

		GDREGISTER_CLASS(InventorySlot);
		GDREGISTER_CLASS(Backpack);
		GDREGISTER_CLASS(ItemActionResolver);

		GDREGISTER_CLASS(EquipmentSet);
		GDREGISTER_CLASS(AffixDef);

		GDREGISTER_CLASS(CurrencyWallet);
		GDREGISTER_CLASS(Shop);
		GDREGISTER_CLASS(ShopCatalog);
		GDREGISTER_CLASS(CraftRecipe);
		GDREGISTER_CLASS(CraftStation);

		GDREGISTER_CLASS(RPGSnapshot);
	}
}

void uninitialize_hat_factory_rpg_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
}
