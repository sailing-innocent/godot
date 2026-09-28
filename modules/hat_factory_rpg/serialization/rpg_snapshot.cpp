#include "rpg_snapshot.h"

#include "inventory/backpack.h"
#include "economy/currency_wallet.h"

#include "core/object/class_db.h"

void RPGSnapshot::_bind_methods() {
	ClassDB::bind_static_method("RPGSnapshot", D_METHOD("backpack_to_dict", "backpack"), &RPGSnapshot::backpack_to_dict);
	ClassDB::bind_static_method("RPGSnapshot", D_METHOD("backpack_from_dict", "dict"), &RPGSnapshot::backpack_from_dict);
	ClassDB::bind_static_method("RPGSnapshot", D_METHOD("wallet_to_dict", "wallet"), &RPGSnapshot::wallet_to_dict);
	ClassDB::bind_static_method("RPGSnapshot", D_METHOD("wallet_from_dict", "dict"), &RPGSnapshot::wallet_from_dict);
}

Dictionary RPGSnapshot::backpack_to_dict(const Ref<Backpack> &p_backpack) {
	if (p_backpack.is_null()) {
		return Dictionary();
	}
	return p_backpack->to_dict();
}

Ref<Backpack> RPGSnapshot::backpack_from_dict(const Dictionary &p_dict) {
	return Backpack::from_dict(p_dict);
}

Dictionary RPGSnapshot::wallet_to_dict(const Ref<CurrencyWallet> &p_wallet) {
	if (p_wallet.is_null()) {
		return Dictionary();
	}
	return p_wallet->to_dict();
}

Ref<CurrencyWallet> RPGSnapshot::wallet_from_dict(const Dictionary &p_dict) {
	return CurrencyWallet::from_dict(p_dict);
}
