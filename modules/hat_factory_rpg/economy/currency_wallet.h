#ifndef HAT_FACTORY_RPG_CURRENCY_WALLET_H
#define HAT_FACTORY_RPG_CURRENCY_WALLET_H

#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/type_info.h"

enum CurrencyType {
	GOLD = 0,
	CRYSTAL = 1,
	REPUTATION = 2
};

class CurrencyWallet : public RefCounted {
	GDCLASS(CurrencyWallet, RefCounted)

	Dictionary amounts; // int (CurrencyType) -> int

protected:
	static void _bind_methods();

public:
	int get_amount(int p_type) const;
	bool can_afford(int p_type, int p_amount) const;
	bool add(int p_type, int p_amount);
	bool spend(int p_type, int p_amount);

	Dictionary to_dict() const;
	static Ref<CurrencyWallet> from_dict(const Dictionary &p_dict);
	Ref<CurrencyWallet> clone() const;
};

VARIANT_ENUM_CAST(CurrencyType);

#endif // HAT_FACTORY_RPG_CURRENCY_WALLET_H
