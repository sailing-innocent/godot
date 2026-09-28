#include "status_component.h"

#include "core/object/class_db.h"

void StatusEffectInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_effect_id", "effect_id"), &StatusEffectInstance::set_effect_id);
	ClassDB::bind_method(D_METHOD("get_effect_id"), &StatusEffectInstance::get_effect_id);
	ClassDB::bind_method(D_METHOD("set_stacks", "stacks"), &StatusEffectInstance::set_stacks);
	ClassDB::bind_method(D_METHOD("get_stacks"), &StatusEffectInstance::get_stacks);
	ClassDB::bind_method(D_METHOD("set_remaining_turns", "remaining_turns"), &StatusEffectInstance::set_remaining_turns);
	ClassDB::bind_method(D_METHOD("get_remaining_turns"), &StatusEffectInstance::get_remaining_turns);
	ClassDB::bind_method(D_METHOD("set_source_entity_id", "source_entity_id"), &StatusEffectInstance::set_source_entity_id);
	ClassDB::bind_method(D_METHOD("get_source_entity_id"), &StatusEffectInstance::get_source_entity_id);
	ClassDB::bind_method(D_METHOD("to_dict"), &StatusEffectInstance::to_dict);
	ClassDB::bind_method(D_METHOD("clone"), &StatusEffectInstance::clone);

	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "effect_id"), "set_effect_id", "get_effect_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "stacks"), "set_stacks", "get_stacks");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "remaining_turns"), "set_remaining_turns", "get_remaining_turns");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "source_entity_id"), "set_source_entity_id", "get_source_entity_id");
}

Dictionary StatusEffectInstance::to_dict() const {
	Dictionary d;
	d["effect_id"] = effect_id;
	d["stacks"] = stacks;
	d["remaining_turns"] = remaining_turns;
	d["source_entity_id"] = source_entity_id;
	return d;
}

Ref<StatusEffectInstance> StatusEffectInstance::from_dict(const Dictionary &p_dict) {
	Ref<StatusEffectInstance> inst;
	inst.instantiate();
	if (p_dict.has("effect_id")) inst->effect_id = p_dict["effect_id"];
	if (p_dict.has("stacks")) inst->stacks = p_dict["stacks"];
	if (p_dict.has("remaining_turns")) inst->remaining_turns = p_dict["remaining_turns"];
	if (p_dict.has("source_entity_id")) inst->source_entity_id = p_dict["source_entity_id"];
	return inst;
}

Ref<StatusEffectInstance> StatusEffectInstance::clone() const {
	return from_dict(to_dict());
}

void StatusComponent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_effects", "effects"), &StatusComponent::set_effects);
	ClassDB::bind_method(D_METHOD("get_effects"), &StatusComponent::get_effects);
	ClassDB::bind_method(D_METHOD("add_effect", "effect"), &StatusComponent::add_effect);
	ClassDB::bind_method(D_METHOD("remove_effect", "effect_id"), &StatusComponent::remove_effect);
	ClassDB::bind_method(D_METHOD("has_effect", "effect_id"), &StatusComponent::has_effect);
	ClassDB::bind_method(D_METHOD("get_effect", "effect_id"), &StatusComponent::get_effect);
	ClassDB::bind_method(D_METHOD("tick_turns"), &StatusComponent::tick_turns);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "effects", PROPERTY_HINT_ARRAY_TYPE, "StatusEffectInstance"), "set_effects", "get_effects");
}

void StatusComponent::add_effect(const Ref<StatusEffectInstance> &p_effect) {
	if (p_effect.is_null()) return;
	Ref<StatusEffectInstance> existing = get_effect(p_effect->get_effect_id());
	if (existing.is_valid()) {
		existing->set_stacks(existing->get_stacks() + p_effect->get_stacks());
		existing->set_remaining_turns(MAX(existing->get_remaining_turns(), p_effect->get_remaining_turns()));
	} else {
		effects.push_back(p_effect);
	}
}

void StatusComponent::remove_effect(const StringName &p_effect_id) {
	for (int i = effects.size() - 1; i >= 0; i--) {
		Ref<StatusEffectInstance> e = effects[i];
		if (e.is_valid() && e->get_effect_id() == p_effect_id) {
			effects.remove_at(i);
		}
	}
}

bool StatusComponent::has_effect(const StringName &p_effect_id) const {
	return get_effect(p_effect_id).is_valid();
}

Ref<StatusEffectInstance> StatusComponent::get_effect(const StringName &p_effect_id) const {
	for (int i = 0; i < effects.size(); i++) {
		Ref<StatusEffectInstance> e = effects[i];
		if (e.is_valid() && e->get_effect_id() == p_effect_id) {
			return e;
		}
	}
	return Ref<StatusEffectInstance>();
}

void StatusComponent::tick_turns() {
	for (int i = effects.size() - 1; i >= 0; i--) {
		Ref<StatusEffectInstance> e = effects[i];
		if (e.is_null()) continue;
		e->set_remaining_turns(e->get_remaining_turns() - 1);
		if (e->get_remaining_turns() <= 0) {
			effects.remove_at(i);
		}
	}
}

Dictionary StatusComponent::to_dict() const {
	Dictionary d;
	Array arr;
	for (int i = 0; i < effects.size(); i++) {
		Ref<StatusEffectInstance> e = effects[i];
		if (e.is_valid()) arr.push_back(e->to_dict());
	}
	d["effects"] = arr;
	return d;
}

void StatusComponent::from_dict(const Dictionary &p_dict) {
	effects.clear();
	if (p_dict.has("effects")) {
		Array arr = p_dict["effects"];
		for (int i = 0; i < arr.size(); i++) {
			Ref<StatusEffectInstance> e = StatusEffectInstance::from_dict(arr[i]);
			effects.push_back(e);
		}
	}
}

Ref<CombatComponent> StatusComponent::clone() const {
	Ref<StatusComponent> copy;
	copy.instantiate();
	copy->from_dict(to_dict());
	return copy;
}
