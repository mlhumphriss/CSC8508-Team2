/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanShaderBuilder.h"
#include "VulkanUtils.h"
#include "VulkanShader.h"
#include "VulkanDescriptorSetLayoutBuilder.h"
#include "Assets.h"

using std::string;

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

const char* ErrorMessages[ShaderStages::MAX_SIZE] =
{
	"Multiple vertex shaders attached to shader object!",
	"Multiple fragment shaders attached to shader object!",
	"Multiple geometry shaders attached to shader object!",
	"Multiple TCS shaders attached to shader object!",
	"Multiple TES shaders attached to shader object!",	
	"Multiple mesh shaders attached to shader object!",
};



ShaderBuilder& ShaderBuilder::AddBinary(ShaderStages::Type stage, const std::string& name, const std::string& entry) {
	assert(MessageAssert(shaderFiles[stage].empty(), ErrorMessages[stage]));
	assert(MessageAssert(stage != (uint32_t)ShaderStages::MAX_SIZE, "Invalid shader stage!"));
	shaderFiles[stage] = name;
	entryPoints[stage] = entry;
	return *this;
}

ShaderBuilder& ShaderBuilder::WithMeshBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Mesh, name, entry);
}

ShaderBuilder& ShaderBuilder::WithVertexBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Vertex, name, entry);
}

ShaderBuilder& ShaderBuilder::WithFragmentBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Fragment, name, entry);
}

ShaderBuilder& ShaderBuilder::WithGeometryBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::Geometry, name, entry);
}

ShaderBuilder& ShaderBuilder::WithTessControlBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::TessControl, name, entry);
}

ShaderBuilder& ShaderBuilder::WithTessEvalBinary(const string& name, const std::string& entry) {
	return AddBinary(ShaderStages::TessEval, name, entry);
}

UniqueVulkanShader ShaderBuilder::Build(const std::string& debugName) {
	VulkanShader* newShader = new VulkanShader();
	//mesh and 'traditional' pipeline are mutually exclusive
	assert(MessageAssert(!(!shaderFiles[ShaderStages::Mesh].empty() && !shaderFiles[ShaderStages::Vertex].empty()),
		"Cannot use traditional vertex pipeline with mesh shaders!"));

	for (int i = 0; i < ShaderStages::MAX_SIZE; ++i) {
		if (!shaderFiles[i].empty()) {

			char* data;
			size_t dataSize = 0;
			Assets::ReadBinaryFile(Assets::SHADERDIR + "VK/" + shaderFiles[i], &data, dataSize);

			vk::UniqueShaderModule module;

			if (dataSize > 0) {
				module = sourceDevice.createShaderModuleUnique(
					{
						.flags = {},
						.codeSize = dataSize,
						.pCode = (uint32_t*)data
					}
					//vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), dataSize, (uint32_t*)data)		
				);
				newShader->AddReflectionData(dataSize, data, rasterStages[i]);
			}
			else {
				std::cout << __FUNCTION__ << " Problem loading shader file " << shaderFiles[i] << "!\n";
			}

			newShader->AddBinaryShaderModule(static_cast<ShaderStages::Type>(i), module, entryPoints[i]);

			if (!debugName.empty()) {
				SetDebugName(sourceDevice, vk::ObjectType::eShaderModule, GetVulkanHandle(*newShader->shaderModules[i]), debugName);
			}
		}
	};

	newShader->Init();
	newShader->BuildLayouts(sourceDevice);
	return UniqueVulkanShader(newShader);
}