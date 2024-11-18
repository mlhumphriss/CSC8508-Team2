/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanPipelineBuilder.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanUtils.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

PipelineBuilder::PipelineBuilder(vk::Device device) : PipelineBuilderBase(device)	{
	ignoreDynamicDefaults = false;

	sampleCreate.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	viewportCreate.setViewportCount(1);
	viewportCreate.setScissorCount(1);

	pipelineCreate.setPViewportState(&viewportCreate);

	depthStencilCreate.setDepthCompareOp(vk::CompareOp::eAlways)
		.setDepthTestEnable(false)
		.setDepthWriteEnable(false)
		.setStencilTestEnable(false)
		.setDepthBoundsTestEnable(false);

	depthRenderingFormat		= vk::Format::eUndefined;

	rasterCreate.setCullMode(vk::CullModeFlagBits::eNone)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setLineWidth(1.0f);

	inputAsmCreate.setTopology(vk::PrimitiveTopology::eTriangleList);
}

PipelineBuilder& PipelineBuilder::WithRasterState(vk::CullModeFlagBits cullMode, vk::PolygonMode polyMode) {
	rasterCreate.setCullMode(cullMode).setPolygonMode(polyMode);
	return *this;
}

PipelineBuilder& PipelineBuilder::WithRasterState(const vk::PipelineRasterizationStateCreateInfo& info) {
	rasterCreate = info;
	return *this;
}

PipelineBuilder& PipelineBuilder::WithVertexInputState(const vk::PipelineVertexInputStateCreateInfo& spec) {
	vertexCreate = spec;
	return *this;
}

PipelineBuilder& PipelineBuilder::WithTopology(vk::PrimitiveTopology topology, bool primitiveRestart) {
	inputAsmCreate.setTopology(topology).setPrimitiveRestartEnable(primitiveRestart);
	return *this;
}

PipelineBuilder& PipelineBuilder::WithShader(const UniqueVulkanShader& shader) {
	shader->FillShaderStageCreateInfo(pipelineCreate);
	shader->FillDescriptorSetLayouts(reflectionLayouts);
	shader->FillPushConstants(allPushConstants);
	return *this;
}

PipelineBuilder& PipelineBuilder::WithShader(const VulkanShader& shader) {
	shader.FillShaderStageCreateInfo(pipelineCreate);
	shader.FillDescriptorSetLayouts(reflectionLayouts);
	shader.FillPushConstants(allPushConstants);
	return *this;
}

PipelineBuilder& PipelineBuilder::WithPass(vk::RenderPass& renderPass) {
	pipelineCreate.setRenderPass(renderPass);
	return *this;
}

PipelineBuilder& PipelineBuilder::WithLayout(vk::PipelineLayout& layout) {
	externalLayout = layout;
	return *this;
}

PipelineBuilder& PipelineBuilder::WithColourAttachment(vk::Format f) {
	allColourRenderingFormats.push_back(f);

	vk::PipelineColorBlendAttachmentState pipeBlend;
	pipeBlend.setBlendEnable(false);
	pipeBlend.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	blendAttachStates.emplace_back(pipeBlend);

	return *this;
}

PipelineBuilder& PipelineBuilder::WithColourAttachment(vk::Format f, vk::BlendFactor srcState, vk::BlendFactor dstState) {
	allColourRenderingFormats.push_back(f);

	vk::PipelineColorBlendAttachmentState pipeBlend;

	pipeBlend.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
		.setBlendEnable(true)
		.setAlphaBlendOp(vk::BlendOp::eAdd)
		.setColorBlendOp(vk::BlendOp::eAdd)

		.setSrcAlphaBlendFactor(srcState)
		.setSrcColorBlendFactor(srcState)

		.setDstAlphaBlendFactor(dstState)
		.setDstColorBlendFactor(dstState);

	blendAttachStates.emplace_back(pipeBlend);

	return *this;
}

PipelineBuilder& PipelineBuilder::WithColourAttachment(vk::Format f, vk::PipelineColorBlendAttachmentState state) {
	allColourRenderingFormats.push_back(f);
	blendAttachStates.emplace_back(state);
	return *this;
}

PipelineBuilder& PipelineBuilder::WithoutDefaultDynamicState() {
	ignoreDynamicDefaults = true;
	return *this;
}

PipelineBuilder& PipelineBuilder::WithDynamicState(vk::DynamicState state) {
	dynamicStates.push_back(state);
	return *this;
}

PipelineBuilder& PipelineBuilder::WithDepthAttachment(vk::Format depthFormat) {
	depthRenderingFormat = depthFormat;

	return *this;
}

PipelineBuilder& PipelineBuilder::WithDepthAttachment(vk::Format depthFormat, vk::CompareOp op, bool testEnabled, bool writeEnabled) {
	depthRenderingFormat = depthFormat;
	depthStencilCreate.setDepthCompareOp(op)
		.setDepthTestEnable(testEnabled)
		.setDepthWriteEnable(writeEnabled);
	return *this;
}

PipelineBuilder& PipelineBuilder::WithDepthAttachment(vk::Format depthFormat, vk::PipelineDepthStencilStateCreateInfo& info) {
	depthRenderingFormat	= depthFormat;
	depthStencilCreate		= info;
	return *this;
}

PipelineBuilder& PipelineBuilder::WithStencilOps(vk::StencilOpState state) {
	depthStencilCreate.front = state;
	return *this;
}

PipelineBuilder& PipelineBuilder::WithStencilOpsFront(vk::StencilOpState state) {
	depthStencilCreate.front = state;
	return *this;
}

PipelineBuilder& PipelineBuilder::WithStencilOpsBack(vk::StencilOpState state) {
	depthStencilCreate.back = state;
	return *this;
}

PipelineBuilder& PipelineBuilder::WithTessellationPatchVertexCount(uint32_t controlPointsPerPatch) {
	tessellationCreate.setPatchControlPoints(controlPointsPerPatch);
	pipelineCreate.setPTessellationState(&tessellationCreate);
	return *this;
}

VulkanPipeline	PipelineBuilder::Build(const std::string& debugName, vk::PipelineCache cache) {
	blendCreate.setAttachments(blendAttachStates);
	blendCreate.setBlendConstants({ 1.0f, 1.0f, 1.0f, 1.0f });

	if (!ignoreDynamicDefaults)	{
		dynamicStates.push_back(vk::DynamicState::eViewport);
		dynamicStates.push_back(vk::DynamicState::eScissor);
	}		

	dynamicCreate.setDynamicStateCount(dynamicStates.size());
	dynamicCreate.setPDynamicStates(dynamicStates.data());

	vk::Format stencilRenderingFormat = vk::Format::eUndefined; //TODO

	VulkanPipeline output;

	FinaliseDescriptorLayouts();

	pipelineCreate.setPColorBlendState(&blendCreate)
		.setPDepthStencilState(&depthStencilCreate)
		.setPDynamicState(&dynamicCreate)
		.setPInputAssemblyState(&inputAsmCreate)
		.setPMultisampleState(&sampleCreate)
		.setPRasterizationState(&rasterCreate)
		.setPVertexInputState(&vertexCreate);

	if (externalLayout) {
		pipelineCreate.setLayout(externalLayout);
	}
	else {	
		vk::PipelineLayoutCreateInfo pipeLayoutCreate = vk::PipelineLayoutCreateInfo();
		pipeLayoutCreate.setSetLayouts(allLayouts);
		pipeLayoutCreate.setPushConstantRanges(allPushConstants);

		output.layout = sourceDevice.createPipelineLayoutUnique(pipeLayoutCreate);
		pipelineCreate.setLayout(*output.layout);
		if (!debugName.empty()) {
			SetDebugName(sourceDevice, vk::ObjectType::ePipelineLayout, GetVulkanHandle(*output.layout)	 , debugName);
		}
	}

	//We must be using dynamic rendering, better set it up!
	if (!allColourRenderingFormats.empty() || depthRenderingFormat != vk::Format::eUndefined) {
		renderingCreate.depthAttachmentFormat		= depthRenderingFormat;
		renderingCreate.stencilAttachmentFormat		= stencilRenderingFormat;

		renderingCreate.colorAttachmentCount		= (uint32_t)allColourRenderingFormats.size();
		renderingCreate.pColorAttachmentFormats		= allColourRenderingFormats.data();

		pipelineCreate.pNext = &renderingCreate;
	}

	output.pipeline			= sourceDevice.createGraphicsPipelineUnique(cache, pipelineCreate).value;

	if (!debugName.empty()) {
		SetDebugName(sourceDevice, vk::ObjectType::ePipeline	  , GetVulkanHandle(*output.pipeline), debugName);
	}

	return output;
}
