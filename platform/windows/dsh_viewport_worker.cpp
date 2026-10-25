/**************************************************************************/
/*  dsh_viewport_worker.cpp                                               */
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

#if defined(WINDOWS_ENABLED) && defined(DSH_VIEWPORT_ENABLED)

#include "dsh_viewport_worker.h"

#include "core/config/engine.h"
#include "core/input/input.h"
#include "core/io/json.h"
#include "core/os/main_loop.h"
#include "core/os/os.h"
#include "display_server_dsh.h"
#include "drivers/vulkan/rendering_device_driver_vulkan.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/gui/color_rect.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#include "scene/resources/3d/primitive_meshes.h"
#include "scene/resources/material.h"
#include "servers/rendering/rendering_device.h"
#include "main/dsh_viewport_mode.h"

#include <chrono>
#include <cstdio>

// Reuse the maintained DOM code -> Godot key mapping table (normally
// compiled only for the web platform).
#define EM_UTF8 char
#include "platform/web/dom_keys.inc"

const char *DSHViewportWorker::EXTENSION_NAMESPACE = "godot.demo.v1";
const char *DSHViewportWorker::VIEWPORT_NAMESPACE = "godot.viewport.v1";
const char *DSHViewportWorker::WINDOW_NAMESPACE = "godot.window.v1";
const char *DSHViewportWorker::EDITOR_NAMESPACE = "godot.editor.v1";
DSHViewportWorker *DSHViewportWorker::singleton = nullptr;

namespace {
Mutex stdout_mutex;

void protocol_print(const String &p_line) {
	CharString utf8 = p_line.utf8();
	MutexLock lock(stdout_mutex);
	fwrite(utf8.ptr(), 1, utf8.length(), stdout);
	fputc('\n', stdout);
	fflush(stdout);
}

void diagnostic_print(const String &p_line) {
	CharString utf8 = p_line.utf8();
	MutexLock lock(stdout_mutex);
	fwrite(utf8.ptr(), 1, utf8.length(), stderr);
	fputc('\n', stderr);
	fflush(stderr);
}

uint64_t steady_usec() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
} // namespace

/*****************************/
/**** DSHDemoController ****/
/*****************************/

void DSHDemoController::setup(const NodePath &p_cube, const NodePath &p_camera) {
	cube_path = p_cube;
	camera_path = p_camera;
	set_process(true);
	set_process_input(true);
}

void DSHDemoController::reset_camera() {
	yaw = 0.35;
	pitch = 0.25;
	distance = 4.0;
	target = Vector3();
}

void DSHDemoController::_update_camera() {
	Camera3D *camera = Object::cast_to<Camera3D>(get_node_or_null(camera_path));
	if (!camera) {
		return;
	}
	const double clamped_pitch = CLAMP(pitch, -1.45, 1.45);
	Vector3 eye(Math::sin(yaw) * Math::cos(clamped_pitch), Math::sin(clamped_pitch), Math::cos(yaw) * Math::cos(clamped_pitch));
	camera->set_position(target + eye * distance);
	camera->look_at(target, Vector3(0, 1, 0));
}

void DSHDemoController::input(const Ref<InputEvent> &p_event) {
	Ref<InputEventKey> key_event = p_event;
	if (key_event.is_valid() && key_event->is_pressed()) {
		last_unicode = key_event->get_unicode();
	}
	Ref<InputEventMouseButton> button = p_event;
	if (button.is_valid()) {
		if (button->get_button_index() == MouseButton::WHEEL_UP && button->is_pressed()) {
			distance = MAX(1.2, distance - 0.35 * button->get_factor());
			_update_camera();
		} else if (button->get_button_index() == MouseButton::WHEEL_DOWN && button->is_pressed()) {
			distance = MIN(12.0, distance + 0.35 * button->get_factor());
			_update_camera();
		} else if (button->get_button_index() >= MouseButton::LEFT && button->get_button_index() <= MouseButton::RIGHT && button->is_pressed()) {
			// Remember the press anchor; the drag itself is derived per motion
			// event from the live button mask (see below).
			last_pointer = button->get_position();
		}
		return;
	}
	Ref<InputEventMouseMotion> motion = p_event;
	if (motion.is_valid()) {
		const MouseButtonMask mask = motion->get_button_mask();
		// MouseButtonMask is a scoped enum without bitwise operators: test bits
		// through the integer value (the protocol buttons field uses the same
		// DOM bit layout).
		const int mask_bits = (int)mask;
		const bool orbit_drag = (mask_bits & (int)MouseButtonMask::LEFT) != 0 && !motion->is_shift_pressed();
		const bool pan_drag = (mask_bits & (int)MouseButtonMask::MIDDLE) != 0
				|| (mask_bits & (int)MouseButtonMask::LEFT) != 0 && motion->is_shift_pressed()
				|| (mask_bits & (int)MouseButtonMask::RIGHT) != 0 && right_drag_pan;
		if (mask_bits == 0 || (!orbit_drag && !pan_drag)) {
			last_pointer = motion->get_position();
			return;
		}
		Vector2 relative = motion->get_relative();
		if (relative == Vector2()) {
			relative = motion->get_position() - last_pointer;
		}
		last_pointer = motion->get_position();
		if (orbit_drag) {
			yaw -= relative.x * 0.006;
			pitch = CLAMP(pitch + relative.y * 0.006, -1.45, 1.45);
		} else {
			// Pan in the camera plane; scale by distance so dolly speed feels uniform.
			Camera3D *camera = Object::cast_to<Camera3D>(get_node_or_null(camera_path));
			if (camera) {
				const Basis basis = camera->get_global_transform().basis;
				target += (-basis.get_column(0) * relative.x + basis.get_column(1) * relative.y) * (distance * 0.0016);
			}
		}
		_update_camera();
	}
}

Dictionary DSHDemoController::capture_state() const {
	Dictionary state;
	state["yaw"] = yaw;
	state["pitch"] = pitch;
	state["distance"] = distance;
	state["autoRotate"] = auto_rotate;
	state["rightDragPan"] = right_drag_pan;
	// The Input singleton mask is authoritative: release_pressed_events()
	// (focus loss, visibility, pointer cancel) clears it, so a stale local
	// flag can never survive an unfocus.
	Input *input = Input::get_singleton();
	const int mask_bits = input ? (int)input->get_mouse_button_mask() : 0;
	state["dragging"] = mask_bits != 0;
	state["buttons"] = (int64_t)mask_bits;
	state["lastUnicode"] = (int64_t)last_unicode;
	Array pointer;
	pointer.push_back(last_pointer.x);
	pointer.push_back(last_pointer.y);
	state["lastPointer"] = pointer;
	Array target_array;
	target_array.push_back(target.x);
	target_array.push_back(target.y);
	target_array.push_back(target.z);
	state["target"] = target_array;
	return state;
}

void DSHDemoController::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		reset_camera();
		_update_camera();
	}
	if (p_what != NOTIFICATION_PROCESS) {
		return;
	}
	const double delta = get_process_delta_time();

	Node3D *cube = Object::cast_to<Node3D>(get_node_or_null(cube_path));
	if (cube && auto_rotate) {
		cube->rotate_y(delta * 0.9);
		cube->rotate_x(delta * 0.13);
	}

	Input *input = Input::get_singleton();
	if (!input) {
		return;
	}
	const double orbit_speed = 1.4 * delta;
	if (input->is_physical_key_pressed(Key::A)) {
		yaw += orbit_speed;
		_update_camera();
	}
	if (input->is_physical_key_pressed(Key::D)) {
		yaw -= orbit_speed;
		_update_camera();
	}
	if (input->is_physical_key_pressed(Key::W)) {
		pitch = CLAMP(pitch + orbit_speed, -1.45, 1.45);
		_update_camera();
	}
	if (input->is_physical_key_pressed(Key::S)) {
		pitch = CLAMP(pitch - orbit_speed, -1.45, 1.45);
		_update_camera();
	}
	if (input->is_physical_key_pressed(Key::Q)) {
		distance = MIN(12.0, distance + 2.0 * delta);
		_update_camera();
	}
	if (input->is_physical_key_pressed(Key::E)) {
		distance = MAX(1.2, distance - 2.0 * delta);
		_update_camera();
	}
	static thread_local bool reset_was_pressed = false;
	const bool reset_pressed = input->is_physical_key_pressed(Key::R);
	if (reset_pressed && !reset_was_pressed) {
		reset_camera();
		_update_camera();
	}
	reset_was_pressed = reset_pressed;
	static thread_local bool rotate_was_pressed = false;
	const bool rotate_pressed = input->is_physical_key_pressed(Key::SPACE);
	if (rotate_pressed && !rotate_was_pressed) {
		auto_rotate = !auto_rotate;
	}
	rotate_was_pressed = rotate_pressed;
}

/******************************/
/**** DSHViewportWorker ****/
/******************************/

DSHViewportWorker::DSHViewportWorker() {
	singleton = this;
	// Main::iteration polls this hook; main/ never includes platform headers.
	DSHViewportMode::worker_stop_check = &DSHViewportWorker::static_stop_check;
	for (uint32_t i = 0; i < MAX_SLOTS; i++) {
		slot_free[i].store(true);
	}
}

bool DSHViewportWorker::static_stop_check() {
	return singleton != nullptr && singleton->is_stop_requested();
}

DSHViewportWorker::~DSHViewportWorker() {
	if (DSHViewportMode::worker_stop_check == &DSHViewportWorker::static_stop_check) {
		DSHViewportMode::worker_stop_check = nullptr;
	}
	stop_requested.store(true);
	pump_exit.store(true);
	{
		std::lock_guard<std::mutex> lock(sink_mutex);
		sink_cv.notify_all();
	}
	if (pump_thread.joinable()) {
		pump_thread.join();
	}
	if (stdin_thread_started) {
		// Unblock the pending stdin read so the thread can exit.
		CancelSynchronousIo(stdin_thread.native_handle());
		if (stdin_thread.joinable()) {
			stdin_thread.join();
		}
	}
	protocol_print(String("{\"type\":\"stopped\",\"frames\":") + uitos(frame_counter.load()) + ",\"validationErrors\":0}");
	if (completion_semaphore != VK_NULL_HANDLE && vk_device != VK_NULL_HANDLE) {
		vkDestroySemaphore(vk_device, completion_semaphore, nullptr);
		completion_semaphore = VK_NULL_HANDLE;
	}
	{
		// Sub-window sinks must release their pools before the factory closes.
		std::lock_guard<std::mutex> lock(sinks_mutex);
		for (KeyValue<uint32_t, SubWindowState> &kv : sub_windows) {
			if (kv.value.sink) {
				kv.value.sink->shutdown();
				memdelete(kv.value.sink);
				kv.value.sink = nullptr;
			}
		}
		sub_windows.clear();
		active_sinks.clear();
	}
	frames_factory.destroy_frames(frames_pool);
	frames_factory.finalize();
	singleton = nullptr;
}

Error DSHViewportWorker::initialize(uint32_t p_broker_pid) {
	broker_pid = p_broker_pid;
	ERR_FAIL_COND_V_MSG(broker_pid == 0, ERR_INVALID_PARAMETER, "DSH viewport worker requires a valid --broker-pid.");
	return OK;
}

void DSHViewportWorker::start() {
	if (!stdin_thread_started) {
		stdin_thread = std::thread(&DSHViewportWorker::_stdin_loop, this);
		stdin_thread_started = true;
	}
	if (!pump_thread.joinable()) {
		pump_thread = std::thread(&DSHViewportWorker::_pump_loop, this);
	}
}

void DSHViewportWorker::_stdin_loop() {
	char buffer[20000];
	while (!stop_requested.load()) {
		if (!fgets(buffer, sizeof(buffer), stdin)) {
			break; // EOF or error: the broker closed the pipe (stop sequence).
		}
		String line = String::utf8(buffer).strip_edges();
		if (line.is_empty()) {
			continue;
		}
		if (line.length() > 16384) {
			diagnostic_print("DSH viewport: dropping an oversized control line.");
			continue;
		}
		// Releases unblock the render thread, which may be waiting inside
		// swapchain acquisition; they must not wait for the next main-loop
		// poll. The slot state is mutex-protected, so the stdin thread can
		// apply them directly (the broker only releases frames it received).
		const Variant parsed = JSON::parse_string(line);
		if (parsed.get_type() == Variant::DICTIONARY) {
			const Dictionary message = parsed;
			if (String(message.get("kind", "")) == "release") {
				const uint64_t release_epoch = message.get("epoch", 0);
				const uint32_t slot = message.get("slot", UINT32_MAX);
				const uint64_t frame = message.get("frameId", 0);
				const uint32_t window = message.get("window", 0);
				if (window == 0) {
					if (release_epoch == current_epoch.load()) {
						mark_slot_released(slot, frame);
					}
				} else {
					std::lock_guard<std::mutex> lock(sinks_mutex);
					const SubWindowState *sw = sub_windows.getptr(window);
					if (sw && sw->sink && release_epoch == sw->sink->get_epoch()) {
						sw->sink->mark_slot_released(slot, frame);
					}
				}
				continue;
			}
		}
		std::lock_guard<std::mutex> lock(command_mutex);
		commands.push_back(line);
	}
	std::lock_guard<std::mutex> lock(command_mutex);
	stdin_eof = true;
}

void DSHViewportWorker::_pump_loop() {
	uint64_t last_waited_value = 0;
	while (!pump_exit.load()) {
		bool progressed = false;

		// ---- sub-windows: surface receipts lead their frames ----
		LocalVector<std::pair<uint32_t, DSHWindowSink *>> round;
		{
			std::lock_guard<std::mutex> lock(sinks_mutex);
			for (uint32_t i = 0; i < active_sinks.size(); i++) {
				round.push_back(active_sinks[i]);
			}
		}
		for (uint32_t i = 0; i < round.size(); i++) {
			DSHWindowSink *sink = round[i].second;
			const uint32_t window = round[i].first;
			if (sink->get_epoch() != sink->get_published_epoch()) {
				// Announce the new ring before any frame of this epoch can be
				// emitted (the broker rejects frames for unannounced windows).
				protocol_print(String("{\"type\":\"surface\",\"epoch\":") + uitos(sink->get_epoch())
						+ ",\"width\":" + uitos(sink->get_width()) + ",\"height\":" + uitos(sink->get_height())
						+ ",\"slots\":" + uitos(sink->get_slot_count()) + ",\"window\":" + uitos(window) + "}");
				sink->mark_epoch_published(sink->get_epoch());
				progressed = true;
			}
			uint32_t slot;
			while (sink->pop_completed(slot)) {
				// Transient 0-sized rings (engine window setup) must not produce
				// broker-visible frames: release the slot instead of offering a
				// zero-handle frame the broker would reject.
				if (slot >= sink->get_slot_count() || sink->get_width() == 0 || sink->get_height() == 0
						|| sink->get_slot_nt_handle(slot) == 0) {
					sink->mark_slot_released(slot, 0);
				} else {
					_emit_window_frame(window, sink, slot);
				}
				progressed = true;
			}
		}

		// ---- main window ----
		{
			std::unique_lock<std::mutex> lock(sink_mutex);
			if (!pending_presents.empty()) {
				const std::pair<uint64_t, uint32_t> present = pending_presents.front();
				lock.unlock();
				if (present.first <= last_waited_value) {
					// Defensive: values are strictly increasing; never wait backwards.
					std::lock_guard<std::mutex> relock(sink_mutex);
					if (!pending_presents.empty() && pending_presents.front().first == present.first) {
						pending_presents.pop_front();
					}
					sink_cv.notify_all();
					progressed = true;
				} else if (current_epoch.load() >= 1 && DSHSharedFramesWin32::has_frames(frames_pool)) {
					VkSemaphoreWaitInfo wait_info{};
					wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
					wait_info.semaphoreCount = 1;
					wait_info.pSemaphores = &completion_semaphore;
					wait_info.pValues = &present.first;
					VkResult err = vkWaitSemaphores(vk_device, &wait_info, 0);
					if (err == VK_SUCCESS) {
						last_waited_value = present.first;
						{
							std::lock_guard<std::mutex> relock(sink_mutex);
							if (!pending_presents.empty() && pending_presents.front().first == present.first) {
								pending_presents.pop_front();
							}
						}
						sink_cv.notify_all();
						// Same degenerate-ring guard as the sub-window path: the
						// engine can briefly resize a window to 0x0 during setup.
						if (present.second >= DSHSharedFramesWin32::get_slot_count(frames_pool)
								|| DSHSharedFramesWin32::get_width(frames_pool) == 0
								|| DSHSharedFramesWin32::get_height(frames_pool) == 0
								|| DSHSharedFramesWin32::get_slot_nt_handle(frames_pool, present.second) == 0) {
							mark_slot_released(present.second, 0);
						} else {
							_emit_frame(present.second);
						}
						progressed = true;
					} else if (err != VK_TIMEOUT) {
						diagnostic_print(String("DSH viewport: frame completion wait failed (VkResult ") + itos(err) + ").");
						fatal_error.store(true);
						return;
					}
				}
			}
		}

		if (!progressed) {
			std::unique_lock<std::mutex> lock(sink_mutex);
			sink_cv.wait_for(lock, std::chrono::milliseconds(2));
		}
	}
}

void DSHViewportWorker::_emit_frame(uint32_t p_slot) {
	if (stop_requested.load() || current_epoch.load() < 1 || !DSHSharedFramesWin32::has_frames(frames_pool) || p_slot >= DSHSharedFramesWin32::get_slot_count(frames_pool)) {
		return;
	}
	const uint64_t frame_id = ++frame_counter;
	const uint64_t timestamp = steady_usec();
	const uint64_t handle = DSHSharedFramesWin32::get_slot_nt_handle(frames_pool, p_slot);
	char line[512];
	snprintf(line, sizeof(line),
			"{\"type\":\"frame\",\"epoch\":%llu,\"slot\":%u,\"frameId\":%llu,"
			"\"width\":%u,\"height\":%u,\"inputSeq\":%llu,\"timestampUs\":%llu,\"handle\":\"%llu\"}",
			(unsigned long long)current_epoch.load(), p_slot, (unsigned long long)frame_id,
			(unsigned)DSHSharedFramesWin32::get_width(frames_pool), (unsigned)DSHSharedFramesWin32::get_height(frames_pool),
			(unsigned long long)last_input_seq.load(), (unsigned long long)timestamp, (unsigned long long)handle);
	protocol_print(line);
}

void DSHViewportWorker::_emit_window_frame(uint32_t p_window, DSHWindowSink *p_sink, uint32_t p_slot) {
	if (stop_requested.load() || p_sink == nullptr || !p_sink->has_frames() || p_slot >= p_sink->get_slot_count()) {
		return;
	}
	const uint64_t frame_id = ++frame_counter;
	const uint64_t timestamp = steady_usec();
	const uint64_t handle = p_sink->get_slot_nt_handle(p_slot);
	char line[560];
	snprintf(line, sizeof(line),
			"{\"type\":\"frame\",\"epoch\":%llu,\"slot\":%u,\"frameId\":%llu,"
			"\"width\":%u,\"height\":%u,\"inputSeq\":%llu,\"timestampUs\":%llu,\"handle\":\"%llu\",\"window\":%u}",
			(unsigned long long)p_sink->get_epoch(), p_slot, (unsigned long long)frame_id,
			(unsigned)p_sink->get_width(), (unsigned)p_sink->get_height(),
			(unsigned long long)last_input_seq.load(), (unsigned long long)timestamp, (unsigned long long)handle,
			(unsigned)p_window);
	protocol_print(line);
}

void DSHViewportWorker::_send_hello() {
	if (hello_sent) {
		return;
	}
	hello_sent = true;
	uint32_t vendor_id = 0;
	uint32_t device_id = 0;
	String gpu_name = "Unknown";
	RenderingDevice *rd = RenderingDevice::get_singleton();
	if (rd && rd->get_device_driver()) {
		RenderingDeviceDriverVulkan *driver = (RenderingDeviceDriverVulkan *)rd->get_device_driver();
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(driver->get_vulkan_physical_device(), &properties);
		vendor_id = properties.vendorID;
		device_id = properties.deviceID;
		gpu_name = String(properties.deviceName);
	}
	protocol_print(String("{\"type\":\"hello\",\"protocol\":2,\"transport\":\"d3d11-nt-rgba8\",\"extensions\":[\"")
			+ EXTENSION_NAMESPACE + "\",\"" + VIEWPORT_NAMESPACE + "\",\"" + WINDOW_NAMESPACE + "\",\"" + EDITOR_NAMESPACE
			+ "\"],\"features\":[\"interaction.map.v2\",\"input.text\",\"input.ime\",\"pointer.lock\",\"input.drop\",\"windows.multi.v1\"]"
			+ ",\"backend\":\"godot-vulkan\",\"vendorId\":" + uitos(vendor_id)
			+ ",\"deviceId\":" + uitos(device_id) + ",\"gpu\":" + JSON::stringify(gpu_name)
			+ ",\"runtimeCpuReadbacks\":0,\"exportGpuCopies\":0}");
}

void DSHViewportWorker::announce() {
	_send_hello();
}

void DSHViewportWorker::poll() {
	if (!display_server) {
		return;
	}
	SceneTree *tree = Object::cast_to<SceneTree>(OS::get_singleton()->get_main_loop());
	if (tree && !demo_ready) {
		_ensure_demo_scene(tree);
	}

	_apply_queued_commands();
	if (stop_requested.load()) {
		return;
	}
	_apply_pending_resize();

	// Sub-window resizes requested through the protocol (protocol v2.1): the
	// engine remains the geometry source of truth, so these are applied through
	// the regular display-server path (which drives the surface recreation).
	for (KeyValue<uint32_t, SubWindowState> &kv : sub_windows) {
		_apply_sub_window_resize(kv.key);
	}

	const uint64_t now = OS::get_singleton()->get_ticks_usec();
	if (stats_last_usec == 0) {
		stats_last_usec = now;
	} else if (now - stats_last_usec >= 1000000) {
		stats_last_usec = now;
		protocol_print(String("{\"type\":\"stats\",\"frames\":") + uitos(frame_counter.load())
				+ ",\"inputSeq\":" + uitos(input_seq) + ",\"validationErrors\":0}");
	}

	const bool active = viewport_visible.load() && DSHSharedFramesWin32::has_frames(frames_pool) && !has_pending_resize;
	const int target_fps = active ? 60 : 10;
	if (Engine::get_singleton()->get_max_fps() != target_fps) {
		fprintf(stderr, "[dsh] fps target %d->%d visible=%d has_frames=%d pending_resize=%d\n", Engine::get_singleton()->get_max_fps(), target_fps,
				(int)viewport_visible.load(), (int)DSHSharedFramesWin32::has_frames(frames_pool), (int)has_pending_resize);
		fflush(stderr);
		Engine::get_singleton()->set_max_fps(target_fps);
	}
}

void DSHViewportWorker::_ensure_demo_scene(SceneTree *p_tree) {
	demo_ready = true; // Only one attempt, even if a user scene is present.
	Window *root = p_tree->get_root();
	if (!root || root->get_child_count() > 0) {
		return; // A project scene was loaded; the demo stays out of the way.
	}
	Node3D *scene = memnew(Node3D);
	scene->set_name("DSHDemo");

	MeshInstance3D *cube = memnew(MeshInstance3D);
	cube->set_name("Cube");
	Ref<BoxMesh> box_mesh;
	box_mesh.instantiate();
	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_albedo(Color(0.85, 0.55, 0.2));
	material->set_roughness(0.45);
	box_mesh->surface_set_material(0, material);
	cube->set_mesh(box_mesh);
	scene->add_child(cube);

	DirectionalLight3D *light = memnew(DirectionalLight3D);
	light->set_rotation_degrees(Vector3(-50, -30, 0));
	scene->add_child(light);

	Camera3D *camera = memnew(Camera3D);
	camera->set_name("Camera");
	scene->add_child(camera);

	DSHDemoController *controller = memnew(DSHDemoController);
	controller->set_name("Controller");
	scene->add_child(controller);

	root->add_child(scene);

	camera->set_current(true);
	controller->setup(cube->get_path(), camera->get_path());
	controller->set_auto_rotate(demo_rotate_value);
	demo_controller = controller;
}

void DSHViewportWorker::_open_demo_popup(const Dictionary &p_payload) {
	MainLoop *main_loop = OS::get_singleton()->get_main_loop();
	SceneTree *tree = Object::cast_to<SceneTree>(main_loop);
	if (!tree || !tree->get_root()) {
		diagnostic_print("DSH viewport: openPopup ignored (no scene tree).");
		return;
	}
	Window *popup = memnew(Window);
	popup->set_title("DSH Demo Popup");
	const int width = CLAMP((int)p_payload.get("width", 240), 16, (int)MAX_VIEWPORT_DIMENSION);
	const int height = CLAMP((int)p_payload.get("height", 160), 16, (int)MAX_VIEWPORT_DIMENSION);
	popup->set_size(Size2i(width, height));
	ColorRect *rect = memnew(ColorRect);
	const String color_html = p_payload.get("color", "#336699");
	const Color color = Color::html(color_html.is_empty() ? "#336699" : color_html);
	rect->set_color(color);
	rect->set_anchors_preset(Control::PRESET_FULL_RECT);
	popup->add_child(rect);
	tree->get_root()->add_child(popup);
	popup->popup_centered();
	last_demo_popup = popup->get_instance_id();
}

void DSHViewportWorker::_apply_queued_commands() {
	LocalVector<String> batch;
	bool eof = false;
	{
		std::lock_guard<std::mutex> lock(command_mutex);
		batch.resize(commands.size());
		for (uint32_t i = 0; i < commands.size(); i++) {
			batch[i] = commands[i];
		}
		commands.clear();
		eof = stdin_eof;
	}
	for (uint32_t i = 0; i < batch.size(); i++) {
		_handle_command(batch[i]);
		if (stop_requested.load()) {
			break;
		}
	}
	if (eof && !stop_requested.load()) {
		// stdin EOF is part of the broker stop sequence.
		stop_requested.store(true);
	}
}

void DSHViewportWorker::_handle_command(const String &p_line) {
	Variant parsed = JSON::parse_string(p_line);
	if (parsed.get_type() != Variant::DICTIONARY) {
		diagnostic_print("DSH viewport: ignoring a malformed control line.");
		return;
	}
	Dictionary message = parsed;
	const String kind = message.get("kind", "");

	if (kind == "stop") {
		stop_requested.store(true);
		return;
	}
	if (kind == "resize") {
		const uint32_t width = message.get("width", 0);
		const uint32_t height = message.get("height", 0);
		const uint32_t window = message.get("window", 0);
		if (width > MAX_VIEWPORT_DIMENSION || height > MAX_VIEWPORT_DIMENSION) {
			diagnostic_print("DSH viewport: resize beyond the 2048x2048 quota ignored.");
			return;
		}
		if (window != 0 && !sub_windows.has(window)) {
			diagnostic_print("DSH viewport: resize for an unknown window ignored.");
			return;
		}
		double css_width = 0.0;
		double css_height = 0.0;
		bool has_css = false;
		if (message.has("cssWidth") || message.has("cssHeight")) {
			// interaction.map.v2: per-axis CSS geometry for exact inverse mapping.
			// Both fields are required together; partial or non-finite values fall
			// back to the legacy single-scale behavior.
			const double cw = message.get("cssWidth", 0.0);
			const double ch = message.get("cssHeight", 0.0);
			if (cw > 0.0 && ch > 0.0 && cw <= MAX_CSS_DIMENSION && ch <= MAX_CSS_DIMENSION
					&& Math::is_finite(cw) && Math::is_finite(ch)) {
				css_width = cw;
				css_height = ch;
				has_css = true;
			} else {
				diagnostic_print("DSH viewport: resize with invalid cssWidth/cssHeight ignored (both finite, 0<x<=16384 required).");
				return;
			}
		}
		if (window != 0) {
			// Protocol v2.1: sub-window resizes are applied per window; the
			// session-level map factors still come from the main-window resize.
			SubWindowState *sw = sub_windows.getptr(window);
			sw->pending_resize_width = width;
			sw->pending_resize_height = height;
			sw->has_pending_resize = true;
			return;
		}
		pending_resize_width = width;
		pending_resize_height = height;
		pending_scale = MAX(1e-6, (double)message.get("scale", 1.0));
		pending_css_width = css_width;
		pending_css_height = css_height;
		has_pending_css = has_css;
		has_pending_resize = true;
		return;
	}
	if (kind == "visible") {
		const bool visible = message.get("value", true);
		viewport_visible.store(visible);
		if (!visible) {
			// Clears every pressed key/button; the demo controller derives its
			// drag state from the live Input mask, so no separate reset is needed.
			_apply_focus(false);
		}
		return;
	}
	if (kind == "release") {
		const uint64_t release_epoch = message.get("epoch", 0);
		const uint32_t slot = message.get("slot", UINT32_MAX);
		const uint64_t frame = message.get("frameId", 0);
		const uint32_t window = message.get("window", 0);
		if (window == 0) {
			if (release_epoch == current_epoch.load()) {
				mark_slot_released(slot, frame);
			}
		} else {
			const SubWindowState *sw = sub_windows.getptr(window);
			if (sw && sw->sink && release_epoch == sw->sink->get_epoch()) {
				sw->sink->mark_slot_released(slot, frame);
			}
		}
		return;
	}
	if (kind == "input" || kind == "extension") {
		const uint64_t seq = message.get("seq", 0);
		if (seq <= input_seq) {
			return; // Stale or duplicate; the broker drops these too.
		}
		input_seq = seq;
		last_input_seq.store(seq);
			if (kind == "extension") {
				const String ns = message.get("namespace", "");
				const String name = message.get("name", "");
				Dictionary payload = message.get("payload", Dictionary());
				if (ns == EXTENSION_NAMESPACE) {
					_handle_extension(name, payload);
				} else if (ns == VIEWPORT_NAMESPACE) {
					_handle_viewport_extension(name, payload);
				} else if (ns == EDITOR_NAMESPACE) {
					_handle_editor_extension(name, payload);
				} else {
					diagnostic_print("DSH viewport: unknown extension namespace '" + ns + "'.");
				}
			} else {
			Dictionary event = message.get("event", Dictionary());
			_handle_input_event(event);
		}
		return;
	}
	diagnostic_print("DSH viewport: ignoring unknown control kind '" + kind + "'.");
}

void DSHViewportWorker::_handle_extension(const String &p_name, const Dictionary &p_payload) {
	if (!demo_controller) {
		if (p_name == "rotate") {
			demo_rotate_value = p_payload.get("value", true);
		}
		return;
	}
	if (p_name == "reset") {
		demo_controller->reset_camera();
	} else if (p_name == "rotate") {
		demo_controller->set_auto_rotate(p_payload.get("value", true));
	} else if (p_name == "openPopup") {
		// Test hook (protocol v2.1): opens a real sub-window so the fake broker
		// can assert the whole godot.window.v1 lifecycle end to end.
		_open_demo_popup(p_payload);
	} else if (p_name == "closePopup") {
		Object *object = last_demo_popup.is_valid() ? ObjectDB::get_instance(last_demo_popup) : nullptr;
		Window *popup = Object::cast_to<Window>(object);
		if (popup) {
			popup->queue_free();
			last_demo_popup = ObjectID();
		}
	} else if (p_name == "query") {
		// Test hook for the fake broker (scripts/godot.mjs verify): reports the
		// controller state as a worker extension event so coordinate mapping and
		// button semantics stay observable end to end.
		// Input events may still sit in the accumulated queue (flushed once per
		// main-loop iteration); make the state reply deterministic by forcing
		// the pending dispatch now, before sampling the controller.
		Input::get_singleton()->flush_buffered_events();
		Dictionary state = demo_controller->capture_state();
		if (!last_drop_paths.is_empty()) {
			Array dropped;
			for (int i = 0; i < (int)last_drop_paths.size(); i++) {
				dropped.push_back(last_drop_paths[i]);
			}
			state["lastDropPaths"] = dropped;
		}
		if (display_server) {
			const Point2i mouse = display_server->mouse_get_position();
			Array mouse_array;
			mouse_array.push_back(mouse.x);
			mouse_array.push_back(mouse.y);
			state["mouse"] = mouse_array;
			// Multi-window report (protocol v2.1): rect/focus/visibility per window.
			Array windows;
			const Vector<DisplayServerEnums::WindowID> ids = display_server->get_window_list();
			for (int i = 0; i < ids.size(); i++) {
				Dictionary entry;
				entry["window"] = (int64_t)ids[i];
				const Rect2i rect(display_server->window_get_position(ids[i]), display_server->window_get_size(ids[i]));
				Array rect_array;
				rect_array.push_back(rect.position.x);
				rect_array.push_back(rect.position.y);
				rect_array.push_back(rect.size.x);
				rect_array.push_back(rect.size.y);
				entry["rect"] = rect_array;
				entry["focused"] = display_server->window_is_focused(ids[i]);
				entry["visible"] = display_server->window_can_draw(ids[i]);
				windows.push_back(entry);
			}
			state["windows"] = windows;
		}
		_emit_extension_event("state", state);
	} else {
		diagnostic_print("DSH viewport: unknown extension operation '" + p_name + "'.");
	}
}

void DSHViewportWorker::_handle_editor_extension(const String &p_name, const Dictionary &p_payload) {
	if (p_name == "gameLifecycle") {
		// Client -> worker lifecycle report for the spawned game session; the
		// editor's EmbeddedProcessDSH turns it into embedding_completed/failed.
		const bool running = (bool)p_payload.get("running", false);
		const int64_t code = (int64_t)p_payload.get("code", 0);
		if (game_lifecycle_listener.is_valid()) {
			game_lifecycle_listener.call(running, code);
		} else if (debug_game_echo) {
			_debug_game_lifecycle(running, code); // Verify hook round trip.
		}
	} else if (p_name == "debugGame") {
		// Verify-only hook (scripts/godot.mjs --editor gameTab step): drives the
		// same emit paths EmbeddedProcessDSH uses, without a GameView play session.
		// Never clobbers a real game session's listener.
		const String phase = p_payload.get("phase", "");
		const Dictionary rect_dict = p_payload.get("rect", Dictionary());
		const Rect2i rect((int)rect_dict.get("x", 0), (int)rect_dict.get("y", 0),
				(int)rect_dict.get("width", 320), (int)rect_dict.get("height", 240));
		if (phase == "start") {
			Vector<String> args;
			args.push_back("--dsh-verify-game");
			editor_spawn_game(args, rect);
			debug_game_echo = true; // Echo only while no real game session owns the listener.
		} else if (phase == "rect") {
			editor_set_game_rect(rect);
		} else if (phase == "stop") {
			// The echo stays on: the client reports gameLifecycle{running:false}
			// AFTER receiving gameClose, and the verify step asserts that final
			// round trip. It only ever fires while no real game session owns the
			// listener slot, and set_game_lifecycle_listener() clears it.
			editor_close_game();
		} else {
			diagnostic_print("DSH viewport: unknown debugGame phase '" + phase + "'.");
		}
	} else {
		diagnostic_print("DSH viewport: unknown editor operation '" + p_name + "'.");
	}
}

void DSHViewportWorker::_debug_game_lifecycle(bool p_running, int64_t p_code) {
	// Verify hook: makes the client -> worker gameLifecycle round trip observable
	// on the wire as godot.editor.v1/gameDebug (no GameView UI in the loop).
	Dictionary payload;
	payload["running"] = p_running;
	payload["code"] = p_code;
	emit_editor_event("gameDebug", payload);
}

void DSHViewportWorker::editor_spawn_game(const Vector<String> &p_args, const Rect2i &p_rect) {
	Array args;
	for (int i = 0; i < (int)p_args.size(); i++) {
		args.push_back(p_args[i]);
	}
	Dictionary rect;
	rect["x"] = p_rect.position.x;
	rect["y"] = p_rect.position.y;
	rect["width"] = p_rect.size.x;
	rect["height"] = p_rect.size.y;
	Dictionary payload;
	payload["args"] = args;
	payload["rect"] = rect;
	emit_editor_event("spawnGame", payload);
}

void DSHViewportWorker::editor_set_game_rect(const Rect2i &p_rect) {
	Dictionary rect;
	rect["x"] = p_rect.position.x;
	rect["y"] = p_rect.position.y;
	rect["width"] = p_rect.size.x;
	rect["height"] = p_rect.size.y;
	Dictionary payload;
	payload["rect"] = rect;
	emit_editor_event("gameRect", payload);
}

void DSHViewportWorker::editor_close_game() {
	emit_editor_event("gameClose", Dictionary());
}

void DSHViewportWorker::_handle_viewport_extension(const String &p_name, const Dictionary &p_payload) {
	if (p_name == "setRelativeMouseMode") {
		const bool enabled = p_payload.get("value", false);
		if (display_server) {
			display_server->dsh_set_relative_mouse_mode(enabled);
		}
	} else if (p_name == "setRightDrag") {
		const String mode = p_payload.get("value", "pan");
		if (demo_controller) {
			demo_controller->set_right_drag_pan(mode != "none");
		}
	} else if (p_name == "clipboard") {
		// Clipboard bridge (protocol v2.1): the client mirrors the system
		// clipboard content so clipboard_get() can answer synchronously.
		if (display_server) {
			display_server->dsh_set_clipboard_text(p_payload.get("text", ""));
		}
	} else {
		diagnostic_print("DSH viewport: unknown viewport operation '" + p_name + "'.");
	}
}

void DSHViewportWorker::emit_viewport_extension(const String &p_name, const Dictionary &p_payload) {
	_emit_extension_event_ns(VIEWPORT_NAMESPACE, p_name, p_payload);
}

void DSHViewportWorker::emit_window_event(const String &p_name, const Dictionary &p_payload) {
	_emit_extension_event_ns(WINDOW_NAMESPACE, p_name, p_payload);
}

void DSHViewportWorker::emit_editor_event(const String &p_name, const Dictionary &p_payload) {
	_emit_extension_event_ns(EDITOR_NAMESPACE, p_name, p_payload);
}

void DSHViewportWorker::_emit_extension_event(const String &p_name, const Dictionary &p_payload) {
	_emit_extension_event_ns(EXTENSION_NAMESPACE, p_name, p_payload);
}

void DSHViewportWorker::_emit_extension_event_ns(const String &p_ns, const String &p_name, const Dictionary &p_payload) {
	protocol_print(String("{\"type\":\"extension\",\"namespace\":\"") + p_ns + "\",\"name\":"
			+ JSON::stringify(p_name) + ",\"payload\":" + JSON::stringify(p_payload) + "}");
}

Vector2 DSHViewportWorker::_map_css_to_engine(double p_x, double p_y) const {
	return Vector2((float)(p_x * map_scale_x), (float)(p_y * map_scale_y));
}

void DSHViewportWorker::_release_all_input_state() {
	Input *input = Input::get_singleton();
	if (!input) {
		return;
	}
	// Input::release_pressed_events() clears keys/actions but never the mouse
	// button mask (a real OS delivers the matching button-ups when the window
	// loses focus; the synthetic focus path must do the same explicitly, or a
	// drag would survive the unfocus forever).
	const int mask = (int)input->get_mouse_button_mask();
	static const MouseButton buttons_by_bit[5] = { MouseButton::LEFT, MouseButton::RIGHT, MouseButton::MIDDLE, MouseButton::MB_XBUTTON1, MouseButton::MB_XBUTTON2 };
	for (int bit = 0; bit < 5 && mask != 0; bit++) {
		if ((mask & (1 << bit)) == 0) {
			continue;
		}
		Ref<InputEventMouseButton> event;
		event.instantiate();
		event->set_window_id(DisplayServerEnums::MAIN_WINDOW_ID);
		event->set_button_index(buttons_by_bit[bit]);
		event->set_pressed(false);
		event->set_position(last_engine_pointer);
		event->set_global_position(last_engine_pointer);
		event->set_button_mask((MouseButtonMask)(mask & ~(1 << bit)));
		input->parse_input_event(event);
	}
	input->release_pressed_events();
}

void DSHViewportWorker::_apply_focus(bool p_focused) {
	MainLoop *main_loop = OS::get_singleton()->get_main_loop();
	if (!main_loop) {
		return;
	}
	// Mirror a real OS focus change: SceneTree flips Input::application_focused
	// and releases pressed input on FOCUS_OUT (it is a friend of Input; the
	// flag is private, so the notification path is the only legal route).
	main_loop->notification(p_focused ? MainLoop::NOTIFICATION_APPLICATION_FOCUS_IN : MainLoop::NOTIFICATION_APPLICATION_FOCUS_OUT);
	if (!p_focused) {
		// release_pressed_events() never clears the mouse button mask (the OS
		// would deliver button-ups; nothing does in this synthetic viewport).
		_release_all_input_state();
	}
}

void DSHViewportWorker::_handle_input_event(const Dictionary &p_event) {
	const String type = p_event.get("type", "");
	if (type == "pointer") {
		_inject_pointer(p_event, _event_window(p_event));
	} else if (type == "wheel") {
		_inject_wheel(p_event, _event_window(p_event));
	} else if (type == "key") {
		_inject_key(p_event, _event_window(p_event));
	} else if (type == "text") {
		_inject_text(p_event, _event_window(p_event));
	} else if (type == "ime") {
		_inject_ime(p_event, _event_window(p_event));
	} else if (type == "drop") {
		_inject_drop(p_event, _event_window(p_event));
	} else if (type == "focus") {
		const bool focused = p_event.get("focused", true);
		if (p_event.has("window") && !focused) {
			const uint32_t window = p_event.get("window", 0);
			if (window == focused_window) {
				dsh_set_focused_window(UINT32_MAX);
			}
			return;
		}
		_apply_focus(focused);
		if (focused) {
			const uint32_t window = p_event.get("window", focused_window);
			dsh_set_focused_window(window);
		} else {
			dsh_set_focused_window(UINT32_MAX);
		}
	} else {
		diagnostic_print("DSH viewport: unknown input event type '" + type + "'.");
	}
}

uint32_t DSHViewportWorker::_event_window(const Dictionary &p_event) const {
	if (!p_event.has("window")) {
		return focused_window;
	}
	const uint32_t window = p_event.get("window", 0);
	if (window >= MAX_WINDOWS || (window != 0 && !sub_windows.has(window))) {
		return focused_window;
	}
	return window;
}

void DSHViewportWorker::_inject_pointer(const Dictionary &p_event, DisplayServerEnums::WindowID p_window) {
	const String phase = p_event.get("phase", "move");
	// A cancel never carries a usable button value: release everything up front
	// (same path as a focus loss) instead of bailing out on button == NONE.
	if (phase == "cancel") {
		_release_all_input_state();
		return;
	}
	const double x = p_event.get("x", 0.0);
	const double y = p_event.get("y", 0.0);
	const int buttons = p_event.get("buttons", 0);
	const uint32_t modifiers = p_event.get("modifiers", 0);
	// interaction.map.v2: the protocol position is viewport-local CSS px; the
	// engine window is physical px. Factors were locked when the current epoch's
	// resize was applied. Protocol v2.1: the session-level factors map every
	// window's layer-local coordinates (window positions are engine-driven).
	const Vector2 position = _map_css_to_engine(x, y);
	if (display_server) {
		// The global mouse position tracks the virtual screen: layer-local point
		// plus the owning window's engine position.
		Point2i origin;
		if (p_window != DisplayServerEnums::MAIN_WINDOW_ID) {
			origin = display_server->window_get_position(p_window);
		}
		display_server->dsh_set_mouse_position(origin + Point2i((int)Math::round(position.x), (int)Math::round(position.y)));
	}

	if (phase == "move") {
		Ref<InputEventMouseMotion> motion;
		motion.instantiate();
		motion->set_window_id(p_window);
		motion->set_position(position);
		motion->set_global_position(position);
		motion->set_button_mask((MouseButtonMask)buttons);
		motion->set_shift_pressed(modifiers & 1);
		motion->set_ctrl_pressed(modifiers & 2);
		motion->set_alt_pressed(modifiers & 4);
		motion->set_meta_pressed(modifiers & 8);
		Vector2 relative;
		if (p_event.has("movementX") || p_event.has("movementY")) {
			// Pointer-lock deltas (CSS px) take precedence: the OS position is
			// frozen while the pointer is locked, so position deltas are useless.
			relative = _map_css_to_engine((double)p_event.get("movementX", 0.0), (double)p_event.get("movementY", 0.0));
		} else {
			relative = position - last_engine_pointer;
		}
		last_engine_pointer = position;
		motion->set_relative(relative);
		Input::get_singleton()->parse_input_event(motion);
		return;
	}
	last_engine_pointer = position;

	int button_index = (int)p_event.get("button", -1);
	MouseButton button = MouseButton::NONE;
	if (button_index == 0) {
		button = MouseButton::LEFT;
	} else if (button_index == 1) {
		button = MouseButton::MIDDLE;
	} else if (button_index == 2) {
		button = MouseButton::RIGHT;
	} else if (button_index == 3) {
		button = MouseButton::MB_XBUTTON1;
	} else if (button_index == 4) {
		button = MouseButton::MB_XBUTTON2;
	}
	if (button == MouseButton::NONE) {
		return;
	}
	const bool pressed = (phase == "down");
	Ref<InputEventMouseButton> event;
	event.instantiate();
	event->set_window_id(p_window);
	event->set_button_index(button);
	event->set_pressed(pressed);
	event->set_position(position);
	event->set_global_position(position);
	event->set_button_mask((MouseButtonMask)(pressed ? (buttons | (1 << ((int)button - 1))) : (buttons & ~(1 << ((int)button - 1)))));
	event->set_shift_pressed(modifiers & 1);
	event->set_ctrl_pressed(modifiers & 2);
	event->set_alt_pressed(modifiers & 4);
	event->set_meta_pressed(modifiers & 8);
	Input::get_singleton()->parse_input_event(event);
}

void DSHViewportWorker::_inject_wheel(const Dictionary &p_event, DisplayServerEnums::WindowID p_window) {
	const double delta_x = p_event.get("deltaX", 0.0);
	const double delta_y = p_event.get("deltaY", 0.0);
	const double dominant = Math::abs(delta_y) >= Math::abs(delta_x) ? delta_y : delta_x;
	if (dominant == 0.0) {
		return;
	}
	Ref<InputEventMouseButton> event;
	event.instantiate();
	event->set_window_id(p_window);
	event->set_button_index(dominant > 0 ? (Math::abs(delta_y) >= Math::abs(delta_x) ? MouseButton::WHEEL_DOWN : MouseButton::WHEEL_RIGHT)
										 : (Math::abs(delta_y) >= Math::abs(delta_x) ? MouseButton::WHEEL_UP : MouseButton::WHEEL_LEFT));
	event->set_factor(CLAMP(Math::abs(dominant) / 100.0, 0.1, 10.0));
	// Use the coordinates carried by the event itself (mapped CSS -> physical);
	// the wheel may sit far away from the last pointer position.
	const Vector2 position = _map_css_to_engine(p_event.get("x", 0.0), p_event.get("y", 0.0));
	event->set_position(position);
	event->set_global_position(position);
	if (display_server) {
		Point2i origin;
		if (p_window != DisplayServerEnums::MAIN_WINDOW_ID) {
			origin = display_server->window_get_position(p_window);
		}
		display_server->dsh_set_mouse_position(origin + Point2i((int)Math::round(position.x), (int)Math::round(position.y)));
	}

	event->set_pressed(true);
	Input::get_singleton()->parse_input_event(event);
	Ref<InputEventMouseButton> release = event->duplicate();
	release->set_pressed(false);
	Input::get_singleton()->parse_input_event(release);
}

void DSHViewportWorker::_inject_text(const Dictionary &p_event, DisplayServerEnums::WindowID p_window) {
	const String text = p_event.get("text", "");
	if (text.is_empty()) {
		return;
	}
	// Godot 4 has no dedicated text input event: committed text reaches LineEdit
	// and friends as pressed key events carrying a unicode codepoint. Newlines
	// and tabs are mapped to their Enter/Tab key equivalents.
	const int length = text.length();
	for (int i = 0; i < length; i++) {
		const char32_t codepoint = text[i];
		Ref<InputEventKey> event;
		event.instantiate();
		event->set_window_id(p_window);
		event->set_pressed(true);
		event->set_echo(false);
		if (codepoint == '\n' || codepoint == '\r') {
			event->set_keycode(Key::ENTER);
			event->set_physical_keycode(Key::ENTER);
			event->set_unicode(10);
		} else if (codepoint == '\t') {
			event->set_keycode(Key::TAB);
			event->set_physical_keycode(Key::TAB);
			event->set_unicode(9);
		} else {
			event->set_keycode(Key::NONE);
			event->set_physical_keycode(Key::NONE);
			event->set_unicode(codepoint);
		}
		Input::get_singleton()->parse_input_event(event);
		Ref<InputEventKey> release = event->duplicate();
		release->set_pressed(false);
		Input::get_singleton()->parse_input_event(release);
	}
}

void DSHViewportWorker::_inject_ime(const Dictionary &p_event, DisplayServerEnums::WindowID p_window) {
	if (!display_server) {
		return;
	}
	const String state = p_event.get("state", "");
	const String text = p_event.get("text", "");
	if (state == "start" || state == "update") {
		// Preedit surface: the client reports in-progress composition; LineEdit
		// and TextEdit re-query ime_get_text()/ime_get_selection() when the
		// NOTIFICATION_OS_IME_UPDATE propagation reaches them.
		display_server->dsh_set_ime_preedit(text, p_event.get("cursorStart", 0), p_event.get("cursorEnd", 0), p_window);
	} else if (state == "commit") {
		display_server->dsh_set_ime_preedit(String(), 0, 0, p_window);
		if (!text.is_empty()) {
			Dictionary commit;
			commit["type"] = "text";
			commit["text"] = text;
			_inject_text(commit, p_window);
		}
	} else if (state == "cancel") {
		display_server->dsh_set_ime_preedit(String(), 0, 0, p_window);
	} else {
		diagnostic_print("DSH viewport: unknown ime state '" + state + "'.");
		return;
	}
	MainLoop *main_loop = OS::get_singleton()->get_main_loop();
	if (main_loop) {
		main_loop->notification(MainLoop::NOTIFICATION_OS_IME_UPDATE);
	}
}

void DSHViewportWorker::_inject_drop(const Dictionary &p_event, DisplayServerEnums::WindowID p_window) {
	// The editor resolves the drop target from the current mouse position, so
	// land a motion event at the drop point first (same contract as a real OS,
	// where the drag cursor is there when the button releases).
	const double x = p_event.get("x", 0.0);
	const double y = p_event.get("y", 0.0);
	const Vector2 position = _map_css_to_engine(x, y);
	if (display_server) {
		Point2i origin;
		if (p_window != DisplayServerEnums::MAIN_WINDOW_ID) {
			origin = display_server->window_get_position(p_window);
		}
		display_server->dsh_set_mouse_position(origin + Point2i((int)Math::round(position.x), (int)Math::round(position.y)));
	}
	Ref<InputEventMouseMotion> motion;
	motion.instantiate();
	motion->set_window_id(p_window);
	motion->set_position(position);
	motion->set_global_position(position);
	motion->set_button_mask(MouseButtonMask::NONE);
	motion->set_relative(position - last_engine_pointer);
	last_engine_pointer = position;
	Input::get_singleton()->parse_input_event(motion);

	const Array paths_array = p_event.get("paths", Array());
	Vector<String> paths;
	for (int i = 0; i < (int)paths_array.size(); i++) {
		paths.push_back(String(paths_array[i]));
	}
	if (paths.is_empty()) {
		return;
	}
	// Test hook: keep the last drop observable for the fake broker.
	last_drop_paths.clear();
	for (int i = 0; i < paths.size(); i++) {
		last_drop_paths.push_back(paths[i]);
	}
	if (display_server) {
		display_server->dsh_dispatch_drop_files(paths, p_window);
	}
}

void DSHViewportWorker::_inject_key(const Dictionary &p_event, DisplayServerEnums::WindowID p_window) {
	const String code = p_event.get("code", "");
	const CharString code_utf8 = code.utf8();
	Key key = dom_code2godot_scancode(code_utf8.get_data(), code_utf8.get_data(), true);
	if (key == Key::NONE) {
		diagnostic_print("DSH viewport: unmapped DOM code '" + code + "'.");
		return;
	}
	const bool down = p_event.get("down", false);
	const uint32_t modifiers = p_event.get("modifiers", 0);
	// Printable character carried by the DOM key event (input.text capability):
	// text fields only insert on InputEventKey::unicode, keycode alone drives
	// shortcuts. JSON conversion already merged surrogate pairs into char32.
	const String key_text = p_event.get("key", "");
	Ref<InputEventKey> event;
	event.instantiate();
	event->set_window_id(p_window);
	event->set_pressed(down);
	if (key_text.length() == 1) {
		event->set_unicode(key_text[0]);
	}
	event->set_keycode(key);
	event->set_physical_keycode(key);
	event->set_echo(down && (bool)p_event.get("repeat", false));
	event->set_shift_pressed(modifiers & 1);
	event->set_ctrl_pressed(modifiers & 2);
	event->set_alt_pressed(modifiers & 4);
	event->set_meta_pressed(modifiers & 8);
	Input::get_singleton()->parse_input_event(event);
}

void DSHViewportWorker::_apply_pending_resize() {
	if (!has_pending_resize || stop_requested.load()) {
		return;
	}
	if (!are_all_slots_free()) {
		return; // The broker still holds leases; apply once everything is released.
	}
	wait_all_presents_complete();

	const uint32_t width = pending_resize_width;
	const uint32_t height = pending_resize_height;
	has_pending_resize = false;
	if (width == 0 || height == 0) {
		// Zero size suspends production without releasing the surface/epoch.
		display_server->window_set_size(Size2i(), DisplayServerEnums::MAIN_WINDOW_ID);
		surface_width.store(0);
		surface_height.store(0);
		return;
	}

	// Establish the new epoch BEFORE any frame can be emitted for it, and
	// announce the surface up front: the broker rejects frames with epoch < 1
	// and frames must never race their own surface announcement. The coordinate
	// mapping factors are locked at exactly this point, together with the epoch,
	// so input mapping and surface geometry can never disagree.
	const uint64_t new_epoch = ++epoch_counter;
	current_epoch.store(new_epoch);
	surface_width.store(width);
	surface_height.store(height);
	if (has_pending_css) {
		map_scale_x = width / pending_css_width;
		map_scale_y = height / pending_css_height;
	} else {
		// Legacy graphics101 behavior: one scale for both axes.
		map_scale_x = pending_scale;
		map_scale_y = pending_scale;
	}
	if (display_server) {
		// Content scale follows Godot semantics (physical px per logical unit):
		// our window coordinates are physical, so the logical (css) size is
		// physical / map_scale_x.
		display_server->dsh_set_content_scale(CLAMP(map_scale_x, 0.25, 8.0));
	}
	String surface_line = String("{\"type\":\"surface\",\"epoch\":") + uitos(new_epoch)
			+ ",\"width\":" + uitos(width) + ",\"height\":" + uitos(height)
			+ ",\"slots\":" + uitos(MAX_SLOTS);
	if (has_pending_css) {
		surface_line += ",\"cssWidth\":" + JSON::stringify(Variant(pending_css_width))
				+ ",\"cssHeight\":" + JSON::stringify(Variant(pending_css_height));
	}
	surface_line += "}";
	protocol_print(surface_line);
	display_server->window_set_size(Size2i((int)width, (int)height), DisplayServerEnums::MAIN_WINDOW_ID);
}

void DSHViewportWorker::_apply_sub_window_resize(uint32_t p_window) {
	SubWindowState *sw = sub_windows.getptr(p_window);
	if (!sw || !sw->has_pending_resize || stop_requested.load()) {
		return;
	}
	if (sw->sink == nullptr || !sw->sink->are_all_slots_free()) {
		return; // Apply once the broker returned every lease of this ring.
	}
	const uint32_t width = sw->pending_resize_width;
	const uint32_t height = sw->pending_resize_height;
	sw->has_pending_resize = false;
	if (width == 0 || height == 0) {
		return; // Zero size suspends the sub-window without touching the ring.
	}
	display_server->window_set_size(Size2i((int)width, (int)height), (DisplayServerEnums::WindowID)p_window);
}

/**** Multi-window registry (protocol v2.1) ****/

DSHWindowSink *DSHViewportWorker::dsh_create_window_sink(uint32_t p_window) {
	ERR_FAIL_COND_V_MSG(p_window == 0 || p_window >= MAX_WINDOWS, nullptr, "DSH viewport: sub-window id out of range.");
	std::lock_guard<std::mutex> lock(sinks_mutex);
	ERR_FAIL_COND_V_MSG(sub_windows.has(p_window), nullptr, "DSH viewport: duplicate sub-window id.");
	SubWindowState state;
	state.sink = memnew(DSHWindowSink);
	// The factory initializes lazily inside the first create_frames(); the
	// broker PID is needed for that first import.
	state.sink->set_factory(&frames_factory);
	state.sink->set_broker_pid(broker_pid);
	sub_windows.insert(p_window, state);
	active_sinks.push_back(std::make_pair(p_window, state.sink));
	return state.sink;
}

void DSHViewportWorker::dsh_destroy_window_sink(uint32_t p_window) {
	DSHWindowSink *sink = nullptr;
	{
		std::lock_guard<std::mutex> lock(sinks_mutex);
		SubWindowState *sw = sub_windows.getptr(p_window);
		if (!sw) {
			return;
		}
		sink = sw->sink;
		sw->sink = nullptr;
		for (uint32_t i = 0; i < active_sinks.size(); i++) {
			if (active_sinks[i].first == p_window) {
				active_sinks.remove_at(i);
				break;
			}
		}
		sub_windows.erase(p_window);
	}
	if (sink) {
		sink->shutdown();
		memdelete(sink);
	}
}

void DSHViewportWorker::dsh_set_focused_window(uint32_t p_window) {
	if (p_window != UINT32_MAX && p_window >= MAX_WINDOWS) {
		return;
	}
	const uint32_t previous = focused_window;
	if (previous == p_window) {
		return;
	}
	focused_window = p_window;
	if (display_server) {
		display_server->dsh_apply_window_focus(p_window);
	}
	if (previous != UINT32_MAX) {
		Dictionary payload;
		payload["window"] = (int64_t)previous;
		payload["focused"] = false;
		emit_window_event("focus", payload);
	}
	if (p_window != UINT32_MAX) {
		Dictionary payload;
		payload["window"] = (int64_t)p_window;
		payload["focused"] = true;
		emit_window_event("focus", payload);
	}
}

void DSHViewportWorker::dsh_notify_engine_focus(uint32_t p_window, bool p_focused) {
	// Engine-internal focus change (e.g. a popup grabbing focus on show): keep
	// the client layers aligned. The engine already fired its own callbacks.
	if (p_focused) {
		if (focused_window != p_window) {
			focused_window = p_window;
			Dictionary payload;
			payload["window"] = (int64_t)p_window;
			payload["focused"] = true;
			emit_window_event("focus", payload);
		}
	} else if (focused_window == p_window) {
		focused_window = UINT32_MAX;
		Dictionary payload;
		payload["window"] = (int64_t)p_window;
		payload["focused"] = false;
		emit_window_event("focus", payload);
	}
}

/**** Frame sink implementation (main window) ****/

Error DSHViewportWorker::create_frames(VkPhysicalDevice p_physical_device, VkDevice p_device, uint32_t p_graphics_queue_family, uint32_t p_width, uint32_t p_height, uint32_t p_slot_count) {
	vk_device = p_device;
	if (completion_semaphore == VK_NULL_HANDLE) {
		VkSemaphoreTypeCreateInfo type_info{};
		type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_info.pNext = &type_info;
		VkResult err = vkCreateSemaphore(p_device, &semaphore_info, nullptr, &completion_semaphore);
		ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, ERR_CANT_CREATE, "DSH viewport: couldn't create the completion semaphore.");
	}

	const bool first_init = !frames_pool_ready;
	if (first_init) {
		Error init_err = frames_factory.initialize(p_physical_device, p_device, broker_pid);
		ERR_FAIL_COND_V_MSG(init_err != OK, init_err, "DSH viewport: D3D11 initialization failed.");
		frames_pool_ready = true;
	}

	// TEMP DEBUG (black-screen investigation): log every main-ring recreate with
	// outstanding lease state so mid-run destroy sources can be identified.
	{
		std::lock_guard<std::mutex> debug_lock(sink_mutex);
		uint32_t busy = 0;
		for (uint32_t i = 0; i < MAX_SLOTS; i++) {
			if (!slot_free[i].load()) busy++;
		}
		diagnostic_print(String("[dsh-debug] main create_frames ") + uitos(p_width) + "x" + uitos(p_height)
				+ " slots=" + uitos(p_slot_count) + " pending=" + uitos(pending_presents.size())
				+ " busy_slots=" + uitos(busy) + " had_pool=" + (DSHSharedFramesWin32::has_frames(frames_pool) ? "yes" : "no"));
	}

	Error err = frames_factory.create_frames(frames_pool, p_width, p_height, p_slot_count);
	ERR_FAIL_COND_V_MSG(err != OK, err, "DSH viewport: couldn't create the D3D11 shared frame pool.");

	std::lock_guard<std::mutex> lock(sink_mutex);
	const uint32_t slot_count = DSHSharedFramesWin32::get_slot_count(frames_pool);
	for (uint32_t i = 0; i < MAX_SLOTS; i++) {
		slot_free[i].store(i < slot_count);
	}
	pending_presents.clear();
	sink_cv.notify_all();
	return OK;
}

void DSHViewportWorker::destroy_frames() {
	{
		std::lock_guard<std::mutex> lock(sink_mutex);
		pending_presents.clear();
		for (uint32_t i = 0; i < MAX_SLOTS; i++) {
			slot_free[i].store(true);
		}
		sink_cv.notify_all();
	}
	frames_factory.destroy_frames(frames_pool);
}

bool DSHViewportWorker::has_frames() const {
	return DSHSharedFramesWin32::has_frames(frames_pool);
}

uint32_t DSHViewportWorker::get_slot_count() const {
	return DSHSharedFramesWin32::get_slot_count(frames_pool);
}

VkImage DSHViewportWorker::get_slot_image(uint32_t p_slot) const {
	return DSHSharedFramesWin32::get_slot_image(frames_pool, p_slot);
}

uint64_t DSHViewportWorker::get_slot_nt_handle(uint32_t p_slot) const {
	return DSHSharedFramesWin32::get_slot_nt_handle(frames_pool, p_slot);
}

uint64_t DSHViewportWorker::allocate_present(uint32_t p_slot) {
	std::lock_guard<std::mutex> lock(sink_mutex);
	const uint64_t value = ++next_present_value;
	pending_presents.push_back(std::make_pair(value, p_slot));
	return value;
}

void DSHViewportWorker::cancel_present(uint64_t p_value) {
	std::lock_guard<std::mutex> lock(sink_mutex);
	for (auto it = pending_presents.begin(); it != pending_presents.end(); ++it) {
		if (it->first == p_value) {
			pending_presents.erase(it);
			break;
		}
	}
	sink_cv.notify_all();
}

uint32_t DSHViewportWorker::acquire_slot_blocking() {
	const uint64_t deadline = steady_usec() + SLOT_LEASE_TIMEOUT_NS / 1000;
	std::unique_lock<std::mutex> lock(sink_mutex);
	for (;;) {
		const uint32_t slot_count = DSHSharedFramesWin32::get_slot_count(frames_pool);
		for (uint32_t i = 0; i < slot_count && i < MAX_SLOTS; i++) {
			if (slot_free[i].load()) {
				slot_free[i].store(false);
				return i;
			}
		}
		const uint64_t now = steady_usec();
		if (now >= deadline) {
			return UINT32_MAX;
		}
		sink_cv.wait_for(lock, std::chrono::milliseconds(20));
		if (pump_exit.load()) {
			return UINT32_MAX;
		}
	}
}

void DSHViewportWorker::mark_slot_released(uint32_t p_slot, uint64_t p_frame_id) {
	(void)p_frame_id;
	std::lock_guard<std::mutex> lock(sink_mutex);
	if (p_slot < MAX_SLOTS) {
		slot_free[p_slot].store(true);
	}
	sink_cv.notify_all();
}

bool DSHViewportWorker::are_all_slots_free() const {
	if (!DSHSharedFramesWin32::has_frames(frames_pool)) {
		return true;
	}
	std::lock_guard<std::mutex> lock(sink_mutex);
	if (!pending_presents.empty()) {
		return false;
	}
	const uint32_t slot_count = DSHSharedFramesWin32::get_slot_count(frames_pool);
	for (uint32_t i = 0; i < slot_count && i < MAX_SLOTS; i++) {
		if (!slot_free[i].load()) {
			return false;
		}
	}
	return true;
}

void DSHViewportWorker::wait_all_presents_complete() {
	std::unique_lock<std::mutex> lock(sink_mutex);
	while (!pending_presents.empty() && !pump_exit.load()) {
		sink_cv.wait_for(lock, std::chrono::milliseconds(50));
	}
}

#endif // WINDOWS_ENABLED && DSH_VIEWPORT_ENABLED
