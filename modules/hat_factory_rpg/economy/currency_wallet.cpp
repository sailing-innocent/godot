#include "currency_wallet.h"

#include "core/object/class_db.h"

void CurrencyWallet::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_amount", "type"), &CurrencyWallet::get_amount);
	ClassDB::bind_method(D_METHOD("can_afford", "type", "amount"), &CurrencyWallet::can_afford);
	ClassDB::bind_method(D_METHOD("add", "type", "amount"), &CurrencyWallet::add);
	ClassDB::bind_method(D_METHOD("spend", "type", "amount"), &CurrencyWallet::spend);

	ClassDB::bind_method(D_METHOD("to_dict"), &CurrencyWallet::to_dict);
	ClassDB::bind_static_method("CurrencyWallet", D_METHOD("from_dict", "dict"), &CurrencyWallet::from_dict);
	ClassDB::bind_method(D_METHOD("clone"), &CurrencyWallet::clone);

	ADD_SIGNAL(MethodInfo("changed"));

	BIND_ENUM_CONSTANT(GOLD);
	BIND_ENUM_CONSTANT(CRYSTAL);
	BIND_ENUM_CONSTANT(REPUTATION);
}

int CurrencyWallet::get_amount(int p_type) const {
	if (amounts.has(p_type)) {
		return amounts[p_type];
	}
	return 0;
}

bool CurrencyWallet::can_afford(int p_type, int p_amount) const {
	if (p_amount <= 0) {
		return true;
	}
	return get_amount(p_type) >= p_amount;
}

bool CurrencyWallet::add(int p_type, int p_amount) {
	if (p_amount < 0) {
		return false;
	}
	const int current = get_amount(p_type);
	amounts[p_type] = current + p_amount;
	emit_signal("changed");
	return true;
}

bool CurrencyWallet::spend(int p_type, int p_amount) {
	if (p_amount < 0) {
		return false;
	}
	if (!can_afford(p_type, p_amount)) {
		return false;
	}
	const int current = get_amount(p_type);
	amounts[p_type] = current - p_amount;
	emit_signal("changed");
	return true;
}

Dictionary CurrencyWallet::to_dict() const {
	Dictionary d;
	Array keys = amounts.keys();
	for (int i = 0; i < keys.size(); ++i) {
		const int type = keys[i];
		d[type] = amounts[type];
	}
	return d;
}

Ref<CurrencyWallet> CurrencyWallet::from_dict(const Dictionary &p_dict) {
	Ref<CurrencyWallet> wallet;
	wallet.instantiate();

	Array keys = p_dict.keys();
	for (int i = 0; i < keys.size(); ++i) {
		const int type = keys[i];
		wallet->amounts[type] = p_dict[type];
	}

	return wallet;
}

Ref<CurrencyWallet> CurrencyWallet::clone() const {
	return from_dict(to_dict());
}
