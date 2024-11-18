/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanPipeline.h"
#include "VulkanUtils.h"


namespace NCL::Rendering::Vulkan {
	class VulkanRenderer;
	class VulkanShader;

	struct VulkanVertexSpecification;

	template <class T, class P>
	class PipelineBuilderBase	{
	public:

		T& WithLayout(vk::PipelineLayout pipeLayout) {
			layout = pipeLayout;
			pipelineCreate.setLayout(pipeLayout);
			return (T&)*this;
		}

		T& WithDescriptorSetLayout(uint32_t setIndex, vk::DescriptorSetLayout layout) {
			assert(setIndex < 32);
			if (setIndex >= userLayouts.size()) {
				vk::DescriptorSetLayout nullLayout = Vulkan::GetNullDescriptor(sourceDevice);
				while (userLayouts.size() <= setIndex) {
					userLayouts.push_back(nullLayout);
				}
			}
			userLayouts[setIndex] = layout;
			return (T&)*this;
		}

		T& WithDescriptorSetLayout(uint32_t setIndex, const vk::UniqueDescriptorSetLayout& layout) {
			return WithDescriptorSetLayout(setIndex, *layout);
		}

		T& WithDescriptorBuffers() {
			pipelineCreate.flags |= vk::PipelineCreateFlagBits::eDescriptorBufferEXT;
			return (T&)*this;
		}

		P& GetCreateInfo() {
			return pipelineCreate;
		}
	protected:
		PipelineBuilderBase(vk::Device device) {
			sourceDevice = device;
		}
		~PipelineBuilderBase() {}

		void FinaliseDescriptorLayouts() {
			allLayouts.clear();
			for (int i = 0; i < reflectionLayouts.size(); ++i) {
				if (userLayouts.size() > i && userLayouts[i] != Vulkan::GetNullDescriptor(sourceDevice)) {
					allLayouts.push_back(userLayouts[i]);
				}
				else {
					allLayouts.push_back(reflectionLayouts[i]);
				}
			}
		}

	protected:
		P pipelineCreate;
		vk::PipelineLayout	layout;
		vk::Device			sourceDevice;

		std::vector< vk::DescriptorSetLayout> allLayouts;

		std::vector< vk::DescriptorSetLayout> reflectionLayouts;
		std::vector< vk::DescriptorSetLayout> userLayouts;

		std::vector< vk::PushConstantRange> allPushConstants;
	};
}