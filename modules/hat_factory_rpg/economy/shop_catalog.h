#ifndef HAT_FACTORY_RPG_SHOP_CATALOG_H
#define HAT_FACTORY_RPG_SHOP_CATALOG_H

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

#include "modules/hat_factory_rpg/economy/shop_entry.h"

class ShopCatalog : public Resource {
	GDCLASS(ShopCatalog, Resource)

	StringName shop_id;
	TypedArray<ShopEntry> entries;
	int refresh_cost = 0;
	int refresh_currency = 0;

protected:
	static void _bind_methods();

public:
	void set_shop_id(const StringName &p_id) { shop_id = p_id; }
	StringName get_shop_id() const { return shop_id; }

	void set_entries(const TypedArray<ShopEntry> &p_entries) { entries = p_entries; }
	TypedArray<ShopEntry> get_entries() const { return entries; }

	void set_refresh_cost(int p_cost) { refresh_cost = p_cost; }
	int get_refresh_cost() const { return refresh_cost; }

	void set_refresh_currency(int p_currency) { refresh_currency = p_currency; }
	int get_refresh_currency() const { return refresh_currency; }

	Dictionary to_dict() const;
	void from_dict(const Dictionary &p_dict);
	Ref<ShopCatalog> clone() const;
};

#endif // HAT_FACTORY_RPG_SHOP_CATALOG_H
