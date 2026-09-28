#ifndef STATUS_COMPONENT_H
#define STATUS_COMPONENT_H

#include "combat_component.h"
#include "core/variant/typed_array.h"

class StatusEffectInstance : public RefCounted {
	GDCLASS(StatusEffectInstance, RefCounted)

	StringName effect_id;
	int stacks = 1;
	int remaining_turns = 1;
	int source_entity_id = 0;

protected:
	static void _bind_methods();

public:
	void set_effect_id(const StringName &p_value) { effect_id = p_value; }
	StringName get_effect_id() const { return effect_id; }

	void set_stacks(int p_value) { stacks = p_value; }
	int get_stacks() const { return stacks; }

	void set_remaining_turns(int p_value) { remaining_turns = p_value; }
	int get_remaining_turns() const { return remaining_turns; }

	void set_source_entity_id(int p_value) { source_entity_id = p_value; }
	int get_source_entity_id() const { return source_entity_id; }

	Dictionary to_dict() const;
	static Ref<StatusEffectInstance> from_dict(const Dictionary &p_dict);
	Ref<StatusEffectInstance> clone() const;
};

class StatusComponent : public CombatComponent {
	GDCLASS(StatusComponent, CombatComponent)

	TypedArray<StatusEffectInstance> effects;

protected:
	static void _bind_methods();

public:
	virtual StringName get_component_name() const override { return StringName("Status"); }

	void set_effects(const TypedArray<StatusEffectInstance> &p_value) { effects = p_value; }
	TypedArray<StatusEffectInstance> get_effects() const { return effects; }

	void add_effect(const Ref<StatusEffectInstance> &p_effect);
	void remove_effect(const StringName &p_effect_id);
	bool has_effect(const StringName &p_effect_id) const;
	Ref<StatusEffectInstance> get_effect(const StringName &p_effect_id) const;
	void tick_turns();

	virtual Dictionary to_dict() const override;
	virtual void from_dict(const Dictionary &p_dict) override;
	virtual Ref<CombatComponent> clone() const override;
};

#endif // STATUS_COMPONENT_H
