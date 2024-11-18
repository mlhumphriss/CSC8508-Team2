/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
namespace NCL::Rendering::Vulkan {
	class DescriptorSetWriter {
	public:
		DescriptorSetWriter(vk::Device device, vk::DescriptorSet set) {
			this->device = device;
			this->set = set;
		}
		~DescriptorSetWriter() {
		}

		DescriptorSetWriter& WriteImage(uint32_t binding, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal) {
			Vulkan::WriteImageDescriptor(device, set, binding, view, sampler, layout);
			return *this;
		}

		DescriptorSetWriter& WriteStorageImage(uint32_t binding, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal) {
			Vulkan::WriteStorageImageDescriptor(device, set, binding, view, sampler, layout);
			return *this;
		}

		DescriptorSetWriter& WriteBuffer(uint32_t binding, vk::Buffer buffer, vk::DescriptorType type, size_t offset = 0, size_t range = VK_WHOLE_SIZE) {
			Vulkan::WriteBufferDescriptor(device, set, binding, type, buffer, offset, range);
			return *this;
		}

	protected:
		vk::Device device;
		vk::DescriptorSet set;
	};
}