/**************************************************************************/
/*  dsh_viewport_worker.h                                                 */
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

#pragma once

#if defined(WINDOWS_ENABLED) && defined(DSH_VIEWPORT_ENABLED)

#include "core/math/rect2i.h"
#include "core/math/vector3.h"
#include "core/os/mutex.h"
#include "core/string/ustring.h"
#include "core/templates/local_vector.h"
#include "core/variant/dictionary.h"
#include "drivers/vulkan/dsh_viewport_frame_sink_vulkan.h"
#include "scene/main/node.h"
#include "servers/display/display_server_enums.h"

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <mutex>
#include <thread>

#include "dsh_shared_frames_win32.h"
#include "dsh_window_sink.h"

class DisplayServerDSH;
class SceneTree;

// Demo scene controller (spinning cube + orbit camera), driven by the
// generic protocol input events through the Input singleton. Drag state is
// re-derived from the button mask of every motion event, so a focus loss that
// releases pressed buttons can never leave a stale local "dragging" flag.
class DSHDemoController : public Node {
	GDCLASS(DSHDemoController, Node);

	NodePath cube_path;
	NodePath camera_path;
	double yaw = 0.0;
	double pitch = 0.0;
	double distance = 4.0;
	bool auto_rotate = true;
	bool right_drag_pan = true; // godot.viewport.v1 setRightDrag may switch this off.
	Vector3 target; // Pan offset (world units); the camera orbits this point.
	Vector2 last_pointer;
	char32_t last_unicode = 0; // Test hook: last pressed key's unicode (0 = none).

	void _update_camera();

protected:
	void _notification(int p_what);
	static void _bind_methods() {}

public:
	void setup(const NodePath &p_cube, const NodePath &p_camera);
	void reset_camera();
	void set_auto_rotate(bool p_enabled) { auto_rotate = p_enabled; }
	void set_right_drag_pan(bool p_enabled) { right_drag_pan = p_enabled; }
	Dictionary capture_state() const; // Test hook: observable controller state for the fake broker.
	void input(const Ref<InputEvent> &p_event) override;
};

// DSH native viewport worker: owns wire protocol 2 over stdin/stdout and the
// frame lease state machine, and acts as the DSHViewportFrameSinkVulkan that
// redirects swapchain presentation to D3D11 shared textures. All protocol
// prints go to stdout (raw writes; the engine's own stdout printing is
// disabled by the worker mode), diagnostics go to stderr.
//
// Protocol v2.1 (multi-window): the worker additionally owns a registry of
// per-sub-window sinks (window ids 1..15, main window stays 0 on this sink),
// emits window lifecycle/geometry events on the godot.window.v1 namespace and
// routes window-tagged input/resize/focus controls back to the right engine
// window. Window geometry is engine-driven: the client follows rect events.
class DSHViewportWorker : public DSHViewportFrameSinkVulkan {
public:
	static constexpr uint32_t MAX_VIEWPORT_DIMENSION = 2048;
	static constexpr uint32_t MAX_CSS_DIMENSION = 16384;
	static constexpr uint32_t MAX_WINDOWS = 16; // Protocol window ids 0..15.
	static constexpr uint32_t SUB_WINDOW_SLOTS = 2; // Per sub-window ring size (protocol v2.1).
	static constexpr uint64_t SLOT_LEASE_TIMEOUT_NS = 10ULL * 1000 * 1000 * 1000; // Mirrors the graphics101 lease watchdog.
	static const char *EXTENSION_NAMESPACE;
	static const char *VIEWPORT_NAMESPACE; // godot.viewport.v1: relative mouse mode and drag behavior.
	static const char *WINDOW_NAMESPACE; // godot.window.v1: window lifecycle/geometry (protocol v2.1).
	static const char *EDITOR_NAMESPACE; // godot.editor.v1: game spawn requests (protocol v2.1).

	static DSHViewportWorker *get_singleton() { return singleton; }
	static bool static_stop_check(); // Registered as DSHViewportMode::worker_stop_check.

	// Per sub-window state (main thread only). The sink renders the window's
	// swapchain through the same DSH path as the main surface; metadata feeds
	// the godot.window.v1 lifecycle events.
	struct SubWindowState {
		DSHWindowSink *sink = nullptr;
		Rect2i rect;
		String title;
		uint32_t flags = 0;
		bool visible = false;
		bool focused = false;
		int z = 0;
		DisplayServerEnums::WindowID transient_parent = DisplayServerEnums::INVALID_WINDOW_ID;
		bool has_pending_resize = false;
		uint32_t pending_resize_width = 0;
		uint32_t pending_resize_height = 0;
	};

private:
	static DSHViewportWorker *singleton;

	uint32_t broker_pid = 0;
	bool hello_sent = false;

	// ---- stdin thread -> main thread command queue ----
	std::thread stdin_thread;
	bool stdin_thread_started = false;
	std::mutex command_mutex;
	std::deque<String> commands;
	bool stdin_eof = false;

	// ---- protocol state (main thread) ----
	DisplayServerDSH *display_server = nullptr;
	uint64_t input_seq = 0;
	uint64_t epoch_counter = 0; // Bumped when a broker resize is applied; reported in surface/frame messages.
	std::atomic<uint64_t> frame_counter{ 0 }; // Emission counter reported as frameId.
	std::atomic<uint64_t> last_input_seq{ 0 };
	std::atomic<uint64_t> current_epoch{ 0 };
	std::atomic<uint32_t> surface_width{ 0 };
	std::atomic<uint32_t> surface_height{ 0 };
	std::atomic<bool> viewport_visible{ true };
	std::atomic<bool> stop_requested{ false };
	std::atomic<bool> fatal_error{ false };

	uint32_t pending_resize_width = 0;
	uint32_t pending_resize_height = 0;
	bool has_pending_resize = false; // Written by stdin thread, consumed in poll().

	// Coordinate mapping (interaction.map.v2): pointer/wheel events arrive in
	// viewport-local CSS px; the engine window works in physical px. The factors
	// are locked exactly when a resize is applied (same point as the epoch), so
	// geometry can never be half-old/half-new. Without cssWidth/cssHeight the
	// legacy single `scale` factor applies to both axes. Protocol v2.1 reuses
	// the session-level factors for every window (sub-window sizes and
	// positions are engine-driven, so only the session DPR is needed).
	double map_scale_x = 1.0;
	double map_scale_y = 1.0;
	double pending_scale = 1.0;
	double pending_css_width = 0.0;
	double pending_css_height = 0.0;
	bool has_pending_css = false;
	Vector2 last_engine_pointer; // Last mapped pointer position, for motion relatives.

	// ---- multi-window state (protocol v2.1, main thread) ----
	HashMap<uint32_t, SubWindowState> sub_windows;
	// Pump-thread snapshot of the active sub-window sinks; guarded by sinks_mutex
	// (the registry itself is main-thread only).
	std::mutex sinks_mutex;
	LocalVector<std::pair<uint32_t, DSHWindowSink *>> active_sinks;
	uint32_t focused_window = 0; // Engine window that currently holds keyboard focus (UINT32_MAX = none).
	ObjectID last_demo_popup; // Test hook: last popup opened through godot.demo.v1/openPopup.
	Callable game_lifecycle_listener; // EmbeddedProcessDSH, set while a game session runs (editor stage).
	bool debug_game_echo = false; // Verify hook: gameLifecycle controls echo back as gameDebug while no real game session runs (scripts/godot.mjs gameTab).

	uint64_t stats_last_usec = 0;

	// ---- frame sink state (mixed threads, main window) ----
	DSHSharedFramesWin32 frames_factory;
	DSHSharedFramesWin32::Pool frames_pool;
	bool frames_pool_ready = false;
	VkDevice vk_device = VK_NULL_HANDLE;
	VkSemaphore completion_semaphore = VK_NULL_HANDLE;
	mutable std::mutex sink_mutex;
	std::condition_variable sink_cv;
	std::deque<std::pair<uint64_t, uint32_t>> pending_presents; // (timeline value, slot) in allocation order.
	uint64_t next_present_value = 0;
	std::atomic<bool> slot_free[MAX_SLOTS];
	std::thread pump_thread;
	std::atomic<bool> pump_exit{ false };

	// ---- demo scene (main thread) ----
	DSHDemoController *demo_controller = nullptr;
	Vector<String> last_drop_paths; // Test hook: last OS file drop delivered to the engine.
	bool demo_ready = false;
	bool demo_rotate_value = true;

	void _stdin_loop();
	void _pump_loop();
	void _send_hello();
	void _ensure_demo_scene(SceneTree *p_tree);
	void _apply_queued_commands();
	void _handle_command(const String &p_line);
	void _handle_input_event(const Dictionary &p_event);
	void _handle_extension(const String &p_name, const Dictionary &p_payload);
	void _handle_viewport_extension(const String &p_name, const Dictionary &p_payload);
	void _handle_editor_extension(const String &p_name, const Dictionary &p_payload);
	void _debug_game_lifecycle(bool p_running, int64_t p_code); // Verify hook: echoes gameLifecycle back as a godot.editor.v1/gameDebug event.
	void _inject_key(const Dictionary &p_event, DisplayServerEnums::WindowID p_window);
	void _inject_drop(const Dictionary &p_event, DisplayServerEnums::WindowID p_window);
	void _inject_pointer(const Dictionary &p_event, DisplayServerEnums::WindowID p_window);
	void _inject_wheel(const Dictionary &p_event, DisplayServerEnums::WindowID p_window);
	void _inject_text(const Dictionary &p_event, DisplayServerEnums::WindowID p_window);
	void _inject_ime(const Dictionary &p_event, DisplayServerEnums::WindowID p_window);
	void _apply_pending_resize();
	void _apply_sub_window_resize(uint32_t p_window);
	void _emit_frame(uint32_t p_slot);
	void _emit_window_frame(uint32_t p_window, DSHWindowSink *p_sink, uint32_t p_slot);
	uint32_t _event_window(const Dictionary &p_event) const;
	Vector2 _map_css_to_engine(double p_x, double p_y) const;
	void _release_all_input_state();
	void _apply_focus(bool p_focused);
	void _emit_extension_event(const String &p_name, const Dictionary &p_payload);
	void _emit_extension_event_ns(const String &p_ns, const String &p_name, const Dictionary &p_payload);
	void _open_demo_popup(const Dictionary &p_payload);

	static String _modifiers_to_string(uint32_t p_modifiers);

public:
	// DSHViewportFrameSinkVulkan implementation (main window sink).
	virtual Error create_frames(VkPhysicalDevice p_physical_device, VkDevice p_device, uint32_t p_graphics_queue_family, uint32_t p_width, uint32_t p_height, uint32_t p_slot_count) override;
	virtual void destroy_frames() override;
	virtual bool has_frames() const override;
	virtual uint32_t get_slot_count() const override;
	virtual VkImage get_slot_image(uint32_t p_slot) const override;
	virtual uint64_t get_slot_nt_handle(uint32_t p_slot) const override;
	virtual VkSemaphore get_completion_semaphore() const override { return completion_semaphore; }
	virtual uint64_t allocate_present(uint32_t p_slot) override;
	virtual void cancel_present(uint64_t p_value) override;
	virtual uint32_t acquire_slot_blocking() override;
	virtual void mark_slot_released(uint32_t p_slot, uint64_t p_frame_id) override;
	virtual bool are_all_slots_free() const override;
	virtual void wait_all_presents_complete() override;

	Error initialize(uint32_t p_broker_pid); // Stores the broker PID; the D3D11 side initializes on the first create_frames() once the VkDevice exists.
	void emit_viewport_extension(const String &p_name, const Dictionary &p_payload); // Worker -> client event on the godot.viewport.v1 namespace (e.g. imeState).
	void emit_window_event(const String &p_name, const Dictionary &p_payload); // Worker -> client event on the godot.window.v1 namespace (protocol v2.1).
	void emit_editor_event(const String &p_name, const Dictionary &p_payload); // Worker -> client event on the godot.editor.v1 namespace (protocol v2.1).
	void set_display_server(DisplayServerDSH *p_display_server) { display_server = p_display_server; }
	void start(); // Spawns stdin/pump threads; called once the DisplayServer is fully created.
	void announce(); // Emits the protocol hello (first stdout line); requires the RenderingDevice.
	bool is_stop_requested() const { return stop_requested.load(); }
	bool has_fatal_error() const { return fatal_error.load(); }
	bool has_applied_surface() const { return current_epoch.load() >= 1; } // A broker resize has been applied.
	bool is_viewport_visible() const { return viewport_visible.load(); } // Broker visibility (hidden suspends production).

	// ---- multi-window API used by DisplayServerDSH (protocol v2.1) ----
	DSHWindowSink *dsh_create_window_sink(uint32_t p_window); // Registers a sub-window sink (ids 1..MAX_WINDOWS-1).
	void dsh_destroy_window_sink(uint32_t p_window);
	bool dsh_has_window(uint32_t p_window) const { return p_window == 0 || sub_windows.has(p_window); }
	SubWindowState *dsh_get_sub_window(uint32_t p_window) { return sub_windows.getptr(p_window); }
	const SubWindowState *dsh_get_sub_window(uint32_t p_window) const { return sub_windows.getptr(p_window); }
	void dsh_set_focused_window(uint32_t p_window); // Client/engine focus routing; fires FOCUS_IN/OUT + alignment event.
	uint32_t dsh_get_focused_window() const { return focused_window; }
	void dsh_notify_engine_focus(uint32_t p_window, bool p_focused); // Engine-internal focus change -> client alignment event.

	// ---- game spawn channel (protocol v2.1, editor stage) ----
	void set_game_lifecycle_listener(const Callable &p_listener) {
		game_lifecycle_listener = p_listener;
		if (p_listener.is_valid()) {
			debug_game_echo = false; // A real game session owns the lifecycle slot.
		}
	}
	void editor_spawn_game(const Vector<String> &p_args, const Rect2i &p_rect); // -> godot.editor.v1/spawnGame.
	void editor_set_game_rect(const Rect2i &p_rect); // -> godot.editor.v1/gameRect (GameView size follow).
	void editor_close_game(); // -> godot.editor.v1/gameClose (user pressed Stop).

	// Called by DisplayServerDSH::process_events() on the main thread.
	void poll();

	DSHViewportWorker();
	~DSHViewportWorker() override;
};

#endif // WINDOWS_ENABLED && DSH_VIEWPORT_ENABLED
