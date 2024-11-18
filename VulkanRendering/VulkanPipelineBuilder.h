/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanPipelineBuilderBase.h"
#include "VulkanPipeline.h"

namespace NCL::Rendering::Vulkan {
	class VulkanRenderer;
	class VulkanShader;
	using UniqueVulkanShader = std::unique_ptr<VulkanShader>;

	struct VulkanVertexSpecification;
	/*
	PipelineBuilder: Builder class for rasterisation pipelines. 
	The builder can automatically obtain descriptor set layouts and
	push constants from the shader binary, so these don't have to be
	manually added. By default, all pipelines will have two dynamic
	states - the viewport and scissor region. The reasoning behind this
	is that a) it means we don't have to recreate pipelines if the screen
	is resized, and b) these both appear to be zero-cost on every platform
	worth thinking about. 
	*/
	class PipelineBuilder	: public PipelineBuilderBase<PipelineBuilder, vk::GraphicsPipelineCreateInfo> {
	public:
		PipelineBuilder(vk::Device device);
		~PipelineBuilder() {}

		PipelineBuilder& WithRasterState(vk::CullModeFlagBits cullMode, vk::PolygonMode polyMode = vk::PolygonMode::eFill);
		PipelineBuilder& WithRasterState(const vk::PipelineRasterizationStateCreateInfo& info);


		PipelineBuilder& WithVertexInputState(const vk::PipelineVertexInputStateCreateInfo& spec);

		PipelineBuilder& WithTessellationPatchVertexCount(uint32_t controlPointsPerPatch);

		PipelineBuilder& WithTopology(vk::PrimitiveTopology topology, bool primitiveRestart = false);

		PipelineBuilder& WithShader(const UniqueVulkanShader& shader);
		PipelineBuilder& WithShader(const VulkanShader& shader);

		PipelineBuilder& WithPass(vk::RenderPass& renderPass);

		PipelineBuilder& WithLayout(vk::PipelineLayout& layout);

		//Depth attachment that does nothing?
		PipelineBuilder& WithDepthAttachment(vk::Format depthFormat);
		//Depth attachment with standard settings
		PipelineBuilder& WithDepthAttachment(vk::Format depthFormat, vk::CompareOp op, bool testEnabled, bool writeEnable);
		//Depth attachment with user-defined settings
		PipelineBuilder& WithDepthAttachment(vk::Format depthFormat, vk::PipelineDepthStencilStateCreateInfo& info);

		PipelineBuilder& WithStencilOps(vk::StencilOpState state);
		PipelineBuilder& WithStencilOpsFront(vk::StencilOpState state);
		PipelineBuilder& WithStencilOpsBack(vk::StencilOpState state);

		//A colour attachment, no blending
		PipelineBuilder& WithColourAttachment(vk::Format f); 
		//A colour attachment, with blending
		PipelineBuilder& WithColourAttachment(vk::Format f, vk::BlendFactor srcState, vk::BlendFactor dstState);
		//A colour attachment, with user-defined state
		PipelineBuilder& WithColourAttachment(vk::Format f, vk::PipelineColorBlendAttachmentState state);

		//By default, pipelines have a dynamic viewport and scissor region.
		//If you really want to, you can disable that here.
		PipelineBuilder& WithoutDefaultDynamicState();

		PipelineBuilder& WithDynamicState(vk::DynamicState state);

		VulkanPipeline	Build(const std::string& debugName = "", vk::PipelineCache cache = {});

		vk::PipelineRenderingCreateInfoKHR& GetRenderingCreateInfo()  {
			return renderingCreate;
		}

	protected:
		vk::PipelineCacheCreateInfo					cacheCreate;
		vk::PipelineInputAssemblyStateCreateInfo	inputAsmCreate;
		vk::PipelineRasterizationStateCreateInfo	rasterCreate;
		vk::PipelineColorBlendStateCreateInfo		blendCreate;
		vk::PipelineDepthStencilStateCreateInfo		depthStencilCreate;
		vk::PipelineViewportStateCreateInfo			viewportCreate;
		vk::PipelineMultisampleStateCreateInfo		sampleCreate;
		vk::PipelineDynamicStateCreateInfo			dynamicCreate;
		vk::PipelineVertexInputStateCreateInfo		vertexCreate;
		vk::PipelineTessellationStateCreateInfo		tessellationCreate;

		vk::PipelineRenderingCreateInfoKHR			renderingCreate;

		vk::PipelineLayout							externalLayout;

		std::vector< vk::PipelineColorBlendAttachmentState>			blendAttachStates;

		std::vector<vk::DynamicState> dynamicStates;

		std::vector<vk::Format> allColourRenderingFormats;
		vk::Format depthRenderingFormat;

		bool ignoreDynamicDefaults;
	};
}