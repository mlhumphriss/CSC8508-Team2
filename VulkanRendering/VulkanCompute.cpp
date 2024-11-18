/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanCompute.h"
#include "Assets.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

VulkanCompute::VulkanCompute(vk::Device device, const std::string& filename) : localThreadSize{ 0,0,0 } {
	char* data;
	size_t dataSize = 0;
	Assets::ReadBinaryFile(Assets::SHADERDIR + "VK/" + filename, &data, dataSize);

	if (dataSize > 0) {
		computeModule = device.createShaderModuleUnique(//vk::ShaderModuleCreateInfo(
			{
				.flags = {},
				.codeSize = dataSize,
				.pCode = (uint32_t*)data

			}	
			//vk::ShaderModuleCreateFlags(), dataSize, (uint32_t*)data)		
		);
	}
	else {
		std::cout << __FUNCTION__ << " Problem loading shader file " << filename << "!\n";
		return;
	}
	info.stage	= vk::ShaderStageFlagBits::eCompute;
	info.module = *computeModule;
	info.pName	= "main";

	AddReflectionData(dataSize, data, vk::ShaderStageFlagBits::eCompute);
	BuildLayouts(device);

	delete data;
}

void	VulkanCompute::FillShaderStageCreateInfo(vk::ComputePipelineCreateInfo& pipeInfo) const {
	pipeInfo.setStage(info);
}