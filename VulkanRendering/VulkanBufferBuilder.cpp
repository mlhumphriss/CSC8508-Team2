/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanBufferBuilder.h"
#include "VulkanBuffers.h"
#include "VulkanUtils.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

BufferBuilder::BufferBuilder(vk::Device device, VmaAllocator allocator) {
	sourceDevice	= device;
	sourceAllocator = allocator;
	vmaInfo = {};
	vmaInfo.usage		= VMA_MEMORY_USAGE_AUTO;
}

BufferBuilder::BufferBuilder(VkDevice device, VmaAllocator allocator) {
	sourceDevice = device;
	sourceAllocator = allocator;
	vmaInfo = {};
	vmaInfo.usage = VMA_MEMORY_USAGE_AUTO;
}

BufferBuilder& BufferBuilder::WithBufferUsage(vk::BufferUsageFlags flags) {
	vkInfo.usage = flags;
	return *this;
}

BufferBuilder& BufferBuilder::WithBufferUsage(VkBufferUsageFlags flags) {
	vkInfo.usage = vk::BufferUsageFlags(flags);
	return *this;
}

BufferBuilder& BufferBuilder::WithMemoryProperties(vk::MemoryPropertyFlags flags) {
	vmaInfo.requiredFlags = (VkMemoryPropertyFlags)flags;
	return *this;
}

BufferBuilder& BufferBuilder::WithMemoryProperties(VkMemoryPropertyFlags flags) {
	vmaInfo.requiredFlags = flags;
	return *this;
}

BufferBuilder& BufferBuilder::WithHostVisibility() {
	vmaInfo.requiredFlags |= (VkMemoryPropertyFlags)vk::MemoryPropertyFlagBits::eHostVisible;
	vmaInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;

	return *this;
}
/*
This can be added via WithBufferUsage just fine, but it seems very
separated from the other usages - everything else describes what
you might want to store IN the buffer, while this is an intrinsic
property OF the buffer(i.e it has an address, it doesn't store addresses)
*/
BufferBuilder& BufferBuilder::WithDeviceAddress() {
	vkInfo.usage |= vk::BufferUsageFlagBits::eShaderDeviceAddress;
	return *this;
}

BufferBuilder& BufferBuilder::WithPersistentMapping() {
	vmaInfo.requiredFlags |= (VkMemoryPropertyFlags)vk::MemoryPropertyFlagBits::eHostCoherent;

	vmaInfo.flags |= (VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);
	return *this;
}

BufferBuilder& BufferBuilder::WithUniqueAllocation() {
	vmaInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	return *this;
}

BufferBuilder& BufferBuilder::WithConcurrentSharing() {
	vkInfo.sharingMode = vk::SharingMode::eConcurrent;
	return *this;
}

VulkanBuffer BufferBuilder::Build(size_t byteSize, const std::string& debugName) {
	VulkanBuffer	outputBuffer;

	outputBuffer.size = byteSize;
	vkInfo.size = byteSize;

	outputBuffer.allocator = sourceAllocator;

	vmaCreateBuffer(sourceAllocator, (VkBufferCreateInfo*)&vkInfo, &vmaInfo, (VkBuffer*)&(outputBuffer.buffer), &outputBuffer.allocationHandle, &outputBuffer.allocationInfo);

	if (vkInfo.usage & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
		outputBuffer.deviceAddress = sourceDevice.getBufferAddress(
			{
				.buffer = outputBuffer.buffer
			}
		);
	}

	if (!debugName.empty()) {
		SetDebugName(sourceDevice, vk::ObjectType::eBuffer, GetVulkanHandle(outputBuffer.buffer), debugName);
	}

	return outputBuffer;
}