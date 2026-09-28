#ifndef HAT_FACTORY_RPG_ITEM_EFFECT_H
#define HAT_FACTORY_RPG_ITEM_EFFECT_H

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"

class CombatEntity;

class ItemEffect : public Resource {
	GDCLASS(ItemEffect, Resource)

public:
	enum EffectType {
		EFFECT_ATTACK = 0,
		EFFECT_DEFENSE = 1,
		EFFECT_MAGIC = 2,
		EFFECT_RESISTANCE = 3,
		EFFECT_AIM = 4,
		EFFECT_EVASION = 5,
		EFFECT_CRIT_RATE = 6,
		EFFECT_CRIT_DAMAGE = 7,
		EFFECT_HEAL_HP = 8,
		EFFECT_RESTORE_MP = 9,
		EFFECT_RESTORE_AP = 10,
		EFFECT_APPLY_STATUS = 11,
	};

private:
	StringName effect_id;
	int effect_type = 0;
	int value = 0;
	int duration = 0;

protected:
	static void _bind_methods();

public:
	void set_effect_id(const StringName &p_id) { effect_id = p_id; }
	StringName get_effect_id() const { return effect_id; }

	void set_effect_type(int p_type) { effect_type = p_type; }
	int get_effect_type() const { return effect_type; }

	void set_value(int p_value) { value = p_value; }
	int get_value() const { return value; }

	void set_duration(int p_duration) { duration = p_duration; }
	int get_duration() const { return duration; }

	virtual void apply(const Ref<CombatEntity> &p_target) const;

	Dictionary to_dict() const;
	void from_dict(const Dictionary &p_dict);
	Ref<ItemEffect> clone() const;
};

VARIANT_ENUM_CAST(ItemEffect::EffectType);

#endif // HAT_FACTORY_RPG_ITEM_EFFECT_H
