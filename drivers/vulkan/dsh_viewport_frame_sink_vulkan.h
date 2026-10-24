/**************************************************************************/
/*  dsh_viewport_frame_sink_vulkan.h                                      */
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

#if defined(VULKAN_ENABLED) && defined(DSH_VIEWPORT_ENABLED)

#include "core/error/error_list.h"

#include <drivers/vulkan/godot_vulkan.h>

// Abstract presentation sink used by the DSH (DeepSeek Harness) native
// viewport worker integration. The Vulkan rendering device driver renders
// into offscreen images provided by this sink instead of presenting to a
// window-owned swapchain; the platform layer (Windows) backs the images
// with D3D11 shared textures and re-exports them to the DSH broker through
// the wire protocol, which is owned by DSHViewportWorker.
//
// Threading: all methods except where noted are called from the render
// thread (inside RenderingDeviceDriverVulkan). acquire_slot_blocking() may
// block waiting for the broker to release a slot.
class DSHViewportFrameSinkVulkan {
public:
	static constexpr uint32_t MAX_SLOTS = 4;

	virtual ~DSHViewportFrameSinkVulkan() = default;

	// Creates the shared frame pool (D3D11 textures imported into Vulkan).
	// Must be called before any other frame method, and again with a new
	// size after destroy_frames(). Only called once the broker-side leases
	// for the previous pool have all been returned.
	virtual Error create_frames(VkPhysicalDevice p_physical_device, VkDevice p_device, uint32_t p_graphics_queue_family, uint32_t p_width, uint32_t p_height, uint32_t p_slot_count) = 0;
	virtual void destroy_frames() = 0;
	virtual bool has_frames() const = 0;

	virtual uint32_t get_slot_count() const = 0;
	virtual VkImage get_slot_image(uint32_t p_slot) const = 0;
	// NT handle opened for the broker process (decimal string in protocol).
	virtual uint64_t get_slot_nt_handle(uint32_t p_slot) const = 0;

	// Timeline semaphore signaled (value monotonically increasing) every
	// time the rendering device submits work that writes to a slot.
	virtual VkSemaphore get_completion_semaphore() const = 0;

	// Records a pending present for the given slot and returns the timeline
	// value that will be signaled once the GPU work is submitted.
	virtual uint64_t allocate_present(uint32_t p_slot) = 0;
	// Cancels a previously allocated present when the submit failed.
	virtual void cancel_present(uint64_t p_value) = 0;

	// Blocks until a slot is free for rendering. Returns UINT32_MAX on
	// timeout (broker misbehavior / lease leak).
	virtual uint32_t acquire_slot_blocking() = 0;
	// Called by the protocol adapter when the broker releases a slot.
	virtual void mark_slot_released(uint32_t p_slot, uint64_t p_frame_id) = 0;
	virtual bool are_all_slots_free() const = 0;

	// Waits until every allocated present has completed on the GPU and the
	// broker has been notified. Used before destroying the frame pool.
	virtual void wait_all_presents_complete() = 0;
};

#endif // VULKAN_ENABLED && DSH_VIEWPORT_ENABLED
