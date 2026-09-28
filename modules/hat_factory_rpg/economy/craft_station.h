#ifndef HAT_FACTORY_RPG_CRAFT_STATION_H
#define HAT_FACTORY_RPG_CRAFT_STATION_H

#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

#include "modules/hat_factory_rpg/economy/craft_recipe.h"
class Backpack;
class CurrencyWallet;
class ItemInstance;

class CraftStation : public RefCounted {
	GDCLASS(CraftStation, RefCounted)

	TypedArray<CraftRecipe> recipes;
	Ref<Backpack> backpack;
	Ref<CurrencyWallet> wallet;

protected:
	static void _bind_methods();

public:
	void set_recipes(const TypedArray<CraftRecipe> &p_recipes) { recipes = p_recipes; }
	TypedArray<CraftRecipe> get_recipes() const { return recipes; }

	void set_backpack(const Ref<Backpack> &p_backpack) { backpack = p_backpack; }
	Ref<Backpack> get_backpack() const { return backpack; }

	void set_wallet(const Ref<CurrencyWallet> &p_wallet) { wallet = p_wallet; }
	Ref<CurrencyWallet> get_wallet() const { return wallet; }

	bool can_craft(const Ref<CraftRecipe> &p_recipe) const;
	Ref<ItemInstance> craft(const Ref<CraftRecipe> &p_recipe);
};

#endif // HAT_FACTORY_RPG_CRAFT_STATION_H
