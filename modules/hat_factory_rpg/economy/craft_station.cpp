#include "craft_station.h"

#include "craft_recipe.h"
#include "currency_wallet.h"
#include "inventory/backpack.h"
#include "item/item_instance.h"

#include "core/object/class_db.h"
#include "core/string/string_name.h"

void CraftStation::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_recipes", "recipes"), &CraftStation::set_recipes);
	ClassDB::bind_method(D_METHOD("get_recipes"), &CraftStation::get_recipes);

	ClassDB::bind_method(D_METHOD("set_backpack", "backpack"), &CraftStation::set_backpack);
	ClassDB::bind_method(D_METHOD("get_backpack"), &CraftStation::get_backpack);

	ClassDB::bind_method(D_METHOD("set_wallet", "wallet"), &CraftStation::set_wallet);
	ClassDB::bind_method(D_METHOD("get_wallet"), &CraftStation::get_wallet);

	ClassDB::bind_method(D_METHOD("can_craft", "recipe"), &CraftStation::can_craft);
	ClassDB::bind_method(D_METHOD("craft", "recipe"), &CraftStation::craft);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "recipes", PROPERTY_HINT_ARRAY_TYPE, "CraftRecipe"), "set_recipes", "get_recipes");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "backpack", PROPERTY_HINT_RESOURCE_TYPE, "Backpack"), "set_backpack", "get_backpack");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "wallet", PROPERTY_HINT_RESOURCE_TYPE, "CurrencyWallet"), "set_wallet", "get_wallet");
}

bool CraftStation::can_craft(const Ref<CraftRecipe> &p_recipe) const {
	if (p_recipe.is_null()) {
		return false;
	}

	Dictionary materials = p_recipe->get_materials();
	Array keys = materials.keys();
	for (int i = 0; i < keys.size(); ++i) {
		const StringName item_id = keys[i];
		const int required = materials[item_id];
		if (backpack.is_null() || backpack->count(item_id) < required) {
			return false;
		}
	}

	if (wallet.is_null() || !wallet->can_afford(p_recipe->get_currency(), p_recipe->get_currency_cost())) {
		return false;
	}

	return true;
}

Ref<ItemInstance> CraftStation::craft(const Ref<CraftRecipe> &p_recipe) {
	if (!can_craft(p_recipe)) {
		return Ref<ItemInstance>();
	}

	Dictionary materials = p_recipe->get_materials();
	Array keys = materials.keys();
	for (int i = 0; i < keys.size(); ++i) {
		const StringName item_id = keys[i];
		const int required = materials[item_id];
		if (!backpack->remove(item_id, required)) {
			return Ref<ItemInstance>();
		}
	}

	if (!wallet->spend(p_recipe->get_currency(), p_recipe->get_currency_cost())) {
		return Ref<ItemInstance>();
	}

	Ref<ItemInstance> product = p_recipe->get_product();
	if (product.is_null()) {
		return Ref<ItemInstance>();
	}

	Ref<ItemInstance> result = product->clone();
	return result;
}
