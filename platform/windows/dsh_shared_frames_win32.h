/**************************************************************************/
/*  dsh_shared_frames_win32.h                                             */
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

#ifdef WINDOWS_ENABLED

#include "core/error/error_list.h"
#include "core/templates/local_vector.h"

#include <drivers/vulkan/godot_vulkan.h>

#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

// D3D11 <-> Vulkan shared frame pool for the DSH native viewport worker.
//
// Creates a fixed set of D3D11 RGBA8 textures (SHARED | SHARED_NTHANDLE),
// imports them into Vulkan as color-attachment images and opens one NT
// handle per texture targeted at the DSH broker process. Vulkan renders
// directly into the imported images (zero-copy; no readbacks), and the
// broker composites them through Electron sharedTexture.
//
// Ported from the validated graphics101 SharedSurface interop code.
class DSHSharedFramesWin32 {
	VkDevice device = VK_NULL_HANDLE;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	HANDLE broker_process = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Device> d3d_device;
	LocalVector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> textures;
	LocalVector<Microsoft::WRL::ComPtr<ID3D11DeviceContext>> d3d_contexts; // Unused placeholders kept for debugging readback extensions.
	LocalVector<VkImage> images;
	LocalVector<VkDeviceMemory> memories;
	LocalVector<uint64_t> nt_handles;
	uint32_t width = 0;
	uint32_t height = 0;

public:
	Error initialize(VkPhysicalDevice p_physical_device, VkDevice p_device, uint32_t p_broker_pid);
	void finalize(); // Destroys frames + closes the broker process handle.

	Error create_frames(uint32_t p_width, uint32_t p_height, uint32_t p_slot_count);
	void destroy_frames();

	bool has_frames() const { return !images.is_empty(); }
	uint32_t get_slot_count() const { return images.size(); }
	VkImage get_slot_image(uint32_t p_slot) const { return images[p_slot]; }
	uint64_t get_slot_nt_handle(uint32_t p_slot) const { return nt_handles[p_slot]; }

	uint32_t get_width() const { return width; }
	uint32_t get_height() const { return height; }

	// Debugging/verification helper: synchronously copies the slot content
	// to a CPU buffer (RGBA8, top-down) through a D3D11 staging texture.
	Error read_slot_rgba8(uint32_t p_slot, LocalVector<uint8_t> &r_pixels);
};

#endif // WINDOWS_ENABLED
