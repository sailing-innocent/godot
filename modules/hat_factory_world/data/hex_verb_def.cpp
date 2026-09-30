/**************************************************************************/
/*  hex_verb_def.cpp                                                      */
/**************************************************************************/
#include "hex_verb_def.h"

#include "core/object/class_db.h"

void HexVerbDef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_verb_id", "id"), &HexVerbDef::set_verb_id);
	ClassDB::bind_method(D_METHOD("get_verb_id"), &HexVerbDef::get_verb_id);
	ClassDB::bind_method(D_METHOD("set_display_name", "name"), &HexVerbDef::set_display_name);
	ClassDB::bind_method(D_METHOD("get_display_name"), &HexVerbDef::get_display_name);
	ClassDB::bind_method(D_METHOD("set_target_kind", "kind"), &HexVerbDef::set_target_kind);
	ClassDB::bind_method(D_METHOD("get_target_kind"), &HexVerbDef::get_target_kind);
	ClassDB::bind_method(D_METHOD("set_required_hat_tags", "tags"), &HexVerbDef::set_required_hat_tags);
	ClassDB::bind_method(D_METHOD("get_required_hat_tags"), &HexVerbDef::get_required_hat_tags);
	ClassDB::bind_method(D_METHOD("set_required_item_id", "item"), &HexVerbDef::set_required_item_id);
	ClassDB::bind_method(D_METHOD("get_required_item_id"), &HexVerbDef::get_required_item_id);
	ClassDB::bind_method(D_METHOD("set_required_key_id", "key"), &HexVerbDef::set_required_key_id);
	ClassDB::bind_method(D_METHOD("get_required_key_id"), &HexVerbDef::get_required_key_id);
	ClassDB::bind_method(D_METHOD("set_ap_cost", "cost"), &HexVerbDef::set_ap_cost);
	ClassDB::bind_method(D_METHOD("get_ap_cost"), &HexVerbDef::get_ap_cost);
	ClassDB::bind_method(D_METHOD("set_time_ms", "ms"), &HexVerbDef::set_time_ms);
	ClassDB::bind_method(D_METHOD("get_time_ms"), &HexVerbDef::get_time_ms);
	ClassDB::bind_method(D_METHOD("set_command_template", "template"), &HexVerbDef::set_command_template);
	ClassDB::bind_method(D_METHOD("get_command_template"), &HexVerbDef::get_command_template);
	ClassDB::bind_method(D_METHOD("to_dict"), &HexVerbDef::to_dict);
	ClassDB::bind_static_method("HexVerbDef", D_METHOD("from_dict", "dict"), &HexVerbDef::from_dict);
	ClassDB::bind_method(D_METHOD("validate"), &HexVerbDef::validate);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "verb_id"), "set_verb_id", "get_verb_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "display_name"), "set_display_name", "get_display_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "target_kind"), "set_target_kind", "get_target_kind");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "required_hat_tags"), "set_required_hat_tags", "get_required_hat_tags");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "required_item_id"), "set_required_item_id", "get_required_item_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "required_key_id"), "set_required_key_id", "get_required_key_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ap_cost"), "set_ap_cost", "get_ap_cost");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "time_ms"), "set_time_ms", "get_time_ms");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "command_template"), "set_command_template", "get_command_template");
}

void HexVerbDef::set_verb_id(const StringName &p_id) { verb_id = p_id; }
StringName HexVerbDef::get_verb_id() const { return verb_id; }
void HexVerbDef::set_display_name(const String &p_name) { display_name = p_name; }
String HexVerbDef::get_display_name() const { return display_name; }
void HexVerbDef::set_target_kind(const StringName &p_kind) { target_kind = p_kind; }
StringName HexVerbDef::get_target_kind() const { return target_kind; }
void HexVerbDef::set_required_hat_tags(const PackedStringArray &p_tags) { required_hat_tags = p_tags; }
PackedStringArray HexVerbDef::get_required_hat_tags() const { return required_hat_tags; }
void HexVerbDef::set_required_item_id(const StringName &p_item) { required_item_id = p_item; }
StringName HexVerbDef::get_required_item_id() const { return required_item_id; }
void HexVerbDef::set_required_key_id(const StringName &p_key) { required_key_id = p_key; }
StringName HexVerbDef::get_required_key_id() const { return required_key_id; }
void HexVerbDef::set_ap_cost(int p_cost) { ap_cost = p_cost; }
int HexVerbDef::get_ap_cost() const { return ap_cost; }
void HexVerbDef::set_time_ms(const int p_ms) { time_ms = p_ms; }
int HexVerbDef::get_time_ms() const { return time_ms; }
void HexVerbDef::set_command_template(const Dictionary &p_template) { command_template = p_template; }
Dictionary HexVerbDef::get_command_template() const { return command_template; }

Dictionary HexVerbDef::to_dict() const {
	Dictionary d;
	d[StringName("verb_id")] = verb_id;
	d[StringName("display_name")] = display_name;
	d[StringName("target_kind")] = target_kind;
	d[StringName("required_hat_tags")] = required_hat_tags;
	d[StringName("required_item_id")] = required_item_id;
	d[StringName("required_key_id")] = required_key_id;
	d[StringName("ap_cost")] = ap_cost;
	d[StringName("time_ms")] = time_ms;
	d[StringName("command_template")] = command_template;
	return d;
}

Ref<HexVerbDef> HexVerbDef::from_dict(const Dictionary &p_dict) {
	Ref<HexVerbDef> v;
	v.instantiate();
	v->set_verb_id(p_dict.get(StringName("verb_id"), StringName()));
	v->set_display_name(p_dict.get(StringName("display_name"), String()));
	v->set_target_kind(p_dict.get(StringName("target_kind"), StringName("cell")));
	v->set_required_hat_tags(p_dict.get(StringName("required_hat_tags"), PackedStringArray()));
	v->set_required_item_id(p_dict.get(StringName("required_item_id"), StringName()));
	v->set_required_key_id(p_dict.get(StringName("required_key_id"), StringName()));
	v->set_ap_cost(p_dict.get(StringName("ap_cost"), 0));
	v->set_time_ms(p_dict.get(StringName("time_ms"), 0));
	v->set_command_template(p_dict.get(StringName("command_template"), Dictionary()));
	return v;
}

String HexVerbDef::validate() const {
	if (verb_id == StringName()) {
		return "verb missing verb_id";
	}
	if (target_kind != StringName("cell") && target_kind != StringName("edge") && target_kind != StringName("object")) {
		return "verb '" + String(verb_id) + "' has unknown target_kind '" + String(target_kind) + "'";
	}
	if (ap_cost < 0 || time_ms < 0) {
		return "verb '" + String(verb_id) + "' has negative cost";
	}
	if (!command_template.has(StringName("operation"))) {
		return "verb '" + String(verb_id) + "' command_template missing 'operation'";
	}
	return String();
}
