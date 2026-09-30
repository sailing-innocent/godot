/**************************************************************************/
/*  hex_terrain_layer_defs.cpp                                            */
/**************************************************************************/
#include "hex_terrain_layer_defs.h"

#include "core/object/class_db.h"

/**************************************************************************/
/* HexLayerDefBase                                                        */
/**************************************************************************/

void HexLayerDefBase::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_def_id", "id"), &HexLayerDefBase::set_def_id);
	ClassDB::bind_method(D_METHOD("get_def_id"), &HexLayerDefBase::get_def_id);
	ClassDB::bind_method(D_METHOD("set_display_name", "name"), &HexLayerDefBase::set_display_name);
	ClassDB::bind_method(D_METHOD("get_display_name"), &HexLayerDefBase::get_display_name);
	ClassDB::bind_method(D_METHOD("set_tags", "tags"), &HexLayerDefBase::set_tags);
	ClassDB::bind_method(D_METHOD("get_tags"), &HexLayerDefBase::get_tags);
	ClassDB::bind_method(D_METHOD("has_tag", "tag"), &HexLayerDefBase::has_tag);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "def_id"), "set_def_id", "get_def_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_name"), "set_display_name", "get_display_name");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "tags"), "set_tags", "get_tags");
}

void HexLayerDefBase::set_def_id(const StringName &p_id) { def_id = p_id; }
StringName HexLayerDefBase::get_def_id() const { return def_id; }
void HexLayerDefBase::set_display_name(const String &p_name) { display_name = p_name; }
String HexLayerDefBase::get_display_name() const { return display_name; }
void HexLayerDefBase::set_tags(const PackedStringArray &p_tags) { tags = p_tags; }
PackedStringArray HexLayerDefBase::get_tags() const { return tags; }

bool HexLayerDefBase::has_tag(const StringName &p_tag) const {
	for (int i = 0; i < tags.size(); i++) {
		if (tags[i] == p_tag) {
			return true;
		}
	}
	return false;
}

Dictionary HexLayerDefBase::base_to_dict() const {
	Dictionary d;
	d[StringName("def_id")] = def_id;
	d[StringName("display_name")] = display_name;
	d[StringName("tags")] = tags;
	return d;
}

void HexLayerDefBase::base_from_dict(const Dictionary &p_dict) {
	set_def_id(p_dict.get(StringName("def_id"), StringName()));
	set_display_name(p_dict.get(StringName("display_name"), String()));
	set_tags(p_dict.get(StringName("tags"), PackedStringArray()));
}

/**************************************************************************/
/* HexBaseTerrainDef                                                      */
/**************************************************************************/

void HexBaseTerrainDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_move_cost", "cost"), &HexBaseTerrainDef::set_move_cost);
	ClassDB::bind_method(D_METHOD("get_move_cost"), &HexBaseTerrainDef::get_move_cost);
	ClassDB::bind_method(D_METHOD("set_blocks_movement", "enabled"), &HexBaseTerrainDef::set_blocks_movement);
	ClassDB::bind_method(D_METHOD("get_blocks_movement"), &HexBaseTerrainDef::get_blocks_movement);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexBaseTerrainDef::to_dict);
	ClassDB::bind_static_method("HexBaseTerrainDef", D_METHOD("from_dict", "dict"), &HexBaseTerrainDef::from_dict);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "move_cost"), "set_move_cost", "get_move_cost");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "blocks_movement"), "set_blocks_movement", "get_blocks_movement");
}

void HexBaseTerrainDef::set_move_cost(int p_cost) { move_cost = p_cost; }
int HexBaseTerrainDef::get_move_cost() const { return move_cost; }
void HexBaseTerrainDef::set_blocks_movement(bool p_enabled) { blocks_movement = p_enabled; }
bool HexBaseTerrainDef::get_blocks_movement() const { return blocks_movement; }

Dictionary HexBaseTerrainDef::to_dict() const {
	Dictionary d = base_to_dict();
	d[StringName("move_cost")] = move_cost;
	d[StringName("blocks_movement")] = blocks_movement;
	return d;
}

Ref<HexBaseTerrainDef> HexBaseTerrainDef::from_dict(const Dictionary &p_dict) {
	Ref<HexBaseTerrainDef> d;
	d.instantiate();
	d->base_from_dict(p_dict);
	d->set_move_cost(p_dict.get(StringName("move_cost"), 1));
	d->set_blocks_movement(p_dict.get(StringName("blocks_movement"), false));
	return d;
}

/**************************************************************************/
/* HexSurfaceDef                                                          */
/**************************************************************************/

void HexSurfaceDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_stackable", "enabled"), &HexSurfaceDef::set_stackable);
	ClassDB::bind_method(D_METHOD("get_stackable"), &HexSurfaceDef::get_stackable);
	ClassDB::bind_method(D_METHOD("set_spreadable", "enabled"), &HexSurfaceDef::set_spreadable);
	ClassDB::bind_method(D_METHOD("get_spreadable"), &HexSurfaceDef::get_spreadable);
	ClassDB::bind_method(D_METHOD("set_blocks_vision", "enabled"), &HexSurfaceDef::set_blocks_vision);
	ClassDB::bind_method(D_METHOD("get_blocks_vision"), &HexSurfaceDef::get_blocks_vision);
	ClassDB::bind_method(D_METHOD("set_blocks_movement", "enabled"), &HexSurfaceDef::set_blocks_movement);
	ClassDB::bind_method(D_METHOD("get_blocks_movement"), &HexSurfaceDef::get_blocks_movement);
	ClassDB::bind_method(D_METHOD("set_depth", "depth"), &HexSurfaceDef::set_depth);
	ClassDB::bind_method(D_METHOD("get_depth"), &HexSurfaceDef::get_depth);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexSurfaceDef::to_dict);
	ClassDB::bind_static_method("HexSurfaceDef", D_METHOD("from_dict", "dict"), &HexSurfaceDef::from_dict);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "stackable"), "set_stackable", "get_stackable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "spreadable"), "set_spreadable", "get_spreadable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "blocks_vision"), "set_blocks_vision", "get_blocks_vision");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "blocks_movement"), "set_blocks_movement", "get_blocks_movement");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "depth"), "set_depth", "get_depth");
}

void HexSurfaceDef::set_stackable(bool p_enabled) { stackable = p_enabled; }
bool HexSurfaceDef::get_stackable() const { return stackable; }
void HexSurfaceDef::set_spreadable(bool p_enabled) { spreadable = p_enabled; }
bool HexSurfaceDef::get_spreadable() const { return spreadable; }
void HexSurfaceDef::set_blocks_vision(bool p_enabled) { blocks_vision = p_enabled; }
bool HexSurfaceDef::get_blocks_vision() const { return blocks_vision; }
void HexSurfaceDef::set_blocks_movement(bool p_enabled) { blocks_movement = p_enabled; }
bool HexSurfaceDef::get_blocks_movement() const { return blocks_movement; }
void HexSurfaceDef::set_depth(double p_depth) { depth = p_depth; }
double HexSurfaceDef::get_depth() const { return depth; }

Dictionary HexSurfaceDef::to_dict() const {
	Dictionary d = base_to_dict();
	d[StringName("stackable")] = stackable;
	d[StringName("spreadable")] = spreadable;
	d[StringName("blocks_vision")] = blocks_vision;
	d[StringName("blocks_movement")] = blocks_movement;
	d[StringName("depth")] = depth;
	return d;
}

Ref<HexSurfaceDef> HexSurfaceDef::from_dict(const Dictionary &p_dict) {
	Ref<HexSurfaceDef> d;
	d.instantiate();
	d->base_from_dict(p_dict);
	d->set_stackable(p_dict.get(StringName("stackable"), true));
	d->set_spreadable(p_dict.get(StringName("spreadable"), false));
	d->set_blocks_vision(p_dict.get(StringName("blocks_vision"), false));
	d->set_blocks_movement(p_dict.get(StringName("blocks_movement"), false));
	d->set_depth(p_dict.get(StringName("depth"), 0.0));
	return d;
}

/**************************************************************************/
/* HexObjectDef                                                           */
/**************************************************************************/

void HexObjectDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_occupancy", "occupancy"), &HexObjectDef::set_occupancy);
	ClassDB::bind_method(D_METHOD("get_occupancy"), &HexObjectDef::get_occupancy);
	ClassDB::bind_method(D_METHOD("set_durability", "durability"), &HexObjectDef::set_durability);
	ClassDB::bind_method(D_METHOD("get_durability"), &HexObjectDef::get_durability);
	ClassDB::bind_method(D_METHOD("set_destroyed_form", "form"), &HexObjectDef::set_destroyed_form);
	ClassDB::bind_method(D_METHOD("get_destroyed_form"), &HexObjectDef::get_destroyed_form);
	ClassDB::bind_method(D_METHOD("set_blocks_vision", "enabled"), &HexObjectDef::set_blocks_vision);
	ClassDB::bind_method(D_METHOD("get_blocks_vision"), &HexObjectDef::get_blocks_vision);
	ClassDB::bind_method(D_METHOD("is_destructible"), &HexObjectDef::is_destructible);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexObjectDef::to_dict);
	ClassDB::bind_static_method("HexObjectDef", D_METHOD("from_dict", "dict"), &HexObjectDef::from_dict);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "occupancy"), "set_occupancy", "get_occupancy");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "durability"), "set_durability", "get_durability");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "destroyed_form"), "set_destroyed_form", "get_destroyed_form");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "blocks_vision"), "set_blocks_vision", "get_blocks_vision");
}

void HexObjectDef::set_occupancy(const StringName &p_occupancy) { occupancy = p_occupancy; }
StringName HexObjectDef::get_occupancy() const { return occupancy; }
void HexObjectDef::set_durability(int p_durability) { durability = p_durability; }
int HexObjectDef::get_durability() const { return durability; }
void HexObjectDef::set_destroyed_form(const StringName &p_form) { destroyed_form = p_form; }
StringName HexObjectDef::get_destroyed_form() const { return destroyed_form; }
void HexObjectDef::set_blocks_vision(bool p_enabled) { blocks_vision = p_enabled; }
bool HexObjectDef::get_blocks_vision() const { return blocks_vision; }
bool HexObjectDef::is_destructible() const { return durability >= 0; }

Dictionary HexObjectDef::to_dict() const {
	Dictionary d = base_to_dict();
	d[StringName("occupancy")] = occupancy;
	d[StringName("durability")] = durability;
	d[StringName("destroyed_form")] = destroyed_form;
	d[StringName("blocks_vision")] = blocks_vision;
	return d;
}

Ref<HexObjectDef> HexObjectDef::from_dict(const Dictionary &p_dict) {
	Ref<HexObjectDef> d;
	d.instantiate();
	d->base_from_dict(p_dict);
	d->set_occupancy(p_dict.get(StringName("occupancy"), StringName("none")));
	d->set_durability(p_dict.get(StringName("durability"), -1));
	d->set_destroyed_form(p_dict.get(StringName("destroyed_form"), StringName()));
	d->set_blocks_vision(p_dict.get(StringName("blocks_vision"), false));
	return d;
}

/**************************************************************************/
/* HexEdgeTypeDef                                                         */
/**************************************************************************/

void HexEdgeTypeDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_action_type", "action"), &HexEdgeTypeDef::set_action_type);
	ClassDB::bind_method(D_METHOD("get_action_type"), &HexEdgeTypeDef::get_action_type);
	ClassDB::bind_method(D_METHOD("set_directional", "enabled"), &HexEdgeTypeDef::set_directional);
	ClassDB::bind_method(D_METHOD("get_directional"), &HexEdgeTypeDef::get_directional);
	ClassDB::bind_method(D_METHOD("set_blocks_movement", "enabled"), &HexEdgeTypeDef::set_blocks_movement);
	ClassDB::bind_method(D_METHOD("get_blocks_movement"), &HexEdgeTypeDef::get_blocks_movement);
	ClassDB::bind_method(D_METHOD("set_blocks_vision", "enabled"), &HexEdgeTypeDef::set_blocks_vision);
	ClassDB::bind_method(D_METHOD("get_blocks_vision"), &HexEdgeTypeDef::get_blocks_vision);
	ClassDB::bind_method(D_METHOD("set_climb_height", "height"), &HexEdgeTypeDef::set_climb_height);
	ClassDB::bind_method(D_METHOD("get_climb_height"), &HexEdgeTypeDef::get_climb_height);
	ClassDB::bind_method(D_METHOD("set_durability", "durability"), &HexEdgeTypeDef::set_durability);
	ClassDB::bind_method(D_METHOD("get_durability"), &HexEdgeTypeDef::get_durability);
	ClassDB::bind_method(D_METHOD("set_destroyed_form", "form"), &HexEdgeTypeDef::set_destroyed_form);
	ClassDB::bind_method(D_METHOD("get_destroyed_form"), &HexEdgeTypeDef::get_destroyed_form);
	ClassDB::bind_method(D_METHOD("set_required_key_id", "key"), &HexEdgeTypeDef::set_required_key_id);
	ClassDB::bind_method(D_METHOD("get_required_key_id"), &HexEdgeTypeDef::get_required_key_id);
	ClassDB::bind_method(D_METHOD("set_enabled_by_default", "enabled"), &HexEdgeTypeDef::set_enabled_by_default);
	ClassDB::bind_method(D_METHOD("get_enabled_by_default"), &HexEdgeTypeDef::get_enabled_by_default);
	ClassDB::bind_method(D_METHOD("is_destructible"), &HexEdgeTypeDef::is_destructible);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexEdgeTypeDef::to_dict);
	ClassDB::bind_static_method("HexEdgeTypeDef", D_METHOD("from_dict", "dict"), &HexEdgeTypeDef::from_dict);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "action_type"), "set_action_type", "get_action_type");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "directional"), "set_directional", "get_directional");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "blocks_movement"), "set_blocks_movement", "get_blocks_movement");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "blocks_vision"), "set_blocks_vision", "get_blocks_vision");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "climb_height"), "set_climb_height", "get_climb_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "durability"), "set_durability", "get_durability");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "destroyed_form"), "set_destroyed_form", "get_destroyed_form");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "required_key_id"), "set_required_key_id", "get_required_key_id");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled_by_default"), "set_enabled_by_default", "get_enabled_by_default");
}

void HexEdgeTypeDef::set_action_type(const StringName &p_action) { action_type = p_action; }
StringName HexEdgeTypeDef::get_action_type() const { return action_type; }
void HexEdgeTypeDef::set_directional(bool p_enabled) { directional = p_enabled; }
bool HexEdgeTypeDef::get_directional() const { return directional; }
void HexEdgeTypeDef::set_blocks_movement(bool p_enabled) { blocks_movement = p_enabled; }
bool HexEdgeTypeDef::get_blocks_movement() const { return blocks_movement; }
void HexEdgeTypeDef::set_blocks_vision(bool p_enabled) { blocks_vision = p_enabled; }
bool HexEdgeTypeDef::get_blocks_vision() const { return blocks_vision; }
void HexEdgeTypeDef::set_climb_height(int p_height) { climb_height = p_height; }
int HexEdgeTypeDef::get_climb_height() const { return climb_height; }
void HexEdgeTypeDef::set_durability(int p_durability) { durability = p_durability; }
int HexEdgeTypeDef::get_durability() const { return durability; }
void HexEdgeTypeDef::set_destroyed_form(const StringName &p_form) { destroyed_form = p_form; }
StringName HexEdgeTypeDef::get_destroyed_form() const { return destroyed_form; }
void HexEdgeTypeDef::set_required_key_id(const StringName &p_key) { required_key_id = p_key; }
StringName HexEdgeTypeDef::get_required_key_id() const { return required_key_id; }
void HexEdgeTypeDef::set_enabled_by_default(bool p_enabled) { enabled_by_default = p_enabled; }
bool HexEdgeTypeDef::get_enabled_by_default() const { return enabled_by_default; }
bool HexEdgeTypeDef::is_destructible() const { return durability >= 0; }

Dictionary HexEdgeTypeDef::to_dict() const {
	Dictionary d = base_to_dict();
	d[StringName("action_type")] = action_type;
	d[StringName("directional")] = directional;
	d[StringName("blocks_movement")] = blocks_movement;
	d[StringName("blocks_vision")] = blocks_vision;
	d[StringName("climb_height")] = climb_height;
	d[StringName("durability")] = durability;
	d[StringName("destroyed_form")] = destroyed_form;
	d[StringName("required_key_id")] = required_key_id;
	d[StringName("enabled_by_default")] = enabled_by_default;
	return d;
}

Ref<HexEdgeTypeDef> HexEdgeTypeDef::from_dict(const Dictionary &p_dict) {
	Ref<HexEdgeTypeDef> d;
	d.instantiate();
	d->base_from_dict(p_dict);
	d->set_action_type(p_dict.get(StringName("action_type"), StringName("walk")));
	d->set_directional(p_dict.get(StringName("directional"), false));
	d->set_blocks_movement(p_dict.get(StringName("blocks_movement"), true));
	d->set_blocks_vision(p_dict.get(StringName("blocks_vision"), false));
	d->set_climb_height(p_dict.get(StringName("climb_height"), 0));
	d->set_durability(p_dict.get(StringName("durability"), -1));
	d->set_destroyed_form(p_dict.get(StringName("destroyed_form"), StringName()));
	d->set_required_key_id(p_dict.get(StringName("required_key_id"), StringName()));
	d->set_enabled_by_default(p_dict.get(StringName("enabled_by_default"), true));
	return d;
}

/**************************************************************************/
/* HexAreaEffectDef                                                       */
/**************************************************************************/

void HexAreaEffectDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_duration_ticks", "ticks"), &HexAreaEffectDef::set_duration_ticks);
	ClassDB::bind_method(D_METHOD("get_duration_ticks"), &HexAreaEffectDef::get_duration_ticks);
	ClassDB::bind_method(D_METHOD("set_spread", "enabled"), &HexAreaEffectDef::set_spread);
	ClassDB::bind_method(D_METHOD("get_spread"), &HexAreaEffectDef::get_spread);
	ClassDB::bind_method(D_METHOD("set_blocks_entry", "enabled"), &HexAreaEffectDef::set_blocks_entry);
	ClassDB::bind_method(D_METHOD("get_blocks_entry"), &HexAreaEffectDef::get_blocks_entry);
	ClassDB::bind_method(D_METHOD("set_vision_modifier", "modifier"), &HexAreaEffectDef::set_vision_modifier);
	ClassDB::bind_method(D_METHOD("get_vision_modifier"), &HexAreaEffectDef::get_vision_modifier);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexAreaEffectDef::to_dict);
	ClassDB::bind_static_method("HexAreaEffectDef", D_METHOD("from_dict", "dict"), &HexAreaEffectDef::from_dict);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "duration_ticks"), "set_duration_ticks", "get_duration_ticks");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "spread"), "set_spread", "get_spread");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "blocks_entry"), "set_blocks_entry", "get_blocks_entry");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "vision_modifier"), "set_vision_modifier", "get_vision_modifier");
}

void HexAreaEffectDef::set_duration_ticks(int p_ticks) { duration_ticks = p_ticks; }
int HexAreaEffectDef::get_duration_ticks() const { return duration_ticks; }
void HexAreaEffectDef::set_spread(bool p_enabled) { spread = p_enabled; }
bool HexAreaEffectDef::get_spread() const { return spread; }
void HexAreaEffectDef::set_blocks_entry(bool p_enabled) { blocks_entry = p_enabled; }
bool HexAreaEffectDef::get_blocks_entry() const { return blocks_entry; }
void HexAreaEffectDef::set_vision_modifier(double p_modifier) { vision_modifier = p_modifier; }
double HexAreaEffectDef::get_vision_modifier() const { return vision_modifier; }

Dictionary HexAreaEffectDef::to_dict() const {
	Dictionary d = base_to_dict();
	d[StringName("duration_ticks")] = duration_ticks;
	d[StringName("spread")] = spread;
	d[StringName("blocks_entry")] = blocks_entry;
	d[StringName("vision_modifier")] = vision_modifier;
	return d;
}

Ref<HexAreaEffectDef> HexAreaEffectDef::from_dict(const Dictionary &p_dict) {
	Ref<HexAreaEffectDef> d;
	d.instantiate();
	d->base_from_dict(p_dict);
	d->set_duration_ticks(p_dict.get(StringName("duration_ticks"), -1));
	d->set_spread(p_dict.get(StringName("spread"), false));
	d->set_blocks_entry(p_dict.get(StringName("blocks_entry"), false));
	d->set_vision_modifier(p_dict.get(StringName("vision_modifier"), 0.0));
	return d;
}

/**************************************************************************/
/* HexReactionDef                                                         */
/**************************************************************************/

void HexReactionDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_trigger_event", "event"), &HexReactionDef::set_trigger_event);
	ClassDB::bind_method(D_METHOD("get_trigger_event"), &HexReactionDef::get_trigger_event);
	ClassDB::bind_method(D_METHOD("set_source_tags", "tags"), &HexReactionDef::set_source_tags);
	ClassDB::bind_method(D_METHOD("get_source_tags"), &HexReactionDef::get_source_tags);
	ClassDB::bind_method(D_METHOD("set_target_tags", "tags"), &HexReactionDef::set_target_tags);
	ClassDB::bind_method(D_METHOD("get_target_tags"), &HexReactionDef::get_target_tags);
	ClassDB::bind_method(D_METHOD("set_result_surface", "surface"), &HexReactionDef::set_result_surface);
	ClassDB::bind_method(D_METHOD("get_result_surface"), &HexReactionDef::get_result_surface);
	ClassDB::bind_method(D_METHOD("set_result_effect", "effect"), &HexReactionDef::set_result_effect);
	ClassDB::bind_method(D_METHOD("get_result_effect"), &HexReactionDef::get_result_effect);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexReactionDef::to_dict);
	ClassDB::bind_static_method("HexReactionDef", D_METHOD("from_dict", "dict"), &HexReactionDef::from_dict);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "trigger_event"), "set_trigger_event", "get_trigger_event");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "source_tags"), "set_source_tags", "get_source_tags");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "target_tags"), "set_target_tags", "get_target_tags");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "result_surface"), "set_result_surface", "get_result_surface");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "result_effect"), "set_result_effect", "get_result_effect");
}

void HexReactionDef::set_trigger_event(const StringName &p_event) { trigger_event = p_event; }
StringName HexReactionDef::get_trigger_event() const { return trigger_event; }
void HexReactionDef::set_source_tags(const PackedStringArray &p_tags) { source_tags = p_tags; }
PackedStringArray HexReactionDef::get_source_tags() const { return source_tags; }
void HexReactionDef::set_target_tags(const PackedStringArray &p_tags) { target_tags = p_tags; }
PackedStringArray HexReactionDef::get_target_tags() const { return target_tags; }
void HexReactionDef::set_result_surface(const StringName &p_surface) { result_surface = p_surface; }
StringName HexReactionDef::get_result_surface() const { return result_surface; }
void HexReactionDef::set_result_effect(const StringName &p_effect) { result_effect = p_effect; }
StringName HexReactionDef::get_result_effect() const { return result_effect; }

Dictionary HexReactionDef::to_dict() const {
	Dictionary d = base_to_dict();
	d[StringName("trigger_event")] = trigger_event;
	d[StringName("source_tags")] = source_tags;
	d[StringName("target_tags")] = target_tags;
	d[StringName("result_surface")] = result_surface;
	d[StringName("result_effect")] = result_effect;
	return d;
}

Ref<HexReactionDef> HexReactionDef::from_dict(const Dictionary &p_dict) {
	Ref<HexReactionDef> d;
	d.instantiate();
	d->base_from_dict(p_dict);
	d->set_trigger_event(p_dict.get(StringName("trigger_event"), StringName("applied")));
	d->set_source_tags(p_dict.get(StringName("source_tags"), PackedStringArray()));
	d->set_target_tags(p_dict.get(StringName("target_tags"), PackedStringArray()));
	d->set_result_surface(p_dict.get(StringName("result_surface"), StringName()));
	d->set_result_effect(p_dict.get(StringName("result_effect"), StringName()));
	return d;
}
