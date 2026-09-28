#include "shop.h"

#include "currency_wallet.h"
#include "shop_catalog.h"
#include "shop_entry.h"
#include "inventory/backpack.h"
#include "item/item_def.h"
#include "item/item_instance.h"

#include "core/io/resource_loader.h"
#include "core/object/class_db.h"
#include "core/string/ustring.h"

static Ref<ItemDef> _lookup_item_def(const StringName &p_item_id) {
	String path = "res://resources/items/" + String(p_item_id) + ".tres";
	Ref<Resource> res = ResourceLoader::load(path);
	if (res.is_valid()) {
		Ref<ItemDef> def = Object::cast_to<ItemDef>(res.ptr());
		if (def.is_valid()) {
			return def;
		}
	}
	return Ref<ItemDef>();
}

void Shop::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_catalog", "catalog"), &Shop::set_catalog);
	ClassDB::bind_method(D_METHOD("get_catalog"), &Shop::get_catalog);

	ClassDB::bind_method(D_METHOD("set_player_wallet", "player_wallet"), &Shop::set_player_wallet);
	ClassDB::bind_method(D_METHOD("get_player_wallet"), &Shop::get_player_wallet);

	ClassDB::bind_method(D_METHOD("set_player_backpack", "player_backpack"), &Shop::set_player_backpack);
	ClassDB::bind_method(D_METHOD("get_player_backpack"), &Shop::get_player_backpack);

	ClassDB::bind_method(D_METHOD("buy", "entry_index"), &Shop::buy);
	ClassDB::bind_method(D_METHOD("sell", "item"), &Shop::sell);
	ClassDB::bind_method(D_METHOD("refresh"), &Shop::refresh);
	ClassDB::bind_method(D_METHOD("get_available_entries"), &Shop::get_available_entries);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "catalog", PROPERTY_HINT_RESOURCE_TYPE, "ShopCatalog"), "set_catalog", "get_catalog");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "player_wallet", PROPERTY_HINT_RESOURCE_TYPE, "CurrencyWallet"), "set_player_wallet", "get_player_wallet");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "player_backpack", PROPERTY_HINT_RESOURCE_TYPE, "Backpack"), "set_player_backpack", "get_player_backpack");
}

bool Shop::buy(int p_entry_index) {
	if (catalog.is_null() || player_wallet.is_null()) {
		return false;
	}

	TypedArray<ShopEntry> entries = catalog->get_entries();
	if (p_entry_index < 0 || p_entry_index >= entries.size()) {
		return false;
	}

	Ref<ShopEntry> entry = entries[p_entry_index];
	if (entry.is_null() || entry->get_stock() == 0) {
		return false;
	}

	const int price = entry->get_price();
	const int currency = entry->get_currency();

	if (!player_wallet->can_afford(currency, price)) {
		return false;
	}

	Ref<ItemInstance> item;
	item.instantiate();
	item->set_item_id(entry->get_item_id());
	item->set_quantity(1);

	if (player_backpack.is_valid()) {
		if (!player_backpack->add(item)) {
			return false;
		}
	}

	if (!player_wallet->spend(currency, price)) {
		// Should not fail after can_afford, but handle gracefully.
		if (player_backpack.is_valid()) {
			player_backpack->remove(entry->get_item_id(), 1);
		}
		return false;
	}

	if (entry->get_stock() > 0) {
		entry->set_stock(entry->get_stock() - 1);
	}

	return true;
}

bool Shop::sell(const Ref<ItemInstance> &p_item) {
	if (p_item.is_null() || p_item->get_quantity() <= 0 || player_wallet.is_null()) {
		return false;
	}

	Ref<ItemDef> def = _lookup_item_def(p_item->get_item_id());
	const int value = def.is_valid() ? def->get_base_value() : 0;
	if (value <= 0) {
		return false;
	}

	const int qty = p_item->get_quantity();
	if (player_backpack.is_valid()) {
		if (!player_backpack->remove(p_item->get_item_id(), qty)) {
			return false;
		}
	}

	player_wallet->add(GOLD, value * qty);
	return true;
}

void Shop::refresh() {
	if (catalog.is_null() || player_wallet.is_null()) {
		return;
	}

	const int cost = catalog->get_refresh_cost();
	const int currency = catalog->get_refresh_currency();

	if (cost > 0 && !player_wallet->can_afford(currency, cost)) {
		return;
	}

	if (cost > 0) {
		player_wallet->spend(currency, cost);
	}

	TypedArray<ShopEntry> entries = catalog->get_entries();
	for (int i = 0; i < entries.size(); ++i) {
		Ref<ShopEntry> entry = entries[i];
		if (entry.is_valid()) {
			entry->set_stock(-1);
			entry->set_refresh_count(entry->get_refresh_count() + 1);
		}
	}
}

TypedArray<ShopEntry> Shop::get_available_entries() const {
	TypedArray<ShopEntry> result;
	if (catalog.is_null()) {
		return result;
	}

	TypedArray<ShopEntry> entries = catalog->get_entries();
	for (int i = 0; i < entries.size(); ++i) {
		Ref<ShopEntry> entry = entries[i];
		if (entry.is_valid() && entry->get_stock() != 0) {
			result.append(entry);
		}
	}
	return result;
}
