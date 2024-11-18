/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../NCLCoreClasses/Shader.h"
#include "VulkanShaderBase.h"

namespace NCL::Rendering::Vulkan {
	class DescriptorSetLayoutBuilder;
//These have both been ordered to match the ShaderStages enum for easy lookup!
	static vk::ShaderStageFlagBits rasterStages[] = {
		vk::ShaderStageFlagBits::eVertex,
		vk::ShaderStageFlagBits::eFragment,
		vk::ShaderStageFlagBits::eGeometry,
		vk::ShaderStageFlagBits::eTessellationControl,
		vk::ShaderStageFlagBits::eTessellationEvaluation,
		vk::ShaderStageFlagBits::eMeshNV
	};

	/*
	VulkanShader: This class represents a single combined rasterisation shader,
	that can comprise of multiple shader modules, one for each shader stage.
	*/
	class VulkanShader : public Shader, public VulkanShaderBase {
	public:
		friend class VulkanRenderer;
		friend class ShaderBuilder;

		void ReloadShader() override;

		void	FillShaderStageCreateInfo(vk::GraphicsPipelineCreateInfo& info) const;

		~VulkanShader();

	protected:
		void AddBinaryShaderModule(ShaderStages::Type stage, vk::UniqueShaderModule& shaderModule, const std::string& entryPoint = "main");
		void AddBinaryShaderModule(const std::string& fromFile, ShaderStages::Type stage, vk::Device device, const std::string& entryPoint = "main");

		void Init();

	protected:			
		VulkanShader();
		vk::UniqueShaderModule shaderModules[ShaderStages::MAX_SIZE];
		std::string entryPoints[ShaderStages::MAX_SIZE];

		uint32_t stageCount;
		vk::PipelineShaderStageCreateInfo infos[ShaderStages::MAX_SIZE];
	};
}