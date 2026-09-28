#include "shop_entry.h"

#include "core/object/class_db.h"

void ShopEntry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_item_id", "item_id"), &ShopEntry::set_item_id);
	ClassDB::bind_method(D_METHOD("get_item_id"), &ShopEntry::get_item_id);

	ClassDB::bind_method(D_METHOD("set_currency", "currency"), &ShopEntry::set_currency);
	ClassDB::bind_method(D_METHOD("get_currency"), &ShopEntry::get_currency);

	ClassDB::bind_method(D_METHOD("set_price", "price"), &ShopEntry::set_price);
	ClassDB::bind_method(D_METHOD("get_price"), &ShopEntry::get_price);

	ClassDB::bind_method(D_METHOD("set_stock", "stock"), &ShopEntry::set_stock);
	ClassDB::bind_method(D_METHOD("get_stock"), &ShopEntry::get_stock);

	ClassDB::bind_method(D_METHOD("set_refresh_count", "refresh_count"), &ShopEntry::set_refresh_count);
	ClassDB::bind_method(D_METHOD("get_refresh_count"), &ShopEntry::get_refresh_count);

	ClassDB::bind_method(D_METHOD("to_dict"), &ShopEntry::to_dict);
	ClassDB::bind_static_method("ShopEntry", D_METHOD("from_dict", "dict"), &ShopEntry::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &ShopEntry::clone);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "item_id"), "set_item_id", "get_item_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "currency"), "set_currency", "get_currency");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "price"), "set_price", "get_price");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "stock"), "set_stock", "get_stock");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "refresh_count"), "set_refresh_count", "get_refresh_count");
}

Dictionary ShopEntry::to_dict() const {
	Dictionary d;
	d["item_id"] = item_id;
	d["currency"] = currency;
	d["price"] = price;
	d["stock"] = stock;
	d["refresh_count"] = refresh_count;
	return d;
}

Ref<ShopEntry> ShopEntry::from_dict(const Dictionary &p_dict) {
	Ref<ShopEntry> entry;
	entry.instantiate();

	if (p_dict.has("item_id")) {
		entry->item_id = p_dict["item_id"];
	}
	if (p_dict.has("currency")) {
		entry->currency = p_dict["currency"];
	}
	if (p_dict.has("price")) {
		entry->price = p_dict["price"];
	}
	if (p_dict.has("stock")) {
		entry->stock = p_dict["stock"];
	}
	if (p_dict.has("refresh_count")) {
		entry->refresh_count = p_dict["refresh_count"];
	}

	return entry;
}

Ref<ShopEntry> ShopEntry::clone() const {
	return from_dict(to_dict());
}
