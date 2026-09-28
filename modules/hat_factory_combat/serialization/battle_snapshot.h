#ifndef BATTLE_SNAPSHOT_H
#define BATTLE_SNAPSHOT_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"

class BattleState;

class BattleSnapshot : public RefCounted {
	GDCLASS(BattleSnapshot, RefCounted)

protected:
	static void _bind_methods();

public:
	static String state_to_json(const Ref<BattleState> &p_state, bool p_pretty = true);
	static Ref<BattleState> state_from_json(const String &p_json);
	static Dictionary state_to_dict(const Ref<BattleState> &p_state);
	static Ref<BattleState> state_from_dict(const Dictionary &p_dict);
};

#endif // BATTLE_SNAPSHOT_H
