/**************************************************************************/
/*  display_server_dsh.cpp                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifdef WINDOWS_ENABLED

#include "display_server_dsh.h"

#include "core/config/engine.h"
#include "core/variant/dictionary.h"

#if defined(RD_ENABLED) && defined(DSH_VIEWPORT_ENABLED)

#include "dsh_viewport_worker.h"

#include "core/input/input.h"
#include "main/dsh_viewport_mode.h"
#include "servers/display/native_menu.h"
#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"

Vector<String> DisplayServerDSH::get_rendering_drivers_func() {
	Vector<String> drivers;
#ifdef VULKAN_ENABLED
	drivers.push_back("vulkan");
#endif
	return drivers;
}

DisplayServer *DisplayServerDSH::create_func(const String &p_rendering_driver, DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, DisplayServerEnums::Context p_context, int64_t p_parent_window, Error &r_error) {
	DisplayServerDSH *ds = memnew(DisplayServerDSH);
	r_error = OK;

	DSHViewportWorker *worker = memnew(DSHViewportWorker);
	ds->worker = worker;
	Error err = worker->initialize(DSHViewportMode::broker_pid);
	if (err == OK) {
		ds->rendering_context = memnew(RenderingContextDriverVulkanWindows);
		err = ds->rendering_context->initialize();
	}
	if (err == OK) {
		RenderingContextDriver::SurfaceID surface = ds->rendering_context->surface_create_dsh(worker);
		if (surface == 0) {
			err = ERR_CANT_CREATE;
		} else {
			ds->rendering_context->window_create_from_surface(DisplayServerEnums::MAIN_WINDOW_ID, surface);
			ds->rendering_device = memnew(RenderingDevice);
			err = ds->rendering_device->initialize(ds->rendering_context, DisplayServerEnums::MAIN_WINDOW_ID);
		}
	}
	if (err == OK) {
		err = ds->rendering_device->screen_create(DisplayServerEnums::MAIN_WINDOW_ID);
	}
	if (err != OK) {
		r_error = err;
		if (ds->worker) {
			memdelete(ds->worker);
			ds->worker = nullptr;
		}
		if (ds->rendering_device) {
			memdelete(ds->rendering_device);
			ds->rendering_device = nullptr;
		}
		if (ds->rendering_context) {
			memdelete(ds->rendering_context);
			ds->rendering_context = nullptr;
		}
		return ds;
	}

	WindowData &main_window = ds->windows[DisplayServerEnums::MAIN_WINDOW_ID];
	// Start suspended: the worker only produces frames once the broker
	// requests a non-zero viewport size (protocol resize).
	main_window.size = Size2i();
	main_window.visible = true;
	main_window.focused = true;
	main_window.z_order = 0;
	main_window.title = "DSH Godot";

	worker->set_display_server(ds);
	RendererCompositorRD::make_current();
	worker->start();
	worker->announce();
	return ds;
}

void DisplayServerDSH::register_dsh_driver() {
	register_create_function("dsh", create_func, get_rendering_drivers_func);
}

bool DisplayServerDSH::has_feature(DisplayServerEnums::Feature p_feature) const {
	switch (p_feature) {
		case DisplayServerEnums::FEATURE_SUBWINDOWS:
			// Protocol v2.1: real sub-windows, each with its own headless
			// surface, frame ring and client layer.
			return true;
		case DisplayServerEnums::FEATURE_MOUSE:
			return true;
		case DisplayServerEnums::FEATURE_CLIPBOARD:
			return true;
		case DisplayServerEnums::FEATURE_HIDPI:
			return true;
		case DisplayServerEnums::FEATURE_WINDOW_TRANSPARENCY:
			// RGBA8 carries alpha; the client composites popup layers.
			return true;
		case DisplayServerEnums::FEATURE_WINDOW_EMBEDDING:
			// The GameView embed backend is provided by EmbeddedProcessDSH
			// (editor stage only; runtime games run as their own session).
			return Engine::get_singleton()->is_editor_hint();
		default:
			return false;
	}
}

void DisplayServerDSH::_dispatch_input_events(const Ref<InputEvent> &p_event) {
	static_cast<DisplayServerDSH *>(get_singleton())->_dispatch_input_event(p_event);
}

void DisplayServerDSH::_dispatch_input_event(const Ref<InputEvent> &p_event) {
	DisplayServerEnums::WindowID window_id = DisplayServerEnums::MAIN_WINDOW_ID;
	Ref<InputEventFromWindow> from_window = p_event;
	if (from_window.is_valid() && from_window->get_window_id() != DisplayServerEnums::MAIN_WINDOW_ID) {
		window_id = from_window->get_window_id();
	}
	Ref<InputEventMouse> mouse = p_event;
	if (mouse.is_valid() && hovered_window != window_id) {
		const WindowData *previous = windows.getptr(hovered_window);
		if (previous && previous->window_event_callback.is_valid()) {
			previous->window_event_callback.call(DisplayServerEnums::WINDOW_EVENT_MOUSE_EXIT);
		}
		hovered_window = window_id;
		const WindowData *current = windows.getptr(window_id);
		if (current && current->window_event_callback.is_valid()) {
			current->window_event_callback.call(DisplayServerEnums::WINDOW_EVENT_MOUSE_ENTER);
		}
	}
	const WindowData *wd = windows.getptr(window_id);
	if (wd && wd->input_event_callback.is_valid()) {
		wd->input_event_callback.call(p_event);
	} else {
		const WindowData *main_wd = windows.getptr(DisplayServerEnums::MAIN_WINDOW_ID);
		if (main_wd && main_wd->input_event_callback.is_valid()) {
			main_wd->input_event_callback.call(p_event);
		}
	}
}

Size2i DisplayServerDSH::screen_get_size(int p_screen) const {
	const WindowData *wd = windows.getptr(DisplayServerEnums::MAIN_WINDOW_ID);
	return wd ? wd->size : Size2i();
}

Vector<DisplayServerEnums::WindowID> DisplayServerDSH::get_window_list() const {
	Vector<DisplayServerEnums::WindowID> list;
	for (const KeyValue<DisplayServerEnums::WindowID, WindowData> &kv : windows) {
		list.push_back(kv.key);
	}
	return list;
}

DisplayServerEnums::WindowID DisplayServerDSH::create_sub_window(DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::VSyncMode p_vsync_mode, uint32_t p_flags, const Rect2i &p_rect, bool p_exclusive, DisplayServerEnums::WindowID p_transient_parent) {
	ERR_FAIL_COND_V(p_rect.size.width < 0 || p_rect.size.height < 0, DisplayServerEnums::INVALID_WINDOW_ID);
	DisplayServerEnums::WindowID id = window_id_counter++;
	ERR_FAIL_COND_V_MSG(id >= (DisplayServerEnums::WindowID)DSHViewportWorker::MAX_WINDOWS,
			DisplayServerEnums::INVALID_WINDOW_ID,
			"DSH display server: sub-window quota (15) exceeded; the popup was rejected.");

	WindowData &wd = windows[id];
	wd.mode = p_mode;
	wd.size = p_rect.size;
	wd.position = p_rect.position;
	wd.visible = false;
	wd.flags = p_flags;
	wd.transient_parent = p_transient_parent;
	wd.z_order = ++z_counter;

#if defined(RD_ENABLED)
	if (rendering_context && rendering_device && worker) {
		// Protocol v2.1: a real headless surface + per-window frame ring, so the
		// sub-window renders and presents exactly like the main window.
		DSHWindowSink *sink = worker->dsh_create_window_sink((uint32_t)id);
		if (sink == nullptr) {
			windows.erase(id);
			return DisplayServerEnums::INVALID_WINDOW_ID;
		}
		RenderingContextDriver::SurfaceID surface = rendering_context->surface_create_dsh(sink);
		if (surface != 0) {
			rendering_context->window_create_from_surface(id, surface);
			rendering_context->window_set_size(id, (uint32_t)wd.size.width, (uint32_t)wd.size.height);
			if (rendering_device->screen_create(id) == OK) {
				Dictionary payload;
				payload["window"] = (int64_t)id;
				Dictionary rect;
				rect["x"] = wd.position.x;
				rect["y"] = wd.position.y;
				rect["width"] = wd.size.x;
				rect["height"] = wd.size.y;
				payload["rect"] = rect;
				payload["flags"] = (int64_t)p_flags;
				payload["transientParent"] = p_transient_parent == DisplayServerEnums::INVALID_WINDOW_ID ? Variant() : Variant((int64_t)p_transient_parent);
				payload["z"] = wd.z_order;
				payload["transparent"] = (p_flags & DisplayServerEnums::WINDOW_FLAG_TRANSPARENT_BIT) != 0;
				worker->emit_window_event("created", payload);
				return id;
			}
		}
		// Roll back a partial surface setup; the caller falls back to no window.
		worker->dsh_destroy_window_sink((uint32_t)id);
		if (surface != 0) {
			rendering_context->window_destroy(id);
		}
		windows.erase(id);
		return DisplayServerEnums::INVALID_WINDOW_ID;
	}
#endif
	return id;
}

void DisplayServerDSH::show_window(DisplayServerEnums::WindowID p_id) {
	WindowData *wd = windows.getptr(p_id);
	if (!wd) {
		return;
	}
	wd->visible = true;
	if (worker && p_id != DisplayServerEnums::MAIN_WINDOW_ID) {
		Dictionary payload;
		payload["window"] = (int64_t)p_id;
		payload["visible"] = true;
		worker->emit_window_event("visibility", payload);
	}
	// Popups take keyboard focus when shown (same contract as a real OS menu or
	// dialog); regular windows wait for an explicit foreground request or a
	// client focus click.
	if (wd->flags & DisplayServerEnums::WINDOW_FLAG_POPUP_BIT) {
		dsh_notify_engine_focus((uint32_t)p_id);
	}
}

void DisplayServerDSH::delete_sub_window(DisplayServerEnums::WindowID p_id) {
	if (p_id == DisplayServerEnums::MAIN_WINDOW_ID) {
		return;
	}
	WindowData *wd = windows.getptr(p_id);
	if (!wd) {
		return;
	}
	if (hovered_window == p_id) {
		if (wd->window_event_callback.is_valid()) {
			wd->window_event_callback.call(DisplayServerEnums::WINDOW_EVENT_MOUSE_EXIT);
		}
		hovered_window = DisplayServerEnums::INVALID_WINDOW_ID;
	}
#if defined(RD_ENABLED)
	if (rendering_device) {
		rendering_device->screen_free(p_id);
	}
	if (rendering_context) {
		rendering_context->window_destroy(p_id);
	}
#endif
	if (worker) {
		worker->dsh_destroy_window_sink((uint32_t)p_id);
		Dictionary payload;
		payload["window"] = (int64_t)p_id;
		worker->emit_window_event("destroyed", payload);
	}
	windows.erase(p_id);
}

void DisplayServerDSH::window_attach_instance_id(ObjectID p_instance, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (wd) {
		wd->instance_id = p_instance;
	}
}

ObjectID DisplayServerDSH::window_get_attached_instance_id(DisplayServerEnums::WindowID p_window) const {
	const WindowData *wd = windows.getptr(p_window);
	return wd ? wd->instance_id : ObjectID();
}

void DisplayServerDSH::window_set_rect_changed_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (wd) {
		wd->rect_changed_callback = p_callable;
	}
}

void DisplayServerDSH::window_set_window_event_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (wd) {
		wd->window_event_callback = p_callable;
	}
}

void DisplayServerDSH::window_set_input_event_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (wd) {
		wd->input_event_callback = p_callable;
	}
}

void DisplayServerDSH::window_set_input_text_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window) {
	// The DSH text/IME pipeline rides the protocol input channel (text/ime
	// control messages); there is no separate OS-level text callback.
	(void)p_callable;
	(void)p_window;
}

void DisplayServerDSH::window_set_drop_files_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (wd) {
		wd->drop_files_callback = p_callable;
	}
}

void DisplayServerDSH::dsh_dispatch_drop_files(const Vector<String> &p_files, DisplayServerEnums::WindowID p_window) {
	const WindowData *wd = windows.getptr(p_window);
	if (wd && wd->drop_files_callback.is_valid()) {
		wd->drop_files_callback.call(p_files);
		return;
	}
	const WindowData *main_wd = windows.getptr(DisplayServerEnums::MAIN_WINDOW_ID);
	if (main_wd && main_wd->drop_files_callback.is_valid()) {
		main_wd->drop_files_callback.call(p_files);
	}
}

DisplayServerEnums::WindowID DisplayServerDSH::get_window_at_screen_position(const Point2i &p_position) const {
	// Virtual-screen hit test in z order (topmost first); the main window is
	// the implicit background layer.
	DisplayServerEnums::WindowID best = DisplayServerEnums::MAIN_WINDOW_ID;
	int best_z = INT32_MIN;
	const WindowData *main_wd = windows.getptr(DisplayServerEnums::MAIN_WINDOW_ID);
	if (main_wd) {
		best_z = main_wd->z_order;
	}
	for (const KeyValue<DisplayServerEnums::WindowID, WindowData> &kv : windows) {
		if (kv.key == DisplayServerEnums::MAIN_WINDOW_ID || !kv.value.visible) {
			continue;
		}
		const Rect2i rect(kv.value.position, kv.value.size);
		if (rect.has_point(p_position) && kv.value.z_order >= best_z) {
			best = kv.key;
			best_z = kv.value.z_order;
		}
	}
	return best;
}

Point2i DisplayServerDSH::window_get_position(DisplayServerEnums::WindowID p_window) const {
	const WindowData *wd = windows.getptr(p_window);
	return wd ? wd->position : Point2i();
}

void DisplayServerDSH::_emit_window_rect(DisplayServerEnums::WindowID p_window) {
	const WindowData *wd = windows.getptr(p_window);
	if (!wd || !worker) {
		return;
	}
	Dictionary payload;
	payload["window"] = (int64_t)p_window;
	Dictionary rect;
	rect["x"] = wd->position.x;
	rect["y"] = wd->position.y;
	rect["width"] = wd->size.x;
	rect["height"] = wd->size.y;
	payload["rect"] = rect;
	worker->emit_window_event("rect", payload);
}

void DisplayServerDSH::window_set_position(const Point2i &p_position, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (wd) {
		wd->position = p_position;
		_emit_window_rect(p_window);
	}
}

void DisplayServerDSH::window_set_size(const Size2i p_size, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	ERR_FAIL_NULL(wd);
	if (wd->size == p_size) {
		return;
	}
	wd->size = p_size;
	if (rendering_context) {
		rendering_context->window_set_size(p_window, (uint32_t)p_size.width, (uint32_t)p_size.height);
	}
	if (wd->rect_changed_callback.is_valid()) {
		wd->rect_changed_callback.call(Rect2i(wd->position, wd->size));
	}
	if (p_window != DisplayServerEnums::MAIN_WINDOW_ID) {
		_emit_window_rect(p_window);
	}
}

Size2i DisplayServerDSH::window_get_size(DisplayServerEnums::WindowID p_window) const {
	const WindowData *wd = windows.getptr(p_window);
	return wd ? wd->size : Size2i();
}

void DisplayServerDSH::window_set_title(const String &p_title, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (!wd) {
		return;
	}
	wd->title = p_title;
	if (worker && p_window != DisplayServerEnums::MAIN_WINDOW_ID) {
		Dictionary payload;
		payload["window"] = (int64_t)p_window;
		payload["title"] = p_title;
		worker->emit_window_event("title", payload);
	}
}

void DisplayServerDSH::window_set_transient(DisplayServerEnums::WindowID p_window, DisplayServerEnums::WindowID p_parent) {
	WindowData *wd = windows.getptr(p_window);
	if (wd) {
		wd->transient_parent = p_parent;
	}
}

void DisplayServerDSH::window_set_flag(DisplayServerEnums::WindowFlags p_flag, bool p_enabled, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (!wd) {
		return;
	}
	if (p_enabled) {
		wd->flags |= (1u << (int)p_flag);
	} else {
		wd->flags &= ~(1u << (int)p_flag);
	}
}

bool DisplayServerDSH::window_get_flag(DisplayServerEnums::WindowFlags p_flag, DisplayServerEnums::WindowID p_window) const {
	const WindowData *wd = windows.getptr(p_window);
	return wd ? (wd->flags & (1u << (int)p_flag)) != 0 : false;
}

void DisplayServerDSH::window_set_mode(DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (!wd) {
		return;
	}
	// Hidden windows are represented by the engine destroying the sub-window
	// (Window::_clear_window -> delete_sub_window), so modes only track state.
	wd->mode = p_mode;
	wd->minimized = (p_mode == DisplayServerEnums::WINDOW_MODE_MINIMIZED);
}

DisplayServerEnums::WindowMode DisplayServerDSH::window_get_mode(DisplayServerEnums::WindowID p_window) const {
	const WindowData *wd = windows.getptr(p_window);
	return wd ? wd->mode : DisplayServerEnums::WINDOW_MODE_WINDOWED;
}

void DisplayServerDSH::window_move_to_foreground(DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (!wd) {
		return;
	}
	wd->z_order = ++z_counter;
	if (worker) {
		Dictionary payload;
		payload["window"] = (int64_t)p_window;
		payload["z"] = wd->z_order;
		worker->emit_window_event("raised", payload);
	}
	dsh_notify_engine_focus((uint32_t)p_window);
}

void DisplayServerDSH::dsh_apply_window_focus(uint32_t p_window) {
	// Single source of truth for window focus: exactly one window (or none)
	// carries the focused flag, and every transition fires the engine's
	// WINDOW_EVENT_FOCUS_IN/OUT sequence like a real OS would.
	for (KeyValue<DisplayServerEnums::WindowID, WindowData> &kv : windows) {
		const bool should_focus = (uint32_t)kv.key == p_window;
		if (kv.value.focused == should_focus) {
			continue;
		}
		kv.value.focused = should_focus;
		if (kv.value.window_event_callback.is_valid()) {
			kv.value.window_event_callback.call(should_focus ? DisplayServerEnums::WINDOW_EVENT_FOCUS_IN : DisplayServerEnums::WINDOW_EVENT_FOCUS_OUT);
		}
	}
}

void DisplayServerDSH::dsh_notify_engine_focus(uint32_t p_window) {
	WindowData *wd = windows.getptr((DisplayServerEnums::WindowID)p_window);
	if (!wd) {
		return;
	}
	dsh_apply_window_focus(p_window);
	if (worker) {
		worker->dsh_notify_engine_focus(p_window, true);
	}
}

bool DisplayServerDSH::window_is_focused(DisplayServerEnums::WindowID p_window) const {
	const WindowData *wd = windows.getptr(p_window);
	return wd ? wd->focused : false;
}

bool DisplayServerDSH::window_can_draw(DisplayServerEnums::WindowID p_window) const {
	const WindowData *wd = windows.getptr(p_window);
	return wd && wd->visible && !wd->minimized && wd->size.width > 0 && wd->size.height > 0;
}

bool DisplayServerDSH::can_any_window_draw() const {
	// Frames are only produced once the broker has applied a non-zero resize
	// (surface + epoch) and while the broker reports the viewport visible
	// (hidden suspends production entirely, per protocol 2 semantics). Any
	// visible engine window (main or sub) keeps the draw loop alive.
	if (worker != nullptr && worker->has_applied_surface() && worker->is_viewport_visible() && window_can_draw(DisplayServerEnums::MAIN_WINDOW_ID)) {
		return true;
	}
	for (const KeyValue<DisplayServerEnums::WindowID, WindowData> &kv : windows) {
		if (kv.key != DisplayServerEnums::MAIN_WINDOW_ID && window_can_draw(kv.key)) {
			return true;
		}
	}
	return false;
}

void DisplayServerDSH::window_set_ime_active(const bool p_active, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (!wd) {
		return;
	}
	wd->ime_active = p_active;
	dsh_notify_ime_changed(p_window);
}

void DisplayServerDSH::window_set_ime_position(const Point2i &p_pos, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (!wd) {
		return;
	}
	wd->ime_position = p_pos;
	dsh_notify_ime_changed(p_window);
}

void DisplayServerDSH::dsh_notify_ime_changed(DisplayServerEnums::WindowID p_window) {
	if (worker == nullptr) {
		return;
	}
	const WindowData *wd = windows.getptr(p_window);
	if (!wd) {
		return;
	}
	// Engine-physical window coordinates; the client maps them back to CSS px
	// against the applied surface before positioning its hidden IME host, and
	// offsets them by the window's layer position (protocol v2.1).
	Dictionary payload;
	payload["active"] = wd->ime_active;
	payload["x"] = wd->ime_position.x;
	payload["y"] = wd->ime_position.y;
	payload["window"] = (int64_t)p_window;
	worker->emit_viewport_extension("imeState", payload);
}

String DisplayServerDSH::ime_get_text() const {
	uint32_t focused = worker ? worker->dsh_get_focused_window() : (uint32_t)DisplayServerEnums::MAIN_WINDOW_ID;
	if (focused == UINT32_MAX) {
		focused = (uint32_t)DisplayServerEnums::MAIN_WINDOW_ID;
	}
	const WindowData *wd = windows.getptr((DisplayServerEnums::WindowID)focused);
	return wd ? wd->ime_preedit_text : String();
}

Point2i DisplayServerDSH::ime_get_selection() const {
	uint32_t focused = worker ? worker->dsh_get_focused_window() : (uint32_t)DisplayServerEnums::MAIN_WINDOW_ID;
	if (focused == UINT32_MAX) {
		focused = (uint32_t)DisplayServerEnums::MAIN_WINDOW_ID;
	}
	const WindowData *wd = windows.getptr((DisplayServerEnums::WindowID)focused);
	return wd ? wd->ime_preedit_selection : Point2i();
}

void DisplayServerDSH::clipboard_set(const String &p_text) {
	clipboard_cache = p_text;
	if (worker) {
		// Mirror into the client so the system clipboard carries the copy.
		Dictionary payload;
		payload["text"] = p_text;
		worker->emit_viewport_extension("clipboardWrite", payload);
	}
}

String DisplayServerDSH::clipboard_get() const {
	return clipboard_cache;
}

void DisplayServerDSH::process_events() {
	if (worker) {
		worker->poll();
	}
	Input::get_singleton()->flush_buffered_events();
}

DisplayServerDSH::DisplayServerDSH() {
	native_menu = memnew(NativeMenu);
	Input::get_singleton()->set_event_dispatch_function(_dispatch_input_events);
}

DisplayServerDSH::~DisplayServerDSH() {
	if (native_menu) {
		memdelete(native_menu);
		native_menu = nullptr;
	}
	// The worker owns D3D11/Vulkan interop resources and must shut down
	// before the rendering device and context are destroyed.
	if (worker) {
		memdelete(worker);
		worker = nullptr;
	}
	if (rendering_context) {
		rendering_context->window_destroy(DisplayServerEnums::MAIN_WINDOW_ID);
	}
	if (rendering_device) {
		memdelete(rendering_device);
		rendering_device = nullptr;
	}
	if (rendering_context) {
		memdelete(rendering_context);
		rendering_context = nullptr;
	}
}

#endif // RD_ENABLED && DSH_VIEWPORT_ENABLED

#endif // WINDOWS_ENABLED
