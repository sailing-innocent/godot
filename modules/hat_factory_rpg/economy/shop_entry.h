#ifndef HAT_FACTORY_RPG_SHOP_ENTRY_H
#define HAT_FACTORY_RPG_SHOP_ENTRY_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"

class ShopEntry : public RefCounted {
	GDCLASS(ShopEntry, RefCounted)

	StringName item_id;
	int currency = 0;
	int price = 0;
	int stock = -1;
	int refresh_count = 0;

protected:
	static void _bind_methods();

public:
	void set_item_id(const StringName &p_id) { item_id = p_id; }
	StringName get_item_id() const { return item_id; }

	void set_currency(int p_currency) { currency = p_currency; }
	int get_currency() const { return currency; }

	void set_price(int p_price) { price = p_price; }
	int get_price() const { return price; }

	void set_stock(int p_stock) { stock = p_stock; }
	int get_stock() const { return stock; }

	void set_refresh_count(int p_count) { refresh_count = p_count; }
	int get_refresh_count() const { return refresh_count; }

	Dictionary to_dict() const;
	static Ref<ShopEntry> from_dict(const Dictionary &p_dict);
	Ref<ShopEntry> clone() const;
};

#endif // HAT_FACTORY_RPG_SHOP_ENTRY_H
