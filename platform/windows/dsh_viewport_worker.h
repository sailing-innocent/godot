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

#include "core/os/mutex.h"
#include "core/string/ustring.h"
#include "core/templates/local_vector.h"
#include "drivers/vulkan/dsh_viewport_frame_sink_vulkan.h"
#include "scene/main/node.h"

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <mutex>
#include <thread>

#include "dsh_shared_frames_win32.h"

class DisplayServerDSH;
class SceneTree;

// Demo scene controller (spinning cube + orbit camera), driven by the
// generic protocol input events through the Input singleton.
class DSHDemoController : public Node {
	GDCLASS(DSHDemoController, Node);

	NodePath cube_path;
	NodePath camera_path;
	double yaw = 0.0;
	double pitch = 0.0;
	double distance = 4.0;
	bool dragging = false;
	bool auto_rotate = true;
	Vector2 last_pointer;

	void _update_camera();

protected:
	void _notification(int p_what);
	static void _bind_methods() {}

public:
	void setup(const NodePath &p_cube, const NodePath &p_camera);
	void reset_camera();
	void set_auto_rotate(bool p_enabled) { auto_rotate = p_enabled; }
	void release_pointer() { dragging = false; }
	void input(const Ref<InputEvent> &p_event) override;
};

// DSH native viewport worker: owns wire protocol 2 over stdin/stdout and the
// frame lease state machine, and acts as the DSHViewportFrameSinkVulkan that
// redirects swapchain presentation to D3D11 shared textures. All protocol
// prints go to stdout (raw writes; the engine's own stdout printing is
// disabled by the worker mode), diagnostics go to stderr.
class DSHViewportWorker : public DSHViewportFrameSinkVulkan {
public:
	static constexpr uint32_t MAX_VIEWPORT_DIMENSION = 2048;
	static constexpr uint64_t SLOT_LEASE_TIMEOUT_NS = 10ULL * 1000 * 1000 * 1000; // Mirrors the graphics101 lease watchdog.
	static const char *EXTENSION_NAMESPACE;

	static DSHViewportWorker *get_singleton() { return singleton; }
	static bool static_stop_check(); // Registered as DSHViewportMode::worker_stop_check.

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

	uint64_t stats_last_usec = 0;

	// ---- frame sink state (mixed threads) ----
	DSHSharedFramesWin32 frames;
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
	void _inject_key(const Dictionary &p_event);
	void _inject_pointer(const Dictionary &p_event);
	void _inject_wheel(const Dictionary &p_event);
	void _apply_pending_resize();
	void _emit_frame(uint32_t p_slot);

	static String _modifiers_to_string(uint32_t p_modifiers);

public:
	// DSHViewportFrameSinkVulkan implementation.
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
	void set_display_server(DisplayServerDSH *p_display_server) { display_server = p_display_server; }
	void start(); // Spawns stdin/pump threads; called once the DisplayServer is fully created.
	void announce(); // Emits the protocol hello (first stdout line); requires the RenderingDevice.
	bool is_stop_requested() const { return stop_requested.load(); }
	bool has_fatal_error() const { return fatal_error.load(); }
	bool has_applied_surface() const { return current_epoch.load() >= 1; } // A broker resize has been applied.
	bool is_viewport_visible() const { return viewport_visible.load(); } // Broker visibility (hidden suspends production).

	// Called by DisplayServerDSH::process_events() on the main thread.
	void poll();

	DSHViewportWorker();
	~DSHViewportWorker() override;
};

#endif // WINDOWS_ENABLED && DSH_VIEWPORT_ENABLED
