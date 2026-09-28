#ifndef BATTLE_PHASE_H
#define BATTLE_PHASE_H

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"

class BattleState;

class BattlePhase : public RefCounted {
	GDCLASS(BattlePhase, RefCounted)

protected:
	static void _bind_methods();

public:
	virtual StringName get_name() const { return StringName(); }
	virtual bool can_enter(const Ref<BattleState> &p_state) const;
	virtual Ref<BattleState> enter(const Ref<BattleState> &p_state) const;
	virtual Ref<BattleState> tick(const Ref<BattleState> &p_state, double p_delta) const;
	virtual Ref<BattleState> exit(const Ref<BattleState> &p_state) const;
};

#endif // BATTLE_PHASE_H
