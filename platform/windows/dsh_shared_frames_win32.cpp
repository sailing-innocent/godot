/**************************************************************************/
/*  dsh_shared_frames_win32.cpp                                           */
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

#include "dsh_shared_frames_win32.h"

#include "core/error/error_macros.h"
#include "core/variant/variant.h"

#ifdef _MSC_VER
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#endif

Error DSHSharedFramesWin32::initialize(VkPhysicalDevice p_physical_device, VkDevice p_device, uint32_t p_broker_pid) {
	physical_device = p_physical_device;
	device = p_device;

	VkPhysicalDeviceIDProperties id_properties{};
	id_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
	VkPhysicalDeviceProperties2 properties{};
	properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties.pNext = &id_properties;
	vkGetPhysicalDeviceProperties2(physical_device, &properties);
	ERR_FAIL_COND_V_MSG(!id_properties.deviceLUIDValid, ERR_UNAVAILABLE, "DSH viewport: Vulkan device has no valid LUID.");

	Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	ERR_FAIL_COND_V_MSG(FAILED(hr), ERR_CANT_CREATE, vformat("DSH viewport: CreateDXGIFactory1 failed (HRESULT 0x%x).", (uint32_t)hr));

	Microsoft::WRL::ComPtr<IDXGIAdapter1> matched;
	for (UINT i = 0;; i++) {
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		hr = factory->EnumAdapters1(i, &adapter);
		if (hr == DXGI_ERROR_NOT_FOUND) {
			break;
		}
		ERR_FAIL_COND_V_MSG(FAILED(hr), ERR_CANT_CREATE, vformat("DSH viewport: EnumAdapters1 failed (HRESULT 0x%x).", (uint32_t)hr));
		DXGI_ADAPTER_DESC1 desc{};
		hr = adapter->GetDesc1(&desc);
		ERR_FAIL_COND_V_MSG(FAILED(hr), ERR_CANT_CREATE, "DSH viewport: GetDesc1 failed.");
		if (memcmp(&desc.AdapterLuid, id_properties.deviceLUID, 8) == 0) {
			matched = adapter;
			break;
		}
	}
	ERR_FAIL_COND_V_MSG(!matched, ERR_UNAVAILABLE, "DSH viewport: no DXGI adapter matches the Vulkan device LUID (multi-GPU mismatch?).");

	hr = D3D11CreateDevice(matched.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &d3d_device, nullptr, nullptr);
	ERR_FAIL_COND_V_MSG(FAILED(hr), ERR_CANT_CREATE, vformat("DSH viewport: D3D11CreateDevice failed (HRESULT 0x%x).", (uint32_t)hr));

	broker_process = OpenProcess(PROCESS_DUP_HANDLE, FALSE, p_broker_pid);
	ERR_FAIL_COND_V_MSG(!broker_process, ERR_CANT_CREATE, "DSH viewport: cannot open the DSH broker process for handle duplication.");
	return OK;
}

void DSHSharedFramesWin32::finalize() {
	// Pools must be destroyed by their owners before finalize() so no broker-side
	// handle outlives the broker process handle used to pull them back.
	if (broker_process) {
		CloseHandle(broker_process);
		broker_process = nullptr;
	}
	d3d_device.Reset();
	device = VK_NULL_HANDLE;
	physical_device = VK_NULL_HANDLE;
}

void DSHSharedFramesWin32::destroy_frames(Pool &p_pool) {
	for (uint32_t i = 0; i < p_pool.images.size(); i++) {
		if (p_pool.images[i]) {
			vkDestroyImage(device, p_pool.images[i], nullptr);
			p_pool.images[i] = VK_NULL_HANDLE;
		}
		if (p_pool.memories[i]) {
			vkFreeMemory(device, p_pool.memories[i], nullptr);
			p_pool.memories[i] = VK_NULL_HANDLE;
		}
		if (p_pool.textures[i]) {
			p_pool.textures[i].Reset();
		}
		if (p_pool.nt_handles[i]) {
			// The handle currently lives in the broker process; pull it back
			// and close it so the texture can be destroyed safely.
			HANDLE local = nullptr;
			if (DuplicateHandle(broker_process, reinterpret_cast<HANDLE>(p_pool.nt_handles[i]), GetCurrentProcess(), &local, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE)) {
				CloseHandle(local);
			}
			p_pool.nt_handles[i] = 0;
		}
	}
	p_pool.images.clear();
	p_pool.memories.clear();
	p_pool.textures.clear();
	p_pool.contexts.clear();
	p_pool.nt_handles.clear();
	p_pool.width = 0;
	p_pool.height = 0;
}

Error DSHSharedFramesWin32::create_frames(Pool &r_pool, uint32_t p_width, uint32_t p_height, uint32_t p_slot_count) {
	ERR_FAIL_COND_V_MSG(device == VK_NULL_HANDLE || !d3d_device || !broker_process, ERR_UNCONFIGURED, "DSH viewport: frame pool used before initialize().");
	ERR_FAIL_COND_V_MSG(p_slot_count == 0, ERR_INVALID_PARAMETER, "DSH viewport: at least one frame slot is required.");
	ERR_FAIL_COND_V_MSG(p_width == 0 || p_height == 0, ERR_INVALID_PARAMETER, "DSH viewport: frame size must be non-zero.");
	destroy_frames(r_pool);

	// Validate that D3D11 texture import is supported for color attachments.
	VkPhysicalDeviceExternalImageFormatInfo external_info{};
	external_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
	external_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
	VkPhysicalDeviceImageFormatInfo2 format_info{};
	format_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
	format_info.pNext = &external_info;
	format_info.format = VK_FORMAT_R8G8B8A8_UNORM;
	format_info.type = VK_IMAGE_TYPE_2D;
	format_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	format_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	VkExternalImageFormatProperties external_properties{};
	external_properties.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;
	VkImageFormatProperties2 format_properties{};
	format_properties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
	format_properties.pNext = &external_properties;
	VkResult err = vkGetPhysicalDeviceImageFormatProperties2(physical_device, &format_info, &format_properties);
	ERR_FAIL_COND_V_MSG(err != VK_SUCCESS || !(external_properties.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT),
			ERR_UNAVAILABLE, "DSH viewport: D3D11 texture color attachment import is not supported by this GPU/driver.");

	PFN_vkGetMemoryWin32HandlePropertiesKHR get_handle_properties = reinterpret_cast<PFN_vkGetMemoryWin32HandlePropertiesKHR>(vkGetDeviceProcAddr(device, "vkGetMemoryWin32HandlePropertiesKHR"));
	ERR_FAIL_COND_V(!get_handle_properties, ERR_UNAVAILABLE);

	r_pool.images.resize(p_slot_count);
	r_pool.memories.resize(p_slot_count);
	r_pool.textures.resize(p_slot_count);
	r_pool.nt_handles.resize(p_slot_count);

	for (uint32_t i = 0; i < p_slot_count; i++) {
		D3D11_TEXTURE2D_DESC texture_desc{};
		texture_desc.Width = p_width;
		texture_desc.Height = p_height;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture_desc.SampleDesc.Count = 1;
		texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture_desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED;

		HRESULT hr = d3d_device->CreateTexture2D(&texture_desc, nullptr, &r_pool.textures[i]);
		ERR_FAIL_COND_V_MSG(FAILED(hr), ERR_CANT_CREATE, vformat("DSH viewport: CreateTexture2D failed (HRESULT 0x%x).", (uint32_t)hr));

		Microsoft::WRL::ComPtr<IDXGIResource1> resource;
		hr = r_pool.textures[i].As(&resource);
		ERR_FAIL_COND_V_MSG(FAILED(hr), ERR_CANT_CREATE, "DSH viewport: texture does not implement IDXGIResource1.");
		HANDLE local_handle = nullptr;
		hr = resource->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE, nullptr, &local_handle);
		ERR_FAIL_COND_V_MSG(FAILED(hr), ERR_CANT_CREATE, vformat("DSH viewport: CreateSharedHandle failed (HRESULT 0x%x).", (uint32_t)hr));

		VkExternalMemoryImageCreateInfo external_create_info{};
		external_create_info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
		external_create_info.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
		VkImageCreateInfo image_info{};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.pNext = &external_create_info;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
		image_info.extent.width = p_width;
		image_info.extent.height = p_height;
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		err = vkCreateImage(device, &image_info, nullptr, &r_pool.images[i]);
		if (err != VK_SUCCESS) {
			CloseHandle(local_handle);
			ERR_FAIL_V_MSG(ERR_CANT_CREATE, vformat("DSH viewport: vkCreateImage failed (VkResult %d).", err));
		}

		VkMemoryRequirements requirements{};
		vkGetImageMemoryRequirements(device, r_pool.images[i], &requirements);

		VkMemoryWin32HandlePropertiesKHR handle_properties{};
		handle_properties.sType = VK_STRUCTURE_TYPE_MEMORY_WIN32_HANDLE_PROPERTIES_KHR;
		err = get_handle_properties(device, VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT, local_handle, &handle_properties);
		if (err != VK_SUCCESS) {
			CloseHandle(local_handle);
			ERR_FAIL_V_MSG(ERR_CANT_CREATE, vformat("DSH viewport: vkGetMemoryWin32HandlePropertiesKHR failed (VkResult %d).", err));
		}

		uint32_t type_bits = requirements.memoryTypeBits & handle_properties.memoryTypeBits;
		if (type_bits == 0) {
			CloseHandle(local_handle);
			ERR_FAIL_V_MSG(ERR_CANT_CREATE, "DSH viewport: no compatible external memory type.");
		}
		uint32_t memory_type = 0;
		while (!(type_bits & (1u << memory_type))) {
			memory_type++;
		}

		VkMemoryDedicatedAllocateInfo dedicated_info{};
		dedicated_info.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
		dedicated_info.image = r_pool.images[i];
		VkImportMemoryWin32HandleInfoKHR import_info{};
		import_info.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR;
		import_info.pNext = &dedicated_info;
		import_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;
		import_info.handle = local_handle;
		VkMemoryAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.pNext = &import_info;
		allocate_info.allocationSize = requirements.size;
		allocate_info.memoryTypeIndex = memory_type;
		err = vkAllocateMemory(device, &allocate_info, nullptr, &r_pool.memories[i]);
		if (err == VK_SUCCESS) {
			err = vkBindImageMemory(device, r_pool.images[i], r_pool.memories[i], 0);
		}
		if (err != VK_SUCCESS) {
			CloseHandle(local_handle);
			ERR_FAIL_V_MSG(ERR_CANT_CREATE, vformat("DSH viewport: importing D3D11 texture memory failed (VkResult %d).", err));
		}

		HANDLE remote_handle = nullptr;
		if (!DuplicateHandle(GetCurrentProcess(), local_handle, broker_process, &remote_handle, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
			CloseHandle(local_handle);
			ERR_FAIL_V_MSG(ERR_CANT_CREATE, "DSH viewport: duplicating the texture handle into the broker process failed.");
		}
		CloseHandle(local_handle);
		r_pool.nt_handles[i] = reinterpret_cast<uint64_t>(remote_handle);
	}

	r_pool.width = p_width;
	r_pool.height = p_height;
	return OK;
}

Error DSHSharedFramesWin32::read_slot_rgba8(const Pool &p_pool, uint32_t p_slot, LocalVector<uint8_t> &r_pixels) {
	ERR_FAIL_COND_V_MSG(!d3d_device || p_slot >= p_pool.textures.size() || !p_pool.textures[p_slot], ERR_UNCONFIGURED, "DSH viewport: no frame to read back.");
	ERR_FAIL_COND_V_MSG(p_pool.width == 0 || p_pool.height == 0, ERR_UNCONFIGURED, "DSH viewport: frame pool is empty.");

	D3D11_TEXTURE2D_DESC staging_desc{};
	staging_desc.Width = p_pool.width;
	staging_desc.Height = p_pool.height;
	staging_desc.MipLevels = 1;
	staging_desc.ArraySize = 1;
	staging_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	staging_desc.SampleDesc.Count = 1;
	staging_desc.Usage = D3D11_USAGE_STAGING;
	staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;
	HRESULT hr = d3d_device->CreateTexture2D(&staging_desc, nullptr, &staging);
	ERR_FAIL_COND_V_MSG(FAILED(hr), ERR_CANT_CREATE, "DSH viewport: staging texture creation failed.");

	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	d3d_device->GetImmediateContext(&context);
	context->CopyResource(staging.Get(), p_pool.textures[p_slot].Get());

	D3D11_MAPPED_SUBRESOURCE mapped{};
	hr = context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
	ERR_FAIL_COND_V_MSG(FAILED(hr), ERR_CANT_CREATE, "DSH viewport: staging texture map failed.");

	r_pixels.resize(p_pool.width * p_pool.height * 4);
	const uint8_t *src = reinterpret_cast<const uint8_t *>(mapped.pData);
	for (uint32_t y = 0; y < p_pool.height; y++) {
		memcpy(r_pixels.ptr() + y * p_pool.width * 4, src + y * mapped.RowPitch, p_pool.width * 4);
	}
	context->Unmap(staging.Get(), 0);
	return OK;
}

#endif // WINDOWS_ENABLED
