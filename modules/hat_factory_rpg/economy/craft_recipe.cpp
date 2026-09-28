#include "craft_recipe.h"

#include "item/item_instance.h"

#include "core/object/class_db.h"

void CraftRecipe::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_recipe_id", "recipe_id"), &CraftRecipe::set_recipe_id);
	ClassDB::bind_method(D_METHOD("get_recipe_id"), &CraftRecipe::get_recipe_id);

	ClassDB::bind_method(D_METHOD("set_materials", "materials"), &CraftRecipe::set_materials);
	ClassDB::bind_method(D_METHOD("get_materials"), &CraftRecipe::get_materials);

	ClassDB::bind_method(D_METHOD("set_currency", "currency"), &CraftRecipe::set_currency);
	ClassDB::bind_method(D_METHOD("get_currency"), &CraftRecipe::get_currency);

	ClassDB::bind_method(D_METHOD("set_currency_cost", "currency_cost"), &CraftRecipe::set_currency_cost);
	ClassDB::bind_method(D_METHOD("get_currency_cost"), &CraftRecipe::get_currency_cost);

	ClassDB::bind_method(D_METHOD("set_product", "product"), &CraftRecipe::set_product);
	ClassDB::bind_method(D_METHOD("get_product"), &CraftRecipe::get_product);

	ClassDB::bind_method(D_METHOD("to_dict"), &CraftRecipe::to_dict);
	ClassDB::bind_method(D_METHOD("from_dict", "dict"), &CraftRecipe::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &CraftRecipe::clone);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "recipe_id"), "set_recipe_id", "get_recipe_id");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "materials"), "set_materials", "get_materials");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "currency"), "set_currency", "get_currency");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "currency_cost"), "set_currency_cost", "get_currency_cost");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "product", PROPERTY_HINT_RESOURCE_TYPE, "ItemInstance"), "set_product", "get_product");
}

Dictionary CraftRecipe::to_dict() const {
	Dictionary d;
	d["recipe_id"] = recipe_id;
	d["materials"] = materials;
	d["currency"] = currency;
	d["currency_cost"] = currency_cost;
	if (product.is_valid()) {
		d["product"] = product->to_dict();
	} else {
		d["product"] = Dictionary();
	}
	return d;
}

void CraftRecipe::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("recipe_id")) {
		recipe_id = p_dict["recipe_id"];
	}
	if (p_dict.has("materials")) {
		materials = p_dict["materials"];
	}
	if (p_dict.has("currency")) {
		currency = p_dict["currency"];
	}
	if (p_dict.has("currency_cost")) {
		currency_cost = p_dict["currency_cost"];
	}
	if (p_dict.has("product")) {
		Variant prod_var = p_dict["product"];
		if (prod_var.get_type() == Variant::DICTIONARY) {
			Dictionary prod_dict = prod_var;
			if (!prod_dict.is_empty()) {
				product = ItemInstance::from_dict(prod_dict);
			}
		}
	}
}

Ref<CraftRecipe> CraftRecipe::clone() const {
	Ref<CraftRecipe> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
