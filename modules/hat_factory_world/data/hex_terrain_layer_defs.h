/**************************************************************************/
/*  hex_terrain_layer_defs.h                                              */
/*  Hat Factory World module — layered terrain definition tables.        */
/*                                                                        */
/*  Terrain is a stack of layers, never a single id (MAP-20):            */
/*    base terrain + elevation + surfaces[] + objects[] + edges[6] +       */
/*    area_effects[]. Rules fire on tags, never on hardcoded ids           */
/*  (MAP-21). Destructible layers declare durability + destroyed form      */
/*  (MAP-22). Reactions and spread are declarative in v1 (MAP-23/24):      */
/*  data + event hooks land now, engine settlement is an extension point.  */
/**************************************************************************/
#ifndef HEX_TERRAIN_LAYER_DEFS_H
#define HEX_TERRAIN_LAYER_DEFS_H

#include "../world/world_ids.h"

#include "core/io/resource.h"
#include "core/string/string_name.h"
#include "core/templates/hash_set.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

/**
 * @brief Common base for every table entry: stable id + tags.
 */
class HexLayerDefBase : public Resource {
	GDCLASS(HexLayerDefBase, Resource)
	StringName def_id;
	String display_name;
	PackedStringArray tags;

protected:
	static void _bind_methods();

public:
	void set_def_id(const StringName &p_id);
	StringName get_def_id() const;
	void set_display_name(const String &p_name);
	String get_display_name() const;
	void set_tags(const PackedStringArray &p_tags);
	PackedStringArray get_tags() const;
	bool has_tag(const StringName &p_tag) const;
	Dictionary base_to_dict() const;
	void base_from_dict(const Dictionary &p_dict);
};

/**
 * @brief Base terrain (泥土/岩石/草地/瓷砖/沙). Migration target of
 * HexTerrainDef.move_cost / blocks_movement (see dual-map-v1-scope §迁移口径).
 */
class HexBaseTerrainDef : public HexLayerDefBase {
	GDCLASS(HexBaseTerrainDef, HexLayerDefBase)
	int move_cost = 1;
	bool blocks_movement = false;

protected:
	static void _bind_methods();

public:
	void set_move_cost(int p_cost);
	int get_move_cost() const;
	void set_blocks_movement(bool p_enabled);
	bool get_blocks_movement() const;
	Dictionary to_dict() const;
	static Ref<HexBaseTerrainDef> from_dict(const Dictionary &p_dict);
};

/**
 * @brief Stacked surface layer (水深浅/深、熔岩、黏浆、釉油、冰、雪).
 */
class HexSurfaceDef : public HexLayerDefBase {
	GDCLASS(HexSurfaceDef, HexLayerDefBase)
	bool stackable = true;
	bool spreadable = false; // MAP-24 declaration; engine settlement is an extension point.
	bool blocks_vision = false;
	bool blocks_movement = false;
	double depth = 0.0; // world meters; deep vs shallow water comes from depth + tags

protected:
	static void _bind_methods();

public:
	void set_stackable(bool p_enabled);
	bool get_stackable() const;
	void set_spreadable(bool p_enabled);
	bool get_spreadable() const;
	void set_blocks_vision(bool p_enabled);
	bool get_blocks_vision() const;
	void set_blocks_movement(bool p_enabled);
	bool get_blocks_movement() const;
	void set_depth(double p_depth);
	double get_depth() const;
	Dictionary to_dict() const;
	static Ref<HexSurfaceDef> from_dict(const Dictionary &p_dict);
};

/**
 * @brief Object on a cell (树/瓷柱/箱子/路障/机关/宝箱/拾取物).
 * Occupancy: "none" | "occupies_cell" | "pass_through" (MAP-14).
 */
class HexObjectDef : public HexLayerDefBase {
	GDCLASS(HexObjectDef, HexLayerDefBase)
	StringName occupancy = StringName("none");
	int durability = -1; // -1 = 坚固 (indestructible)
	StringName destroyed_form; // object def id after destruction; empty = vanishes
	bool blocks_vision = false;

protected:
	static void _bind_methods();

public:
	void set_occupancy(const StringName &p_occupancy);
	StringName get_occupancy() const;
	void set_durability(int p_durability);
	int get_durability() const;
	void set_destroyed_form(const StringName &p_form);
	StringName get_destroyed_form() const;
	void set_blocks_vision(bool p_enabled);
	bool get_blocks_vision() const;
	bool is_destructible() const;
	Dictionary to_dict() const;
	static Ref<HexObjectDef> from_dict(const Dictionary &p_dict);
};

/**
 * @brief Edge between adjacent micro cells (墙/栅栏/崖壁/闸门/桥/河岸/窄缝).
 * Action types: walk / hop(翻跳) / fall(跌落) / glide(滑翔) / wade(涉水) /
 * break_through(破障后通过). Gates and shortcuts carry lock/enable data
 * (MAP-33/34: data + commands in v1-min).
 */
class HexEdgeTypeDef : public HexLayerDefBase {
	GDCLASS(HexEdgeTypeDef, HexLayerDefBase)
	StringName action_type = StringName("walk");
	bool directional = false;
	bool blocks_movement = true;
	bool blocks_vision = false;
	int climb_height = 0; // height delta this edge mediates
	int durability = -1; // -1 = 坚固
	StringName destroyed_form; // edge type id after destruction
	StringName required_key_id; // empty = unlocked (MAP-33)
	bool enabled_by_default = true; // false = closed gate / not-yet-opened shortcut (MAP-34)

protected:
	static void _bind_methods();

public:
	void set_action_type(const StringName &p_action);
	StringName get_action_type() const;
	void set_directional(bool p_enabled);
	bool get_directional() const;
	void set_blocks_movement(bool p_enabled);
	bool get_blocks_movement() const;
	void set_blocks_vision(bool p_enabled);
	bool get_blocks_vision() const;
	void set_climb_height(int p_height);
	int get_climb_height() const;
	void set_durability(int p_durability);
	int get_durability() const;
	void set_destroyed_form(const StringName &p_form);
	StringName get_destroyed_form() const;
	void set_required_key_id(const StringName &p_key);
	StringName get_required_key_id() const;
	void set_enabled_by_default(bool p_enabled);
	bool get_enabled_by_default() const;
	bool is_destructible() const;
	Dictionary to_dict() const;
	static Ref<HexEdgeTypeDef> from_dict(const Dictionary &p_dict);
};

/**
 * @brief Area effect (雾/黑暗/火焰/毒沼/警戒圈/安全区).
 * Duration/spread are declarations in v1 (MAP-24 engine = extension point).
 */
class HexAreaEffectDef : public HexLayerDefBase {
	GDCLASS(HexAreaEffectDef, HexLayerDefBase)
	int duration_ticks = -1; // -1 = permanent
	bool spread = false; // MAP-24 declaration; settlement engine is an extension point.
	bool blocks_entry = false;
	double vision_modifier = 0.0; // multiplicative vision range factor

protected:
	static void _bind_methods();

public:
	void set_duration_ticks(int p_ticks);
	int get_duration_ticks() const;
	void set_spread(bool p_enabled);
	bool get_spread() const;
	void set_blocks_entry(bool p_enabled);
	bool get_blocks_entry() const;
	void set_vision_modifier(double p_modifier);
	double get_vision_modifier() const;
	Dictionary to_dict() const;
	static Ref<HexAreaEffectDef> from_dict(const Dictionary &p_dict);
};

/**
 * @brief Declarative terrain reaction (火+草→燃烧, MAP-23).
 * v1 stores the table and exposes the trigger-event hook; the settlement
 * engine is an explicit extension point (see dual-map-v1-scope §留空清单).
 */
class HexReactionDef : public HexLayerDefBase {
	GDCLASS(HexReactionDef, HexLayerDefBase)
	StringName trigger_event = StringName("applied"); // applied | turn_start | on_enter
	PackedStringArray source_tags; // tags the acting layer carries
	PackedStringArray target_tags; // tags the receiving cell/layer must carry
	StringName result_surface; // surface def id to apply; empty = none
	StringName result_effect; // area effect def id to apply; empty = none

protected:
	static void _bind_methods();

public:
	void set_trigger_event(const StringName &p_event);
	StringName get_trigger_event() const;
	void set_source_tags(const PackedStringArray &p_tags);
	PackedStringArray get_source_tags() const;
	void set_target_tags(const PackedStringArray &p_tags);
	PackedStringArray get_target_tags() const;
	void set_result_surface(const StringName &p_surface);
	StringName get_result_surface() const;
	void set_result_effect(const StringName &p_effect);
	StringName get_result_effect() const;
	Dictionary to_dict() const;
	static Ref<HexReactionDef> from_dict(const Dictionary &p_dict);
};

#endif // HEX_TERRAIN_LAYER_DEFS_H
