#include "skill_component.h"

#include "core/object/class_db.h"

void SkillComponent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_skill_ids", "skill_ids"), &SkillComponent::set_skill_ids);
	ClassDB::bind_method(D_METHOD("get_skill_ids"), &SkillComponent::get_skill_ids);
	ClassDB::bind_method(D_METHOD("set_cooldowns", "cooldowns"), &SkillComponent::set_cooldowns);
	ClassDB::bind_method(D_METHOD("get_cooldowns"), &SkillComponent::get_cooldowns);
	ClassDB::bind_method(D_METHOD("set_cooldown", "skill_id", "turns"), &SkillComponent::set_cooldown);
	ClassDB::bind_method(D_METHOD("get_cooldown", "skill_id"), &SkillComponent::get_cooldown);
	ClassDB::bind_method(D_METHOD("tick_cooldowns"), &SkillComponent::tick_cooldowns);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "skill_ids", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_skill_ids", "get_skill_ids");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "cooldowns"), "set_cooldowns", "get_cooldowns");
}

void SkillComponent::set_cooldown(const StringName &p_skill_id, int p_turns) {
	if (p_turns <= 0) {
		cooldowns.erase(p_skill_id);
	} else {
		cooldowns[p_skill_id] = p_turns;
	}
}

int SkillComponent::get_cooldown(const StringName &p_skill_id) const {
	return cooldowns.get(p_skill_id, 0);
}

void SkillComponent::tick_cooldowns() {
	Array keys = cooldowns.keys();
	for (int i = 0; i < keys.size(); i++) {
		StringName key = keys[i];
		int val = cooldowns[key];
		if (val <= 1) {
			cooldowns.erase(key);
		} else {
			cooldowns[key] = val - 1;
		}
	}
}

Dictionary SkillComponent::to_dict() const {
	Dictionary d;
	d["skill_ids"] = skill_ids;
	d["cooldowns"] = cooldowns;
	return d;
}

void SkillComponent::from_dict(const Dictionary &p_dict) {
	if (p_dict.has("skill_ids")) skill_ids = TypedArray<StringName>(p_dict["skill_ids"]);
	if (p_dict.has("cooldowns")) cooldowns = p_dict["cooldowns"];
}

Ref<CombatComponent> SkillComponent::clone() const {
	Ref<SkillComponent> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
