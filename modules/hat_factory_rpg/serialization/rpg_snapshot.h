#ifndef HAT_FACTORY_RPG_SNAPSHOT_H
#define HAT_FACTORY_RPG_SNAPSHOT_H

#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"

class Backpack;
class CurrencyWallet;

class RPGSnapshot : public RefCounted {
	GDCLASS(RPGSnapshot, RefCounted)

protected:
	static void _bind_methods();

public:
	static Dictionary backpack_to_dict(const Ref<Backpack> &p_backpack);
	static Ref<Backpack> backpack_from_dict(const Dictionary &p_dict);

	static Dictionary wallet_to_dict(const Ref<CurrencyWallet> &p_wallet);
	static Ref<CurrencyWallet> wallet_from_dict(const Dictionary &p_dict);
};

#endif // HAT_FACTORY_RPG_SNAPSHOT_H
