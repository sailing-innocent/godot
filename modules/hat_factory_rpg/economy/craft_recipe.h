#ifndef HAT_FACTORY_RPG_CRAFT_RECIPE_H
#define HAT_FACTORY_RPG_CRAFT_RECIPE_H

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"

#include "modules/hat_factory_rpg/item/item_instance.h"

class CraftRecipe : public Resource {
	GDCLASS(CraftRecipe, Resource)

	StringName recipe_id;
	Dictionary materials; // item_id -> count
	int currency = 0;
	int currency_cost = 0;
	Ref<ItemInstance> product;

protected:
	static void _bind_methods();

public:
	void set_recipe_id(const StringName &p_id) { recipe_id = p_id; }
	StringName get_recipe_id() const { return recipe_id; }

	void set_materials(const Dictionary &p_materials) { materials = p_materials; }
	Dictionary get_materials() const { return materials; }

	void set_currency(int p_currency) { currency = p_currency; }
	int get_currency() const { return currency; }

	void set_currency_cost(int p_cost) { currency_cost = p_cost; }
	int get_currency_cost() const { return currency_cost; }

	void set_product(const Ref<ItemInstance> &p_product) { product = p_product; }
	Ref<ItemInstance> get_product() const { return product; }

	Dictionary to_dict() const;
	void from_dict(const Dictionary &p_dict);
	Ref<CraftRecipe> clone() const;
};

#endif // HAT_FACTORY_RPG_CRAFT_RECIPE_H
