/**************************************************************************/
/*  dsh_window_sink.h                                                     */
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

#include "dsh_shared_frames_win32.h"

#include "core/error/error_list.h"
#include "core/templates/local_vector.h"
#include "drivers/vulkan/dsh_viewport_frame_sink_vulkan.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>

// Per-window presentation sink for DSH multi-window (protocol v2.1). One
// instance backs one engine sub-window surface: its own D3D11/Vulkan imported
// texture ring (2 slots), its own timeline completion semaphore, its own epoch
// (bumped whenever the engine resizes the window and the ring is recreated) and
// its own lease state. The sink interface itself carries no window parameter —
// the sink pointer is the per-surface identity — so sub-windows render through
// exactly the same RenderingDevice path as the main window.
//
// Threading: create_frames()/destroy_frames() run on the render thread inside
// RenderingDeviceDriverVulkan; the worker pump thread calls pop_completed()
// (non-blocking) to emit protocol frames, and publishes the surface receipt for
// a fresh epoch through take_unpublished_epoch() before any frame of that epoch.
class DSHWindowSink : public DSHViewportFrameSinkVulkan {
	DSHSharedFramesWin32 *factory = nullptr;
	uint32_t broker_pid = 0; // For the lazy factory initialization on the first create_frames().
	DSHSharedFramesWin32::Pool pool;
	VkDevice vk_device = VK_NULL_HANDLE;
	VkSemaphore completion_semaphore = VK_NULL_HANDLE;

	mutable std::mutex sink_mutex;
	std::condition_variable sink_cv;
	std::deque<std::pair<uint64_t, uint32_t>> pending_presents; // (timeline value, slot) in allocation order.
	uint64_t next_present_value = 0;
	std::atomic<bool> slot_free[MAX_SLOTS];

	std::atomic<uint64_t> epoch{ 0 }; // Bumped on every create_frames(); lease key per protocol v2.1.
	std::atomic<uint64_t> published_epoch{ 0 }; // Last epoch announced through a surface receipt.

public:
	DSHWindowSink() {
		for (uint32_t i = 0; i < MAX_SLOTS; i++) {
			slot_free[i].store(true);
		}
	}
	~DSHWindowSink() override;

	void set_factory(DSHSharedFramesWin32 *p_factory) { factory = p_factory; }
	void set_broker_pid(uint32_t p_broker_pid) { broker_pid = p_broker_pid; }

	// The render thread must have stopped presenting before teardown; this only
	// releases pool/semaphore resources (pools first, then the factory stays owned by the worker).
	void shutdown();

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

	uint64_t get_epoch() const { return epoch.load(); }
	uint64_t get_published_epoch() const { return published_epoch.load(); }
	void mark_epoch_published(uint64_t p_epoch) { published_epoch.store(p_epoch); }
	uint32_t get_width() const { return pool.width; }
	uint32_t get_height() const { return pool.height; }

	// Worker pump thread: non-blocking; when the oldest pending present finished
	// on the GPU, pops it and returns true with r_slot set.
	bool pop_completed(uint32_t &r_slot);
	bool has_pending_presents() const;
};

#endif // WINDOWS_ENABLED && DSH_VIEWPORT_ENABLED
