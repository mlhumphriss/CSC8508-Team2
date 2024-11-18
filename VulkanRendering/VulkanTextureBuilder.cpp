/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanTextureBuilder.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"
#include "VulkanBufferBuilder.h"
#include "TextureLoader.h"

using namespace NCL;
using namespace Rendering;
using namespace Vulkan;

TextureBuilder::TextureBuilder(vk::Device inDevice, VmaAllocator inAllocator) {
    sourceDevice    = inDevice;
    sourceAllocator = inAllocator;

    generateMips    = true;

    format      = vk::Format::eR8G8B8A8Unorm;
    layout      = vk::ImageLayout::eShaderReadOnlyOptimal;
    usages      = vk::ImageUsageFlagBits::eSampled;
    aspects     = vk::ImageAspectFlagBits::eColor;
    pipeFlags   = vk::PipelineStageFlagBits2::eFragmentShader;

    layerCount      = 1;
}

TextureBuilder& TextureBuilder::WithFormat(vk::Format inFormat) {
    format = inFormat;
    return *this;
}

TextureBuilder& TextureBuilder::WithLayout(vk::ImageLayout inLayout) {
    layout = inLayout;
    return *this;
}

TextureBuilder& TextureBuilder::WithAspects(vk::ImageAspectFlags inAspects) {
    aspects = inAspects;
    return *this;
}

TextureBuilder& TextureBuilder::WithUsages(vk::ImageUsageFlags inUsages) {
    usages = inUsages;
    return *this;
}

TextureBuilder& TextureBuilder::WithPipeFlags(vk::PipelineStageFlags2 inFlags) {
    pipeFlags = inFlags;
    return *this;
}

TextureBuilder& TextureBuilder::WithDimension(uint32_t inWidth, uint32_t inHeight, uint32_t inDepth) {
	requestedSize = { inWidth, inHeight, inDepth };
    return *this;
}

TextureBuilder& TextureBuilder::WithLayerCount(uint32_t layers) {
    layerCount = layers;
    return *this;
}


TextureBuilder& TextureBuilder::WithMips(bool inMips) {
    generateMips = inMips;
    return *this;
}

TextureBuilder& TextureBuilder::WithCommandBuffer(vk::CommandBuffer inBuffer) {
    assert(MessageAssert(queue == 0 && pool == 0, "Builder is either passed a command buffer OR uses a queue and pool!"));
    cmdBuffer = inBuffer;
    return *this;
}

TextureBuilder& TextureBuilder::UsingQueue(vk::Queue inQueue) {
    assert(MessageAssert(cmdBuffer == 0, "Builder is either passed a command buffer OR uses a queue and pool!"));
    queue = inQueue;
    return *this;
}

TextureBuilder& TextureBuilder::UsingPool(vk::CommandPool inPool) {
    assert(MessageAssert(cmdBuffer == 0, "Builder is either passed a command buffer OR uses a queue and pool!"));
    pool = inPool;
    return *this;
}

UniqueVulkanTexture TextureBuilder::Build(const std::string& debugName) {
    vk::UniqueCommandBuffer	uniqueBuffer;
    vk::CommandBuffer	    usingBuffer;
    BeginTexture(debugName, uniqueBuffer, usingBuffer);

    if (generateMips) {
        usages |= vk::ImageUsageFlagBits::eTransferSrc;
        usages |= vk::ImageUsageFlagBits::eTransferDst;
    }

    UniqueVulkanTexture tex = GenerateTexture(usingBuffer, requestedSize, false, debugName);

    //ImageTransitionBarrier(usingBuffer, tex->GetImage(), vk::ImageLayout::eUndefined, layout, aspects, vk::PipelineStageFlagBits::eTopOfPipe, pipeFlags);

    TextureJob job;
    job.image = tex->GetImage();
    EndTexture(debugName, uniqueBuffer, usingBuffer, job, tex);

    return tex;
}

UniqueVulkanTexture TextureBuilder::BuildFromData(void* dataSrc, size_t byteCount, const std::string& debugName) {
    vk::UniqueCommandBuffer	uniqueBuffer;
    vk::CommandBuffer	    usingBuffer;
    BeginTexture(debugName, uniqueBuffer, usingBuffer);

    if (generateMips) {
        usages |= vk::ImageUsageFlagBits::eTransferSrc;
        usages |= vk::ImageUsageFlagBits::eTransferDst;
    }

    UniqueVulkanTexture tex = GenerateTexture(usingBuffer, requestedSize, false, debugName);

    //ImageTransitionBarrier(usingBuffer, tex->GetImage(), vk::ImageLayout::eUndefined, layout, aspects, vk::PipelineStageFlagBits::eTopOfPipe, pipeFlags);

    TextureJob job;
    job.faceCount = 1;
    job.dataSrcs[0] = (char*)dataSrc;
    job.dataOwnership[0] = false;
    job.image = tex->GetImage();
    job.endLayout = layout;
    job.aspect = vk::ImageAspectFlagBits::eColor;
    job.faceByteCount = byteCount;
    job.dimensions = requestedSize;

    UploadTextureData(usingBuffer, job);

    EndTexture(debugName, uniqueBuffer, usingBuffer, job, tex);

    return tex;
}

void TextureBuilder::BeginTexture(const std::string& debugName, vk::UniqueCommandBuffer& uniqueBuffer, vk::CommandBuffer& usingBuffer) {
    //We're appending to an external command buffer
    if (cmdBuffer) {
        usingBuffer = cmdBuffer;
    }
    //We're in charge of our own command buffers
    else if (queue && pool) {
        uniqueBuffer = Vulkan::CmdBufferCreateBegin(sourceDevice, pool, debugName + " Creation");
        usingBuffer = *uniqueBuffer;
    }
    else {
        assert(MessageAssert(true, "Cannot build without either a command buffer OR a queue and pool"));
    }
}

void TextureBuilder::EndTexture(const std::string& debugName, vk::UniqueCommandBuffer& uniqueBuffer, vk::CommandBuffer& usingBuffer, TextureJob& job, UniqueVulkanTexture& t) {
    if (generateMips) {
        int mipCount = VulkanTexture::GetMaxMips(t->GetDimensions());
        if (mipCount > 1) {
            t->mipCount = mipCount;
            t->GenerateMipMaps(usingBuffer);
        }
    }

    //If we're in charge of our own buffers, we just stop and wait for completion now
    if (uniqueBuffer) {
        CmdBufferEndSubmitWait(usingBuffer, sourceDevice, queue);
        for (int i = 0; i < job.faceCount; ++i) {
            if (job.dataOwnership[i]) {
                TextureLoader::DeleteTextureData(job.dataSrcs[i]);
            }
        }
    }
    //Otherwise, this is going to be handled external to the builder, and placed as a 'job'
    else {
        job.workFence   = sourceDevice.createFence({});
        job.jobName     = debugName;
        activeJobs.emplace_back(job);
    }
}

UniqueVulkanTexture TextureBuilder::BuildFromFile(const std::string& filename) {
    char* texData = nullptr;
    Vector3ui dimensions(0, 0, 1);
    uint32_t channels    = 0;
    int flags       = 0;
    TextureLoader::LoadTexture(filename, texData, dimensions.x, dimensions.y, channels, flags);

    vk::UniqueCommandBuffer	uniqueBuffer;
    vk::CommandBuffer	    usingBuffer;

    BeginTexture(filename, uniqueBuffer, usingBuffer);

    vk::ImageUsageFlags	realUsages = usages;

    usages |= vk::ImageUsageFlagBits::eTransferDst;

    if (generateMips) {
        usages |= vk::ImageUsageFlagBits::eTransferSrc;
    }

    UniqueVulkanTexture tex = GenerateTexture(usingBuffer, dimensions, false, filename);

    TextureJob job;
    job.faceCount = 1;
    job.dataSrcs[0] = texData;
    job.dataOwnership[0] = true;
    job.image = tex->GetImage();
    job.endLayout = layout;
    job.aspect = vk::ImageAspectFlagBits::eColor;
    job.dimensions = dimensions;
    job.faceByteCount = dimensions.x * dimensions.y * dimensions.z * channels * sizeof(char);

    UploadTextureData(usingBuffer, job);

    EndTexture(filename, uniqueBuffer, usingBuffer, job, tex);

    usages = realUsages;

    return tex;
}

UniqueVulkanTexture TextureBuilder::BuildCubemapFromFile(
    const std::string& negativeXFile, const std::string& positiveXFile,
    const std::string& negativeYFile, const std::string& positiveYFile,
    const std::string& negativeZFile, const std::string& positiveZFile,
    const std::string& debugName) {

    TextureJob job;
    job.faceCount = 6;
    job.endLayout = layout;
    job.aspect = vk::ImageAspectFlagBits::eColor;

    Vector3ui dimensions[6]{ Vector3ui(0, 0, 1) };
    uint32_t channels[6] = { 0 };
    int flags[6] = { 0 };

    const std::string* filenames[6] = {
        &negativeXFile,
        &positiveXFile,
        &negativeYFile,
        &positiveYFile,
        &negativeZFile,
        &positiveZFile
    };

    for (int i = 0; i < 6; ++i) {
        TextureLoader::LoadTexture(*filenames[i], job.dataSrcs[i], dimensions[i].x, dimensions[i].y, channels[i], flags[i]);
        job.dataOwnership[i] = true;
    }

    vk::UniqueCommandBuffer	uniqueBuffer;
    vk::CommandBuffer	    usingBuffer;
    BeginTexture(debugName, uniqueBuffer, usingBuffer);

    vk::ImageUsageFlags	realUsages = usages;

    usages |= vk::ImageUsageFlagBits::eTransferDst;

    if (generateMips) {
        usages |= vk::ImageUsageFlagBits::eTransferSrc;
    }

    uint32_t dataWidth = sizeof(char) * channels[0];


    job.faceByteCount = dimensions[0].x * dimensions[0].y * dimensions[0].z * channels[0] * sizeof(char);
    job.dimensions = dimensions[0];

    UniqueVulkanTexture tex = GenerateTexture(usingBuffer, dimensions[0], true, debugName);
    job.image = tex->GetImage();

    UploadTextureData(usingBuffer, job);
    EndTexture(debugName, uniqueBuffer, usingBuffer, job, tex);

    usages = realUsages;

    return tex;
}

UniqueVulkanTexture TextureBuilder::BuildCubemap(const std::string& debugName) {
    vk::UniqueCommandBuffer	uniqueBuffer;
    vk::CommandBuffer	    usingBuffer;
    BeginTexture(debugName, uniqueBuffer, usingBuffer);

    UniqueVulkanTexture tex = GenerateTexture(usingBuffer, requestedSize, true, debugName);

    TextureJob job;
    job.image = tex->GetImage();
    job.endLayout = layout;
    EndTexture(debugName, uniqueBuffer, usingBuffer, job, tex);

    return tex;
}

UniqueVulkanTexture	TextureBuilder::GenerateTexture(vk::CommandBuffer cmdBuffer, Vector3ui dimensions, bool isCube, const std::string& debugName) {
    VulkanTexture* t = new VulkanTexture();

    uint32_t mipCount = VulkanTexture::GetMaxMips(dimensions);

    uint32_t genLayerCount = isCube ? layerCount * 6: layerCount;

    auto createInfo = vk::ImageCreateInfo()
        .setImageType(dimensions.z > 1 ? vk::ImageType::e3D : vk::ImageType::e2D)
        .setExtent(vk::Extent3D(dimensions.x, dimensions.y, dimensions.z))
        .setFormat(format)
        .setUsage(usages)
        .setMipLevels(generateMips ? mipCount : 1)
        //.setInitialLayout(layout)
        .setArrayLayers(genLayerCount);

	if (isCube) {
		createInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
	}

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	vmaCreateImage(sourceAllocator, (VkImageCreateInfo*)&createInfo, &vmaallocInfo, (VkImage*)&t->image, &t->allocationHandle, &t->allocationInfo);

    vk::ImageViewType viewType = layerCount > 1 ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;
    if (isCube) {
        viewType = layerCount > 1 ? vk::ImageViewType::eCubeArray : vk::ImageViewType::eCube;
    }
    else if (dimensions.z > 1) {
        viewType = vk::ImageViewType::e3D;
    }

	vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
        .setViewType(viewType)
		.setFormat(format)
        .setSubresourceRange(vk::ImageSubresourceRange(aspects, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS))
		.setImage(t->image);

	t->defaultView  = sourceDevice.createImageViewUnique(viewInfo);
    t->allocator    = sourceAllocator;
    t->layerCount   = genLayerCount;
    t->aspectType   = aspects;
    t->format       = format;
    t->dimensions   = { dimensions.x, dimensions.y };

	SetDebugName(sourceDevice, vk::ObjectType::eImage    , GetVulkanHandle(t->image)       , debugName);
	SetDebugName(sourceDevice, vk::ObjectType::eImageView, GetVulkanHandle(*t->defaultView), debugName);

	ImageTransitionBarrier(cmdBuffer, t->image, vk::ImageLayout::eUndefined, layout, aspects, vk::PipelineStageFlagBits2::eTopOfPipe, pipeFlags);

    return UniqueVulkanTexture(t);
}

void TextureBuilder::UploadTextureData(vk::CommandBuffer cmdBuffer, TextureJob& job) {
    int allocationSize = job.faceByteCount * job.faceCount;

    job.stagingBuffer = BufferBuilder(sourceDevice, sourceAllocator)
        .WithBufferUsage(vk::BufferUsageFlagBits::eTransferSrc)
        .WithHostVisibility()
        .Build(allocationSize, "Staging Buffer");

    //our buffer now has memory! Copy some texture date to it...
    char* gpuPtr = (char*)job.stagingBuffer.Map();
    for (int i = 0; i < job.faceCount; ++i) {
        memcpy(gpuPtr, job.dataSrcs[i], job.faceByteCount);
        gpuPtr += job.faceByteCount;
    }
    job.stagingBuffer.Unmap();

    Vulkan::UploadTextureData(cmdBuffer, job.stagingBuffer.buffer, job.image, vk::ImageLayout::eUndefined, job.endLayout,
        vk::BufferImageCopy{
            .imageSubresource = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .mipLevel = 0,
                .layerCount = job.faceCount
            },
            .imageExtent{job.dimensions.x, job.dimensions.y, job.dimensions.z},          
        }
    );
}

bool	TextureBuilder::IsProcessing() const {
    return !activeJobs.empty();
}

bool	TextureBuilder::IsProcessing(const std::string& debugName) const {
    if (activeJobs.empty()) {
        return false;
    }
    return false;
}

void	TextureBuilder::WaitForProcessing() {
    for (const auto& i : activeJobs) {
        if (sourceDevice.waitForFences(1, &i.workFence, true, UINT64_MAX) != vk::Result::eSuccess) {
        };
        sourceDevice.destroyFence(i.workFence);

        for (int j = 0; j < 6; ++j) {
            TextureLoader::DeleteTextureData(i.dataSrcs[j]);
        }
    }
    activeJobs.clear();
}