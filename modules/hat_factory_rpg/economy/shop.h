#ifndef HAT_FACTORY_RPG_SHOP_H
#define HAT_FACTORY_RPG_SHOP_H

#include "core/object/ref_counted.h"
#include "core/variant/typed_array.h"

class ShopCatalog;
class CurrencyWallet;
class Backpack;
class ItemInstance;
class ShopEntry;

class Shop : public RefCounted {
	GDCLASS(Shop, RefCounted)

	Ref<ShopCatalog> catalog;
	Ref<CurrencyWallet> player_wallet;
	Ref<Backpack> player_backpack;

protected:
	static void _bind_methods();

public:
	void set_catalog(const Ref<ShopCatalog> &p_catalog) { catalog = p_catalog; }
	Ref<ShopCatalog> get_catalog() const { return catalog; }

	void set_player_wallet(const Ref<CurrencyWallet> &p_wallet) { player_wallet = p_wallet; }
	Ref<CurrencyWallet> get_player_wallet() const { return player_wallet; }

	void set_player_backpack(const Ref<Backpack> &p_backpack) { player_backpack = p_backpack; }
	Ref<Backpack> get_player_backpack() const { return player_backpack; }

	bool buy(int p_entry_index);
	bool sell(const Ref<ItemInstance> &p_item);
	void refresh();
	TypedArray<ShopEntry> get_available_entries() const;
};

#endif // HAT_FACTORY_RPG_SHOP_H
