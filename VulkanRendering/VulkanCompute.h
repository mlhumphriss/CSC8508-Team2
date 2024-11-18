/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanShaderBase.h"

namespace NCL::Rendering::Vulkan {
	/*
	VulkanCompute: Represents a single computer shader object
	*/
	class VulkanCompute : public VulkanShaderBase	{
	public:
		VulkanCompute(vk::Device sourceDevice, const std::string& filename);
		~VulkanCompute() {}

		Maths::Vector3i GetThreadCount() const { return localThreadSize; }

		void	FillShaderStageCreateInfo(vk::ComputePipelineCreateInfo& info) const;

	protected:
		Maths::Vector3i localThreadSize;
		vk::PipelineShaderStageCreateInfo info;
		vk::UniqueShaderModule	computeModule;
	};
}