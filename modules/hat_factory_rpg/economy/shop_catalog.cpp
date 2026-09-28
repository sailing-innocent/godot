#include "shop_catalog.h"

#include "shop_entry.h"

#include "core/object/class_db.h"

void ShopCatalog::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shop_id", "shop_id"), &ShopCatalog::set_shop_id);
	ClassDB::bind_method(D_METHOD("get_shop_id"), &ShopCatalog::get_shop_id);

	ClassDB::bind_method(D_METHOD("set_entries", "entries"), &ShopCatalog::set_entries);
	ClassDB::bind_method(D_METHOD("get_entries"), &ShopCatalog::get_entries);

	ClassDB::bind_method(D_METHOD("set_refresh_cost", "refresh_cost"), &ShopCatalog::set_refresh_cost);
	ClassDB::bind_method(D_METHOD("get_refresh_cost"), &ShopCatalog::get_refresh_cost);

	ClassDB::bind_method(D_METHOD("set_refresh_currency", "refresh_currency"), &ShopCatalog::set_refresh_currency);
	ClassDB::bind_method(D_METHOD("get_refresh_currency"), &ShopCatalog::get_refresh_currency);

	ClassDB::bind_method(D_METHOD("to_dict"), &ShopCatalog::to_dict);
	ClassDB::bind_method(D_METHOD("from_dict", "dict"), &ShopCatalog::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &ShopCatalog::clone);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "shop_id"), "set_shop_id", "get_shop_id");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "entries", PROPERTY_HINT_ARRAY_TYPE, "ShopEntry"), "set_entries", "get_entries");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "refresh_cost"), "set_refresh_cost", "get_refresh_cost");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "refresh_currency"), "set_refresh_currency", "get_refresh_currency");
}

Dictionary ShopCatalog::to_dict() const {
	Dictionary d;
	d["shop_id"] = shop_id;
	Array entries_array;
	for (int i = 0; i < entries.size(); ++i) {
		Ref<ShopEntry> entry = entries[i];
		if (entry.is_valid()) {
			entries_array.append(entry->to_dict());
		} else {
			entries_array.append(Dictionary());
		}
	}
	d["entries"] = entries_array;
	d["refresh_cost"] = refresh_cost;
	d["refresh_currency"] = refresh_currency;
	return d;
}

void ShopCatalog::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("shop_id")) {
		shop_id = p_dict["shop_id"];
	}
	if (p_dict.has("entries")) {
		Array entries_array = p_dict["entries"];
		entries.clear();
		for (int i = 0; i < entries_array.size(); ++i) {
			Ref<ShopEntry> entry = ShopEntry::from_dict(entries_array[i]);
			entries.append(entry);
		}
	}
	if (p_dict.has("refresh_cost")) {
		refresh_cost = p_dict["refresh_cost"];
	}
	if (p_dict.has("refresh_currency")) {
		refresh_currency = p_dict["refresh_currency"];
	}
}

Ref<ShopCatalog> ShopCatalog::clone() const {
	Ref<ShopCatalog> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
