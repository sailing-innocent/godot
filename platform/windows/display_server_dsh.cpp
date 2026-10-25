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

#include "core/variant/dictionary.h"

#if defined(RD_ENABLED) && defined(DSH_VIEWPORT_ENABLED)

#include "core/input/input.h"
#include "servers/display/native_menu.h"
#include "dsh_viewport_worker.h"
#include "main/dsh_viewport_mode.h"
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
	main_window.mode = p_mode;

	worker->set_display_server(ds);
	RendererCompositorRD::make_current();
	worker->start();
	worker->announce();
	return ds;
}

void DisplayServerDSH::register_dsh_driver() {
	register_create_function("dsh", create_func, get_rendering_drivers_func);
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
	DisplayServerEnums::WindowID id = window_id_counter++;
	WindowData &wd = windows[id];
	wd.mode = p_mode;
	wd.size = p_rect.size;
	wd.position = p_rect.position;
	wd.visible = false;
	return id;
}

void DisplayServerDSH::show_window(DisplayServerEnums::WindowID p_id) {
	WindowData *wd = windows.getptr(p_id);
	if (wd) {
		wd->visible = true;
	}
}

void DisplayServerDSH::delete_sub_window(DisplayServerEnums::WindowID p_id) {
	if (p_id == DisplayServerEnums::MAIN_WINDOW_ID) {
		return;
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

void DisplayServerDSH::window_set_input_event_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (wd) {
		wd->input_event_callback = p_callable;
	}
}

Point2i DisplayServerDSH::window_get_position(DisplayServerEnums::WindowID p_window) const {
	const WindowData *wd = windows.getptr(p_window);
	return wd ? wd->position : Point2i();
}

void DisplayServerDSH::window_set_position(const Point2i &p_position, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (wd) {
		wd->position = p_position;
	}
}

void DisplayServerDSH::window_set_size(const Size2i p_size, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	ERR_FAIL_NULL(wd);
	wd->size = p_size;
	if (p_window == DisplayServerEnums::MAIN_WINDOW_ID && rendering_context) {
		rendering_context->window_set_size(p_window, (uint32_t)p_size.width, (uint32_t)p_size.height);
	}
	if (wd->rect_changed_callback.is_valid()) {
		wd->rect_changed_callback.call(Rect2i(wd->position, wd->size));
	}
}

Size2i DisplayServerDSH::window_get_size(DisplayServerEnums::WindowID p_window) const {
	const WindowData *wd = windows.getptr(p_window);
	return wd ? wd->size : Size2i();
}

void DisplayServerDSH::window_set_mode(DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::WindowID p_window) {
	WindowData *wd = windows.getptr(p_window);
	if (wd) {
		wd->mode = p_mode;
		wd->minimized = (p_mode == DisplayServerEnums::WINDOW_MODE_MINIMIZED);
	}
}

DisplayServerEnums::WindowMode DisplayServerDSH::window_get_mode(DisplayServerEnums::WindowID p_window) const {
	const WindowData *wd = windows.getptr(p_window);
	return wd ? wd->mode : DisplayServerEnums::WINDOW_MODE_WINDOWED;
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
	// (hidden suspends production entirely, per protocol 2 semantics).
	return worker != nullptr && worker->has_applied_surface() && worker->is_viewport_visible() && window_can_draw(DisplayServerEnums::MAIN_WINDOW_ID);
}

#if defined(DSH_VIEWPORT_ENABLED)
void DisplayServerDSH::window_set_ime_active(const bool p_active, DisplayServerEnums::WindowID p_window) {
	ime_active = p_active;
	dsh_notify_ime_changed();
}

void DisplayServerDSH::window_set_ime_position(const Point2i &p_pos, DisplayServerEnums::WindowID p_window) {
	ime_position = p_pos;
	dsh_notify_ime_changed();
}

void DisplayServerDSH::dsh_notify_ime_changed() {
	if (worker == nullptr) {
		return;
	}
	// Engine-physical window coordinates; the client maps them back to CSS px
	// against the applied surface before positioning its hidden IME host.
	Dictionary payload;
	payload["active"] = ime_active;
	payload["x"] = ime_position.x;
	payload["y"] = ime_position.y;
	worker->emit_viewport_extension("imeState", payload);
}
#else
void DisplayServerDSH::window_set_ime_active(const bool p_active, DisplayServerEnums::WindowID p_window) {
	ime_active = p_active;
}

void DisplayServerDSH::window_set_ime_position(const Point2i &p_pos, DisplayServerEnums::WindowID p_window) {
	ime_position = p_pos;
}

void DisplayServerDSH::dsh_notify_ime_changed() {}
#endif // DSH_VIEWPORT_ENABLED

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
