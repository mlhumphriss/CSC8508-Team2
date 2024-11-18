/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace NCL::Rendering::Vulkan {
	class DescriptorSetLayoutBuilder;
	/*
	VulkanShaderBase: Holds the functionality common to all shaders, such
	as descriptor layout reflection
	*/
	class VulkanShaderBase {
	public:
		friend class VulkanRenderer;
		friend class ShaderBuilder;

		void	FillDescriptorSetLayouts(std::vector<vk::DescriptorSetLayout>& layouts) const;
		void	FillPushConstants(std::vector<vk::PushConstantRange>& constants) const;
		vk::DescriptorSetLayout GetLayout(uint32_t) const;

		std::vector<vk::DescriptorSetLayoutBinding>  GetLayoutBinding(uint32_t index) const;

	protected:
		void AddDescriptorSetLayoutState(std::vector<std::vector<vk::DescriptorSetLayoutBinding>>& data, std::vector<vk::UniqueDescriptorSetLayout>& layouts);
		void AddPushConstantState(std::vector<vk::PushConstantRange>& data);
		void AddReflectionData(uint32_t dataSize, const void* data, vk::ShaderStageFlags stage);
		void BuildLayouts(vk::Device device);
	
	protected:			
		VulkanShaderBase() {

		}
		~VulkanShaderBase() {

		}

		std::vector<std::vector<vk::DescriptorSetLayoutBinding>> allLayoutsBindings;
		std::vector<vk::UniqueDescriptorSetLayout> allLayouts;

		std::vector<vk::PushConstantRange> pushConstants;
	};
}