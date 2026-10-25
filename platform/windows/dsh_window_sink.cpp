/**************************************************************************/
/*  dsh_window_sink.cpp                                                   */
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

#include "dsh_window_sink.h"

#include "core/error/error_macros.h"
#include "core/os/os.h"

#include <chrono>

DSHWindowSink::~DSHWindowSink() {
	shutdown();
}

void DSHWindowSink::shutdown() {
	destroy_frames();
	if (completion_semaphore != VK_NULL_HANDLE && vk_device != VK_NULL_HANDLE) {
		vkDestroySemaphore(vk_device, completion_semaphore, nullptr);
		completion_semaphore = VK_NULL_HANDLE;
	}
	vk_device = VK_NULL_HANDLE;
	factory = nullptr;
}

Error DSHWindowSink::create_frames(VkPhysicalDevice p_physical_device, VkDevice p_device, uint32_t p_graphics_queue_family, uint32_t p_width, uint32_t p_height, uint32_t p_slot_count) {
	ERR_FAIL_COND_V_MSG(p_device == VK_NULL_HANDLE, ERR_UNCONFIGURED, "DSH window sink: Vulkan device is required.");
	// Sub-window rings are capped at 2 slots (protocol v2.1 quota: main ring 4
	// + 2 per sub-window, 24 total across at most 16 windows).
	p_slot_count = MIN(p_slot_count, 2U);
	ERR_FAIL_COND_V_MSG(factory == nullptr, ERR_UNCONFIGURED, "DSH window sink: shared frame factory missing.");
	if (!factory->is_initialized()) {
		// A sub-window may appear before the broker ever resized the main ring;
		// initialize the shared D3D11 side on demand with this ring's device.
		Error init_err = factory->initialize(p_physical_device, p_device, broker_pid);
		ERR_FAIL_COND_V_MSG(init_err != OK, init_err, "DSH window sink: shared frame factory initialization failed.");
	}

	// Never recycle a ring the broker still holds leases on.
	wait_all_presents_complete();
	{
		const uint64_t deadline = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() + 10ULL * 1000 * 1000;
		std::unique_lock<std::mutex> lock(sink_mutex);
		for (;;) {
			bool all_free = true;
			const uint32_t count = DSHSharedFramesWin32::get_slot_count(pool);
			for (uint32_t i = 0; i < count && i < MAX_SLOTS; i++) {
				if (!slot_free[i].load()) {
					all_free = false;
					break;
				}
			}
			if (all_free) {
				break;
			}
			if (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() >= deadline) {
				ERR_FAIL_V_MSG(ERR_TIMEOUT, "DSH window sink: frame lease timeout while resizing a sub-window.");
			}
			sink_cv.wait_for(lock, std::chrono::milliseconds(20));
		}
	}

	vk_device = p_device;
	if (completion_semaphore == VK_NULL_HANDLE) {
		VkSemaphoreTypeCreateInfo type_info{};
		type_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		type_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_info.pNext = &type_info;
		VkResult err = vkCreateSemaphore(p_device, &semaphore_info, nullptr, &completion_semaphore);
		ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, ERR_CANT_CREATE, "DSH window sink: couldn't create the completion semaphore.");
	}

	Error err = factory->create_frames(pool, p_width, p_height, p_slot_count);
	ERR_FAIL_COND_V_MSG(err != OK, err, "DSH window sink: couldn't create the D3D11 shared frame pool.");

	{
		std::lock_guard<std::mutex> lock(sink_mutex);
		const uint32_t slot_count = DSHSharedFramesWin32::get_slot_count(pool);
		for (uint32_t i = 0; i < MAX_SLOTS; i++) {
			slot_free[i].store(i < slot_count);
		}
		pending_presents.clear();
		epoch++;
		sink_cv.notify_all();
	}
	return OK;
}

void DSHWindowSink::destroy_frames() {
	{
		std::lock_guard<std::mutex> lock(sink_mutex);
		pending_presents.clear();
		for (uint32_t i = 0; i < MAX_SLOTS; i++) {
			slot_free[i].store(true);
		}
		sink_cv.notify_all();
	}
	if (factory != nullptr) {
		factory->destroy_frames(pool);
	}
}

bool DSHWindowSink::has_frames() const {
	return DSHSharedFramesWin32::has_frames(pool);
}

uint32_t DSHWindowSink::get_slot_count() const {
	return DSHSharedFramesWin32::get_slot_count(pool);
}

VkImage DSHWindowSink::get_slot_image(uint32_t p_slot) const {
	return DSHSharedFramesWin32::get_slot_image(pool, p_slot);
}

uint64_t DSHWindowSink::get_slot_nt_handle(uint32_t p_slot) const {
	return DSHSharedFramesWin32::get_slot_nt_handle(pool, p_slot);
}

uint64_t DSHWindowSink::allocate_present(uint32_t p_slot) {
	std::lock_guard<std::mutex> lock(sink_mutex);
	const uint64_t value = ++next_present_value;
	pending_presents.push_back(std::make_pair(value, p_slot));
	return value;
}

void DSHWindowSink::cancel_present(uint64_t p_value) {
	std::lock_guard<std::mutex> lock(sink_mutex);
	for (auto it = pending_presents.begin(); it != pending_presents.end(); ++it) {
		if (it->first == p_value) {
			pending_presents.erase(it);
			break;
		}
	}
	sink_cv.notify_all();
}

uint32_t DSHWindowSink::acquire_slot_blocking() {
	const uint64_t deadline = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() + 10ULL * 1000 * 1000;
	std::unique_lock<std::mutex> lock(sink_mutex);
	for (;;) {
		const uint32_t slot_count = DSHSharedFramesWin32::get_slot_count(pool);
		for (uint32_t i = 0; i < slot_count && i < MAX_SLOTS; i++) {
			if (slot_free[i].load()) {
				slot_free[i].store(false);
				return i;
			}
		}
		const uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		if (now >= deadline) {
			return UINT32_MAX;
		}
		sink_cv.wait_for(lock, std::chrono::milliseconds(20));
	}
}

void DSHWindowSink::mark_slot_released(uint32_t p_slot, uint64_t p_frame_id) {
	(void)p_frame_id;
	std::lock_guard<std::mutex> lock(sink_mutex);
	if (p_slot < MAX_SLOTS) {
		slot_free[p_slot].store(true);
	}
	sink_cv.notify_all();
}

bool DSHWindowSink::are_all_slots_free() const {
	if (!DSHSharedFramesWin32::has_frames(pool)) {
		return true;
	}
	std::lock_guard<std::mutex> lock(sink_mutex);
	if (!pending_presents.empty()) {
		return false;
	}
	const uint32_t slot_count = DSHSharedFramesWin32::get_slot_count(pool);
	for (uint32_t i = 0; i < slot_count && i < MAX_SLOTS; i++) {
		if (!slot_free[i].load()) {
			return false;
		}
	}
	return true;
}

void DSHWindowSink::wait_all_presents_complete() {
	// Mirror of the main ring contract: the pump thread pops each pending present
	// once the GPU signals the timeline; we just wait for the queue to drain.
	std::unique_lock<std::mutex> lock(sink_mutex);
	while (!pending_presents.empty()) {
		sink_cv.wait_for(lock, std::chrono::milliseconds(50));
	}
}

bool DSHWindowSink::pop_completed(uint32_t &r_slot) {
	std::unique_lock<std::mutex> lock(sink_mutex);
	if (pending_presents.empty() || vk_device == VK_NULL_HANDLE || completion_semaphore == VK_NULL_HANDLE) {
		return false;
	}
	const uint64_t value = pending_presents.front().first;
	lock.unlock();
	VkSemaphoreWaitInfo wait_info{};
	wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
	wait_info.semaphoreCount = 1;
	wait_info.pSemaphores = &completion_semaphore;
	wait_info.pValues = &value;
	VkResult err = vkWaitSemaphores(vk_device, &wait_info, 0);
	if (err != VK_SUCCESS) {
		return false;
	}
	lock.lock();
	if (pending_presents.empty() || pending_presents.front().first != value) {
		return false; // A concurrent destroy_frames() won the race.
	}
	r_slot = pending_presents.front().second;
	pending_presents.pop_front();
	sink_cv.notify_all();
	return true;
}

bool DSHWindowSink::has_pending_presents() const {
	std::lock_guard<std::mutex> lock(sink_mutex);
	return !pending_presents.empty();
}

#endif // WINDOWS_ENABLED && DSH_VIEWPORT_ENABLED
