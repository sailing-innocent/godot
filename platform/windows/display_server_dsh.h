/**************************************************************************/
/*  display_server_dsh.h                                                  */
/**************************************************************************/
/* This file is part of: */
/* GODOT ENGINE */
/* https://godotengine.org */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/* */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the */
/* "Software"), to deal in the Software without restriction, including */
/* without limitation the rights to use, copy, modify, merge, publish, */
/* distribute, sublicense, and/or sell copies of the Software, and to */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions: */
/* */
/* The above copyright notice and this permission notice shall be */
/* included in all copies or substantial portions of the Software. */
/* */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
/**************************************************************************/

#pragma once

#ifdef WINDOWS_ENABLED

#include "core/input/input_event.h"
#include "servers/display/display_server.h"

#if defined(RD_ENABLED)
#include "rendering_context_driver_vulkan_windows.h"

#include "servers/rendering/rendering_device.h"
#endif

class DSHViewportWorker;

class NativeMenu;

// Headless display server for the DSH (DeepSeek Harness) native viewport
// worker. There is no OS window: every window is a virtual rectangle whose
// "swapchain" is a ring of D3D11 shared textures exported to the DSH broker
// through wire protocol 2 (owned by DSHViewportWorker). Rendering keeps using
// the full Vulkan RenderingDevice path; only presentation is redirected.
//
// Protocol v2.1 (multi-window): FEATURE_SUBWINDOWS is real. Every sub-window
// gets its own headless surface, its own per-window frame ring and its own
// godot.window.v1 lifecycle/geometry events; the client composites layers.
class DisplayServerDSH : public DisplayServer {
	GDSOFTCLASS(DisplayServerDSH, DisplayServer);

	friend class DisplayServer;

	static Vector<String> get_rendering_drivers_func();
	static DisplayServer *create_func(const String &p_rendering_driver, DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, DisplayServerEnums::Context p_context, int64_t p_parent_window, Error &r_error);

	static void _dispatch_input_events(const Ref<InputEvent> &p_event);
	void _dispatch_input_event(const Ref<InputEvent> &p_event);

	struct WindowData {
		Size2i size;
		Point2i position;
		bool visible = false;
		bool focused = false;
		bool minimized = false;
		DisplayServerEnums::WindowMode mode = DisplayServerEnums::WINDOW_MODE_WINDOWED;
		uint32_t flags = 0;
		String title;
		int z_order = 0;
		DisplayServerEnums::WindowID transient_parent = DisplayServerEnums::INVALID_WINDOW_ID;
		Callable input_event_callback;
		Callable rect_changed_callback;
		Callable window_event_callback;
		Callable drop_files_callback;
		ObjectID instance_id;
		// Per-window IME surface (protocol v2.1): composition state is scoped to
		// the window owning the caret.
		bool ime_active = false;
		Point2i ime_position;
		String ime_preedit_text;
		Vector2i ime_preedit_selection; // Caret/selection range inside the preedit string.
	};

	HashMap<DisplayServerEnums::WindowID, WindowData> windows;
	DisplayServerEnums::WindowID window_id_counter = 1; // 0 is reserved for the main window.
	int z_counter = 0; // Monotonic raise order for the godot.window.v1 z events.
	Point2i mouse_position;
	DisplayServerEnums::WindowID hovered_window = DisplayServerEnums::INVALID_WINDOW_ID;

	// ---- interaction state owned by the DSH worker (no OS window behind us) ----
	DisplayServerEnums::MouseMode mouse_mode = DisplayServerEnums::MOUSE_MODE_VISIBLE;
	bool relative_mouse_mode = false; // godot.viewport.v1 setRelativeMouseMode.
	float content_scale = 1.0; // Physical px per logical (CSS) px; locked with each resize, Godot screen-scale semantics.
	String clipboard_cache; // Client-mirrored clipboard (godot.viewport.v1 clipboard bridge).

#if defined(RD_ENABLED)
	RenderingContextDriverVulkanWindows *rendering_context = nullptr;
	RenderingDevice *rendering_device = nullptr;
#endif
	DSHViewportWorker *worker = nullptr;
	NativeMenu *native_menu = nullptr; // The editor queries NativeMenu::get_singleton() unconditionally.

	void _emit_window_rect(DisplayServerEnums::WindowID p_window);

public:
	bool has_feature(DisplayServerEnums::Feature p_feature) const override;
	String get_name() const override { return "dsh"; }

	int get_screen_count() const override { return 1; }
	int get_primary_screen() const override { return 0; }
	Point2i screen_get_position(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override { return Point2i(); }
	Size2i screen_get_size(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override;
	Rect2i screen_get_usable_rect(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override { return Rect2i(Point2i(), screen_get_size(p_screen)); }
	int screen_get_dpi(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override { return (int)(96 * content_scale); }
	float screen_get_scale(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override { return content_scale; }
	float screen_get_max_scale() const override { return content_scale; }
	float screen_get_refresh_rate(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override { return SCREEN_REFRESH_RATE_FALLBACK; }
	void screen_set_orientation(DisplayServerEnums::ScreenOrientation p_orientation, int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) override {}
	void screen_set_keep_on(bool p_enable) override {}

	Vector<DisplayServerEnums::WindowID> get_window_list() const override;

	DisplayServerEnums::WindowID create_sub_window(DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::VSyncMode p_vsync_mode, uint32_t p_flags, const Rect2i &p_rect = Rect2i(), bool p_exclusive = false, DisplayServerEnums::WindowID p_transient_parent = DisplayServerEnums::INVALID_WINDOW_ID) override;
	void show_window(DisplayServerEnums::WindowID p_id) override;
	void delete_sub_window(DisplayServerEnums::WindowID p_id) override;

	DisplayServerEnums::WindowID get_window_at_screen_position(const Point2i &p_position) const override;

	void window_attach_instance_id(ObjectID p_instance, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	ObjectID window_get_attached_instance_id(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	void window_set_rect_changed_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	void window_set_window_event_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	void window_set_input_event_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	void window_set_input_text_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	void window_set_drop_files_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	// Delivers an OS file drop into the engine (Window::files_dropped chain).
	void dsh_dispatch_drop_files(const Vector<String> &p_files, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID);

	void window_set_title(const String &p_title, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;

	void window_set_mouse_passthrough(const Vector<Vector2> &p_region, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override {}

	int window_get_current_screen(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override { return 0; }
	void window_set_current_screen(int p_screen, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override {}

	Point2i window_get_position(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;
	Point2i window_get_position_with_decorations(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override { return window_get_position(p_window); }
	void window_set_position(const Point2i &p_position, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;

	void window_set_transient(DisplayServerEnums::WindowID p_window, DisplayServerEnums::WindowID p_parent) override;

	void window_set_max_size(const Size2i p_size, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override {}
	Size2i window_get_max_size(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override { return Size2i(); }
	void window_set_min_size(const Size2i p_size, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override {}
	Size2i window_get_min_size(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override { return Size2i(); }

	void window_set_size(const Size2i p_size, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	Size2i window_get_size(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;
	Size2i window_get_size_with_decorations(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override { return window_get_size(p_window); }

	void window_set_mode(DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	DisplayServerEnums::WindowMode window_get_mode(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	void window_set_vsync_mode(DisplayServerEnums::VSyncMode p_vsync_mode, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override {}
	DisplayServerEnums::VSyncMode window_get_vsync_mode(DisplayServerEnums::WindowID p_window) const override { return DisplayServerEnums::VSyncMode::VSYNC_DISABLED; }

	bool window_is_maximize_allowed(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override { return false; }

	void window_set_flag(DisplayServerEnums::WindowFlags p_flag, bool p_enabled, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	bool window_get_flag(DisplayServerEnums::WindowFlags p_flag, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	void window_request_attention(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override {}
	void window_set_taskbar_progress_value(float p_value, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override {}
	void window_set_taskbar_progress_state(DisplayServerEnums::ProgressState p_state, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override {}
	void window_move_to_foreground(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	bool window_is_focused(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	bool window_can_draw(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;
	bool can_any_window_draw() const override;

	bool is_window_transparency_available() const override { return true; } // RGBA8 carries alpha; the client composites (protocol v2.1).

	void window_set_ime_active(const bool p_active, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	void window_set_ime_position(const Point2i &p_pos, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	void window_set_ime_rect(const Rect2i &p_rect, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override {}
	// Forwards the current IME state to the client (imeState extension event),
	// so it can host a hidden editable element at the exact caret position.
	void dsh_notify_ime_changed(DisplayServerEnums::WindowID p_window);
	String ime_get_text() const override;
	Point2i ime_get_selection() const override;

	int64_t window_get_native_handle(DisplayServerEnums::HandleType p_handle_type, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override { return 0; }

	void process_events() override;

	void set_native_icon(const String &p_filename) override {}
	void set_icon(const Ref<Image> &p_icon) override {}

	void help_set_search_callbacks(const Callable &p_search_callback = Callable(), const Callable &p_action_callback = Callable()) override {}

	bool tts_is_speaking() const override { return false; }
	bool tts_is_paused() const override { return false; }
	TypedArray<Dictionary> tts_get_voices() const override { return TypedArray<Dictionary>(); }
	void tts_speak(const String &p_text, const String &p_voice, int p_volume = 50, float p_pitch = 1.0f, float p_rate = 1.0f, int64_t p_utterance_id = 0, bool p_interrupt = false) override {}
	void tts_pause() override {}
	void tts_resume() override {}
	void tts_stop() override {}

	void mouse_set_mode(DisplayServerEnums::MouseMode p_mode) override { mouse_mode = p_mode; }
	void mouse_set_mode_override(DisplayServerEnums::MouseMode p_mode) override {}
	void mouse_set_mode_override_enabled(bool p_override_enabled) override {}
	DisplayServerEnums::MouseMode mouse_get_mode() const override { return mouse_mode; }
	Point2i mouse_get_position() const override { return mouse_position; }
	void clipboard_set(const String &p_text) override;
	String clipboard_get() const override;
	void clipboard_set_primary(const String &p_text) override {}
	String clipboard_get_primary() const override { return String(); }

	void virtual_keyboard_show(const String &p_existing_text, const Rect2 &p_screen_rect = Rect2(), DisplayServerEnums::VirtualKeyboardType p_type = DisplayServerEnums::KEYBOARD_TYPE_DEFAULT, int p_max_length = -1, int p_cursor_start = -1, int p_cursor_end = -1) override {}
	void virtual_keyboard_hide() override {}

	void cursor_set_shape(DisplayServerEnums::CursorShape p_shape) override {}
	void cursor_set_custom_image(const Ref<Resource> &p_cursor, DisplayServerEnums::CursorShape p_shape = DisplayServerEnums::CURSOR_ARROW, const Vector2 &p_hotspot = Vector2()) override {}

	Error dialog_show(String p_title, String p_description, Vector<String> p_buttons, const Callable &p_callback) override { return ERR_UNAVAILABLE; }
	Error dialog_input_text(String p_title, String p_description, String p_partial, const Callable &p_callback) override { return ERR_UNAVAILABLE; }
	Error file_dialog_show(const String &p_title, const String &p_current_directory, const String &p_filename, bool p_show_hidden, DisplayServerEnums::FileDialogMode p_mode, const Vector<String> &p_filters, const Callable &p_callback, DisplayServerEnums::WindowID p_window_id) override { return ERR_UNAVAILABLE; }
	Error file_dialog_with_options_show(const String &p_title, const String &p_current_directory, const String &p_root, const String &p_filename, bool p_show_hidden, DisplayServerEnums::FileDialogMode p_mode, const Vector<String> &p_filters, const TypedArray<Dictionary> &p_options, const Callable &p_callback, DisplayServerEnums::WindowID p_window_id) override { return ERR_UNAVAILABLE; }

	void release_rendering_thread() override {}
	void swap_buffers() override {}

	DisplayServerEnums::IndicatorID create_status_indicator(const Ref<Texture2D> &p_icon, const String &p_tooltip, const Callable &p_callback) override { return DisplayServerEnums::INVALID_INDICATOR_ID; }
	void status_indicator_set_icon(DisplayServerEnums::IndicatorID p_id, const Ref<Texture2D> &p_icon) override {}
	void status_indicator_set_tooltip(DisplayServerEnums::IndicatorID p_id, const String &p_tooltip) override {}
	void status_indicator_set_callback(DisplayServerEnums::IndicatorID p_id, const Callable &p_callback) override {}
	void delete_status_indicator(DisplayServerEnums::IndicatorID p_id) override {}

#if defined(RD_ENABLED)
	RenderingDevice *get_rendering_device() const { return rendering_device; }
#endif
	DSHViewportWorker *get_worker() const { return worker; }

	void dsh_set_mouse_position(const Point2i &p_position) { mouse_position = p_position; }
	void dsh_set_relative_mouse_mode(bool p_enabled) { relative_mouse_mode = p_enabled; }
	bool dsh_is_relative_mouse_mode() const { return relative_mouse_mode; }
	void dsh_set_content_scale(float p_scale) { content_scale = CLAMP(p_scale, 0.25f, 8.0f); }
	float dsh_get_content_scale() const { return content_scale; }
	void dsh_set_ime_preedit(const String &p_text, int p_cursor_start, int p_cursor_end, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) {
		WindowData *wd = windows.getptr(p_window);
		if (!wd) {
			wd = windows.getptr(DisplayServerEnums::MAIN_WINDOW_ID);
		}
		if (!wd) {
			return;
		}
		wd->ime_preedit_text = p_text;
		const int length = p_text.length();
		// Selection range as (caret, length) like the IMM-driven display servers.
		const int caret = CLAMP(p_cursor_start, 0, length);
		const int end = p_cursor_end > caret ? CLAMP(p_cursor_end, caret, length) : length;
		wd->ime_preedit_selection = Vector2i(caret, end - caret);
	}

	// Focus orchestration (protocol v2.1): the worker's focus control and
	// engine-internal grabs both funnel through here so the FOCUS_IN/OUT window
	// event sequence has a single source of truth.
	void dsh_apply_window_focus(uint32_t p_window); // UINT32_MAX = no focused window.
	void dsh_notify_engine_focus(uint32_t p_window); // Engine grabbed focus internally (e.g. popup shown).
	void dsh_set_clipboard_text(const String &p_text) { clipboard_cache = p_text; }

	static void register_dsh_driver();

	DisplayServerDSH();
	~DisplayServerDSH() override;
};

#endif // WINDOWS_ENABLED
