#ifndef PHASE_CONTROLLER_H
#define PHASE_CONTROLLER_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/templates/hash_map.h"

#include "battle_phase.h"

class BattleState;

class PhaseController : public RefCounted {
	GDCLASS(PhaseController, RefCounted)

	HashMap<StringName, Ref<BattlePhase>> phases;
	StringName current_phase_name;

protected:
	static void _bind_methods();

public:
	void register_phase(const StringName &p_name, const Ref<BattlePhase> &p_phase);
	Ref<BattleState> transition_to(const Ref<BattleState> &p_state, const StringName &p_phase_name);
	Ref<BattleState> tick(const Ref<BattleState> &p_state, double p_delta);
	StringName get_current_phase() const { return current_phase_name; }
};

#endif // PHASE_CONTROLLER_H
