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
}

void DSHDemoController::_update_camera() {
	Camera3D *camera = Object::cast_to<Camera3D>(get_node_or_null(camera_path));
	if (!camera) {
		return;
	}
	const double clamped_pitch = CLAMP(pitch, -1.45, 1.45);
	Vector3 eye(Math::sin(yaw) * Math::cos(clamped_pitch), Math::sin(clamped_pitch), Math::cos(yaw) * Math::cos(clamped_pitch));
	camera->set_position(eye * distance);
	camera->look_at(Vector3(), Vector3(0, 1, 0));
}

void DSHDemoController::input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> button = p_event;
	if (button.is_valid()) {
		if (button->get_button_index() == MouseButton::WHEEL_UP && button->is_pressed()) {
			distance = MAX(1.2, distance - 0.35 * button->get_factor());
			_update_camera();
		} else if (button->get_button_index() == MouseButton::WHEEL_DOWN && button->is_pressed()) {
			distance = MIN(12.0, distance + 0.35 * button->get_factor());
			_update_camera();
		} else if (button->get_button_index() == MouseButton::LEFT) {
			dragging = button->is_pressed();
			last_pointer = button->get_position();
		}
		return;
	}
	Ref<InputEventMouseMotion> motion = p_event;
	if (motion.is_valid() && dragging) {
		Vector2 relative = motion->get_position() - last_pointer;
		last_pointer = motion->get_position();
		yaw -= relative.x * 0.006;
		pitch = CLAMP(pitch + relative.y * 0.006, -1.45, 1.45);
		_update_camera();
	}
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
	frames.finalize();
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
				if (release_epoch == current_epoch.load()) {
					mark_slot_released(slot, frame);
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
		std::pair<uint64_t, uint32_t> present;
		{
			std::unique_lock<std::mutex> lock(sink_mutex);
			if (pending_presents.empty()) {
				sink_cv.wait_for(lock, std::chrono::milliseconds(50));
				continue;
			}
			present = pending_presents.front();
		}
		if (present.first <= last_waited_value) {
			// Defensive: values are strictly increasing; never wait backwards.
			std::lock_guard<std::mutex> lock(sink_mutex);
			if (!pending_presents.empty() && pending_presents.front().first == present.first) {
				pending_presents.pop_front();
			}
			sink_cv.notify_all();
			continue;
		}
		VkSemaphoreWaitInfo wait_info{};
		wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		wait_info.semaphoreCount = 1;
		wait_info.pSemaphores = &completion_semaphore;
		wait_info.pValues = &present.first;
		VkResult err = vkWaitSemaphores(vk_device, &wait_info, 2ULL * 1000 * 1000 * 1000); // 2 s timeout, then retry.
		if (err == VK_TIMEOUT) {
			continue;
		}
		if (err != VK_SUCCESS) {
			diagnostic_print(String("DSH viewport: frame completion wait failed (VkResult ") + itos(err) + ").");
			fatal_error.store(true);
			return;
		}
		last_waited_value = present.first;
		{
			std::lock_guard<std::mutex> lock(sink_mutex);
			if (!pending_presents.empty() && pending_presents.front().first == present.first) {
				pending_presents.pop_front();
			}
			sink_cv.notify_all();
		}
		_emit_frame(present.second);
	}
}

void DSHViewportWorker::_emit_frame(uint32_t p_slot) {
	if (stop_requested.load() || current_epoch.load() < 1 || !frames.has_frames() || p_slot >= frames.get_slot_count()) {
		return;
	}
	const uint64_t frame_id = ++frame_counter;
	const uint64_t timestamp = steady_usec();
	const uint64_t handle = frames.get_slot_nt_handle(p_slot);
	char line[512];
	snprintf(line, sizeof(line),
			"{\"type\":\"frame\",\"epoch\":%llu,\"slot\":%u,\"frameId\":%llu,"
			"\"width\":%u,\"height\":%u,\"inputSeq\":%llu,\"timestampUs\":%llu,\"handle\":\"%llu\"}",
			(unsigned long long)current_epoch.load(), p_slot, (unsigned long long)frame_id,
			(unsigned)frames.get_width(), (unsigned)frames.get_height(),
			(unsigned long long)last_input_seq.load(), (unsigned long long)timestamp, (unsigned long long)handle);
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
			+ EXTENSION_NAMESPACE + "\"],\"backend\":\"godot-vulkan\",\"vendorId\":" + uitos(vendor_id)
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

	const uint64_t now = OS::get_singleton()->get_ticks_usec();
	if (stats_last_usec == 0) {
		stats_last_usec = now;
	} else if (now - stats_last_usec >= 1000000) {
		stats_last_usec = now;
		protocol_print(String("{\"type\":\"stats\",\"frames\":") + uitos(frame_counter.load())
				+ ",\"inputSeq\":" + uitos(input_seq) + ",\"validationErrors\":0}");
	}

	const bool active = viewport_visible.load() && frames.has_frames() && !has_pending_resize;
	const int target_fps = active ? 60 : 10;
	if (Engine::get_singleton()->get_max_fps() != target_fps) {
		fprintf(stderr, "[dsh] fps target %d->%d visible=%d has_frames=%d pending_resize=%d\n", Engine::get_singleton()->get_max_fps(), target_fps,
				(int)viewport_visible.load(), (int)frames.has_frames(), (int)has_pending_resize);
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
		if (width > MAX_VIEWPORT_DIMENSION || height > MAX_VIEWPORT_DIMENSION) {
			diagnostic_print("DSH viewport: resize beyond the 2048x2048 quota ignored.");
			return;
		}
		pending_resize_width = width;
		pending_resize_height = height;
		has_pending_resize = true;
		return;
	}
	if (kind == "visible") {
		const bool visible = message.get("value", true);
		viewport_visible.store(visible);
		if (!visible) {
			Input::get_singleton()->release_pressed_events();
			if (demo_controller) {
				demo_controller->release_pointer();
			}
		}
		return;
	}
	if (kind == "release") {
		const uint64_t release_epoch = message.get("epoch", 0);
		const uint32_t slot = message.get("slot", UINT32_MAX);
		const uint64_t frame = message.get("frameId", 0);
		if (release_epoch == current_epoch.load()) {
			mark_slot_released(slot, frame);
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
			if (ns != EXTENSION_NAMESPACE) {
				diagnostic_print("DSH viewport: unknown extension namespace '" + ns + "'.");
				return;
			}
			const String name = message.get("name", "");
			Dictionary payload = message.get("payload", Dictionary());
			_handle_extension(name, payload);
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
	} else {
		diagnostic_print("DSH viewport: unknown extension operation '" + p_name + "'.");
	}
}

void DSHViewportWorker::_handle_input_event(const Dictionary &p_event) {
	const String type = p_event.get("type", "");
	if (type == "pointer") {
		_inject_pointer(p_event);
	} else if (type == "wheel") {
		_inject_wheel(p_event);
	} else if (type == "key") {
		_inject_key(p_event);
	} else if (type == "focus") {
		const bool focused = p_event.get("focused", true);
		if (!focused) {
			Input::get_singleton()->release_pressed_events();
		}
	} else {
		diagnostic_print("DSH viewport: unknown input event type '" + type + "'.");
	}
}

void DSHViewportWorker::_inject_pointer(const Dictionary &p_event) {
	const String phase = p_event.get("phase", "move");
	const double x = p_event.get("x", 0.0);
	const double y = p_event.get("y", 0.0);
	const int buttons = p_event.get("buttons", 0);
	const Vector2 position(x, y);
	if (display_server) {
		display_server->dsh_set_mouse_position(Point2i((int)x, (int)y));
	}

	if (phase == "move") {
		Ref<InputEventMouseMotion> motion;
		motion.instantiate();
		motion->set_window_id(DisplayServerEnums::MAIN_WINDOW_ID);
		motion->set_position(position);
		motion->set_global_position(position);
		motion->set_button_mask((MouseButtonMask)buttons);
		Input::get_singleton()->parse_input_event(motion);
		return;
	}

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
	event->set_window_id(DisplayServerEnums::MAIN_WINDOW_ID);
	event->set_button_index(button);
	event->set_pressed(pressed);
	event->set_position(position);
	event->set_global_position(position);
	event->set_button_mask((MouseButtonMask)(pressed ? (buttons | (1 << ((int)button - 1))) : (buttons & ~(1 << ((int)button - 1)))));
	Input::get_singleton()->parse_input_event(event);
	if (phase == "cancel" && demo_controller) {
		Input::get_singleton()->release_pressed_events();
	}
}

void DSHViewportWorker::_inject_wheel(const Dictionary &p_event) {
	const double delta_x = p_event.get("deltaX", 0.0);
	const double delta_y = p_event.get("deltaY", 0.0);
	const double dominant = Math::abs(delta_y) >= Math::abs(delta_x) ? delta_y : delta_x;
	if (dominant == 0.0) {
		return;
	}
	Ref<InputEventMouseButton> event;
	event.instantiate();
	event->set_window_id(DisplayServerEnums::MAIN_WINDOW_ID);
	event->set_button_index(dominant > 0 ? (Math::abs(delta_y) >= Math::abs(delta_x) ? MouseButton::WHEEL_DOWN : MouseButton::WHEEL_RIGHT)
										 : (Math::abs(delta_y) >= Math::abs(delta_x) ? MouseButton::WHEEL_UP : MouseButton::WHEEL_LEFT));
	event->set_factor(CLAMP(Math::abs(dominant) / 100.0, 0.1, 10.0));
	event->set_position(display_server ? Point2(display_server->mouse_get_position()) : Point2());
	event->set_global_position(event->get_position());

	event->set_pressed(true);
	Input::get_singleton()->parse_input_event(event);
	Ref<InputEventMouseButton> release = event->duplicate();
	release->set_pressed(false);
	Input::get_singleton()->parse_input_event(release);
}

void DSHViewportWorker::_inject_key(const Dictionary &p_event) {
	const String code = p_event.get("code", "");
	const CharString code_utf8 = code.utf8();
	Key key = dom_code2godot_scancode(code_utf8.get_data(), code_utf8.get_data(), true);
	if (key == Key::NONE) {
		diagnostic_print("DSH viewport: unmapped DOM code '" + code + "'.");
		return;
	}
	const bool down = p_event.get("down", false);
	const uint32_t modifiers = p_event.get("modifiers", 0);
	Ref<InputEventKey> event;
	event.instantiate();
	event->set_window_id(DisplayServerEnums::MAIN_WINDOW_ID);
	event->set_pressed(down);
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
	// and frames must never race their own surface announcement.
	const uint64_t new_epoch = ++epoch_counter;
	current_epoch.store(new_epoch);
	surface_width.store(width);
	surface_height.store(height);
	protocol_print(String("{\"type\":\"surface\",\"epoch\":") + uitos(new_epoch)
			+ ",\"width\":" + uitos(width) + ",\"height\":" + uitos(height)
			+ ",\"slots\":" + uitos(MAX_SLOTS) + "}");
	display_server->window_set_size(Size2i((int)width, (int)height), DisplayServerEnums::MAIN_WINDOW_ID);
}

/**** Frame sink implementation ****/

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
		Error init_err = frames.initialize(p_physical_device, p_device, broker_pid);
		ERR_FAIL_COND_V_MSG(init_err != OK, init_err, "DSH viewport: D3D11 initialization failed.");
		frames_pool_ready = true;
	}

	Error err = frames.create_frames(p_width, p_height, p_slot_count);
	ERR_FAIL_COND_V_MSG(err != OK, err, "DSH viewport: couldn't create the D3D11 shared frame pool.");

	std::lock_guard<std::mutex> lock(sink_mutex);
	const uint32_t slot_count = frames.get_slot_count();
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
	frames.destroy_frames();
}

bool DSHViewportWorker::has_frames() const {
	return frames.has_frames();
}

uint32_t DSHViewportWorker::get_slot_count() const {
	return frames.get_slot_count();
}

VkImage DSHViewportWorker::get_slot_image(uint32_t p_slot) const {
	return frames.get_slot_image(p_slot);
}

uint64_t DSHViewportWorker::get_slot_nt_handle(uint32_t p_slot) const {
	return frames.get_slot_nt_handle(p_slot);
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
		const uint32_t slot_count = frames.get_slot_count();
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
	if (!frames.has_frames()) {
		return true;
	}
	std::lock_guard<std::mutex> lock(sink_mutex);
	if (!pending_presents.empty()) {
		return false;
	}
	const uint32_t slot_count = frames.get_slot_count();
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
