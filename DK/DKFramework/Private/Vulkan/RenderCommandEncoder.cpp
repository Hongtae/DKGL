//
//  File: RenderCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "RenderCommandEncoder.h"
#include "Buffer.h"
#include "Texture.h"
#include "GraphicsDevice.h"
#include "RenderPipelineState.h"
#include "ShaderBindingSet.h"

#define FLIP_VIEWPORT_Y	1

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

RenderCommandEncoder::Resources::Resources(class CommandBuffer* b)
	: cb(b)
	, framebuffer(VK_NULL_HANDLE)
	, renderPass(VK_NULL_HANDLE)
	, commandBuffer(VK_NULL_HANDLE)
{
}

RenderCommandEncoder::Resources::~Resources()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(cb->Queue()->Device());
	VkDevice device = dev->device;

	if (renderPass)
		vkDestroyRenderPass(device, renderPass, dev->allocationCallbacks);

	if (framebuffer)
		vkDestroyFramebuffer(device, framebuffer, dev->allocationCallbacks);

	if (commandBuffer)
		cb->ReleaseEncodingBuffer(commandBuffer);
}

RenderCommandEncoder::RenderCommandEncoder(VkCommandBuffer vcb, class CommandBuffer* cb, const DKRenderPassDescriptor& desc)
	: commandBuffer(cb)
{
	resources = DKOBJECT_NEW Resources(cb);
	resources->commandBuffer = vcb;
	DKASSERT_DEBUG(resources->commandBuffer);

	uint32_t frameWidth = 0;
	uint32_t frameHeight = 0;

	DKArray<VkAttachmentDescription> attachments;
	attachments.Reserve(desc.colorAttachments.Count() + 1);

	DKArray<VkAttachmentReference> colorReferences;
	colorReferences.Reserve(desc.colorAttachments.Count());

	DKArray<VkImageView> framebufferImageViews;
	framebufferImageViews.Reserve(desc.colorAttachments.Count() + 1);

	DKArray<VkClearValue> attachmentClearValues;
	attachmentClearValues.Reserve(desc.colorAttachments.Count() + 1);

	for (const DKRenderPassColorAttachmentDescriptor& colorAttachment : desc.colorAttachments)
	{
		const Texture* rt = colorAttachment.renderTarget.SafeCast<Texture>();
		if (rt)
		{
			AddWaitSemaphore(rt->waitSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			AddSignalSemaphore(rt->signalSemaphore);

			VkAttachmentDescription attachment = {};
			attachment.format = rt->format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT; // 1 sample per pixel
			switch (colorAttachment.loadAction)
			{
			case DKRenderPassAttachmentDescriptor::LoadActionLoad:
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			case DKRenderPassAttachmentDescriptor::LoadActionClear:
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			default:
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
			}
			switch (colorAttachment.storeAction)
			{
			case DKRenderPassAttachmentDescriptor::StoreActionDontCare:
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				break;
			case DKRenderPassAttachmentDescriptor::StoreActionStore:
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				break;
			}
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
            if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
                attachment.initialLayout = rt->ChangeLayerLayout(0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, nullptr);
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference attachmentReference = {};
			attachmentReference.attachment = static_cast<uint32_t>(attachments.Count());
			attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachments.Add(attachment);
			colorReferences.Add(attachmentReference);

			framebufferImageViews.Add(rt->imageView);

			VkClearValue clearValue = {};
			clearValue.color = { colorAttachment.clearColor.r, colorAttachment.clearColor.g, colorAttachment.clearColor.b, colorAttachment.clearColor.a };
			attachmentClearValues.Add(clearValue);

			frameWidth = (frameWidth > 0) ? Min(frameWidth, rt->Width()) : rt->Width();
			frameHeight = (frameHeight > 0) ? Min(frameHeight, rt->Height()) : rt->Height();
		}
	}

	VkAttachmentReference depthStencilReference = {};
	depthStencilReference.attachment = VK_ATTACHMENT_UNUSED;
	depthStencilReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	if (desc.depthStencilAttachment.renderTarget)
	{
		const DKRenderPassDepthStencilAttachmentDescriptor& depthStencilAttachment = desc.depthStencilAttachment;
		const Texture* rt = depthStencilAttachment.renderTarget.SafeCast<Texture>();
		if (rt)
		{
			AddWaitSemaphore(rt->waitSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			AddSignalSemaphore(rt->signalSemaphore);

			VkAttachmentDescription attachment = {};
			attachment.format = rt->format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			switch (depthStencilAttachment.loadAction)
			{
			case DKRenderPassAttachmentDescriptor::LoadActionLoad:
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			case DKRenderPassAttachmentDescriptor::LoadActionClear:
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			default:
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
			}
			switch (depthStencilAttachment.storeAction)
			{
			case DKRenderPassAttachmentDescriptor::StoreActionDontCare:
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				break;
			case DKRenderPassAttachmentDescriptor::StoreActionStore:
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				break;
			}
			attachment.stencilLoadOp = attachment.loadOp;
			attachment.stencilStoreOp = attachment.storeOp;
            attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if (attachment.loadOp == VK_ATTACHMENT_LOAD_OP_LOAD)
                rt->ChangeLayerLayout(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, nullptr);
			attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			depthStencilReference.attachment = static_cast<uint32_t>(attachments.Count());
			attachments.Add(attachment);

			framebufferImageViews.Add(rt->imageView);

			VkClearValue clearValue = {};
			clearValue.depthStencil.depth = depthStencilAttachment.clearDepth;
			clearValue.depthStencil.stencil = depthStencilAttachment.clearStencil;
			attachmentClearValues.Add(clearValue);

			frameWidth = (frameWidth > 0) ? Min(frameWidth, rt->Width()) : rt->Width();
			frameHeight = (frameHeight > 0) ? Min(frameHeight, rt->Height()) : rt->Height();
		}
	}

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorReferences.Count());
	subpassDescription.pColorAttachments = colorReferences;
	subpassDescription.pDepthStencilAttachment = &depthStencilReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;


	VkRenderPassCreateInfo  renderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.Count());
	renderPassCreateInfo.pAttachments = attachments;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;


	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(commandBuffer->Queue()->Device());
	VkDevice device = dev->device;

	VkResult err = vkCreateRenderPass(device, &renderPassCreateInfo, dev->allocationCallbacks, &resources->renderPass);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkCreateRenderPass failed: %s", VkResultCStr(err));
		DKASSERT(err == VK_SUCCESS);
	}

	VkFramebufferCreateInfo frameBufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	frameBufferCreateInfo.renderPass = resources->renderPass;
	frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(framebufferImageViews.Count());
	frameBufferCreateInfo.pAttachments = framebufferImageViews;
	frameBufferCreateInfo.width = frameWidth;
	frameBufferCreateInfo.height = frameHeight;
	frameBufferCreateInfo.layers = 1;
	err = vkCreateFramebuffer(device, &frameBufferCreateInfo, dev->allocationCallbacks, &resources->framebuffer);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkCreateFramebuffer failed: %s", VkResultCStr(err));
		DKASSERT(err == VK_SUCCESS);
	}

	VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	vkBeginCommandBuffer(resources->commandBuffer, &commandBufferBeginInfo);

	VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBeginInfo.renderPass = resources->renderPass;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(attachmentClearValues.Count());
	renderPassBeginInfo.pClearValues = attachmentClearValues;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = frameWidth;
	renderPassBeginInfo.renderArea.extent.height = frameHeight;
	renderPassBeginInfo.framebuffer = resources->framebuffer;
	vkCmdBeginRenderPass(resources->commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// setup viewport
	bool flipY = FLIP_VIEWPORT_Y;
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(frameWidth);
	viewport.height = static_cast<float>(frameHeight);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	if (flipY) // flip-Y-axis
	{
		viewport.y = viewport.y + viewport.height; // set origin to lower-left.
		viewport.height = -(viewport.height); // negative height.
	}

	vkCmdSetViewport(resources->commandBuffer, 0, 1, &viewport);
	// setup scissor
	VkRect2D scissorRect = { {0, 0},{frameWidth,frameHeight} };
	vkCmdSetScissor(resources->commandBuffer, 0, 1, &scissorRect);
}

RenderCommandEncoder::~RenderCommandEncoder()
{
	resources = NULL;
}

void RenderCommandEncoder::AddWaitSemaphore(VkSemaphore semaphore, VkPipelineStageFlags flags)
{
	if (semaphore != VK_NULL_HANDLE)
	{
		if (!semaphorePipelineStageMasks.Insert(semaphore, flags))
			semaphorePipelineStageMasks.Value(semaphore) |= flags;
	}
}

void RenderCommandEncoder::AddSignalSemaphore(VkSemaphore semaphore)
{
	if (semaphore != VK_NULL_HANDLE)
		signalSemaphores.Insert(semaphore);
}

void RenderCommandEncoder::EndEncoding()
{
	vkCmdEndRenderPass(resources->commandBuffer);

	VkResult err = vkEndCommandBuffer(resources->commandBuffer);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkEndCommandBuffer failed: %s", VkResultCStr(err));
		DKASSERT(err == VK_SUCCESS);
	}

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &resources->commandBuffer;

	resources->waitSemaphores.Reserve(semaphorePipelineStageMasks.Count());
	resources->waitStageMasks.Reserve(semaphorePipelineStageMasks.Count());
		
	semaphorePipelineStageMasks.EnumerateForward([&](decltype(semaphorePipelineStageMasks)::Pair& pair)
	{
		resources->waitSemaphores.Add(pair.key);
		resources->waitStageMasks.Add(pair.value);
	});

	resources->signalSemaphores.Reserve(signalSemaphores.Count());
	signalSemaphores.EnumerateForward([&](VkSemaphore semaphore)
	{
		resources->signalSemaphores.Add(semaphore);
	});

	DKASSERT_DEBUG(resources->waitSemaphores.Count() == resources->waitStageMasks.Count());

	submitInfo.waitSemaphoreCount = resources->waitSemaphores.Count();
	submitInfo.pWaitSemaphores = resources->waitSemaphores;
	submitInfo.pWaitDstStageMask = resources->waitStageMasks;
	submitInfo.signalSemaphoreCount = resources->signalSemaphores.Count();
	submitInfo.pSignalSemaphores = resources->signalSemaphores;

    DKObject<DKOperation> submitCallback = DKFunction([](DKObject<Resources> res)
    {
        res->updateResources.EnumerateForward([](decltype(res->updateResources)::Pair& pair)
        {
            if (pair.value)
                pair.value->UpdateDescriptorSet();
        });
    })->Invocation(resources);

    DKObject<DKOperation> completedCallback = DKFunction([](DKObject<Resources> res)
	{
		res = NULL;
	})->Invocation(resources);

    commandBuffer->Submit(submitInfo, submitCallback, completedCallback);

	resources = NULL;
	semaphorePipelineStageMasks.Clear();
	signalSemaphores.Clear();
}

void RenderCommandEncoder::SetViewport(const DKViewport& v)
{
	bool flipY = FLIP_VIEWPORT_Y;
	VkViewport viewport = {v.x, v.y, v.width, v.height, v.nearZ, v.farZ};
	if (flipY) // flip-Y-axis
	{
		viewport.y = viewport.y + viewport.height; // set origin to lower-left.
		viewport.height = -(viewport.height); // negative height.
	}
	vkCmdSetViewport(resources->commandBuffer, 0, 1, &viewport);
}

void RenderCommandEncoder::SetRenderPipelineState(DKRenderPipelineState* ps)
{
	DKASSERT_DEBUG(dynamic_cast<RenderPipelineState*>(ps));
	RenderPipelineState* pipeline = static_cast<RenderPipelineState*>(ps);
	vkCmdBindPipeline(resources->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
    resources->pipelineStateObjects.Add(pipeline);
    resources->pipelineState = pipeline;

	// bind descriptor sets
    resources->unboundResources.EnumerateForward([&](decltype(resources->unboundResources)::Pair& pair)
    {
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        if (pair.value)
        {
            ShaderBindingSet* bindingSet = pair.value;
            DKASSERT_DEBUG(bindingSet);

            descriptorSet = bindingSet->descriptorSet;
            DKASSERT_DEBUG(descriptorSet != VK_NULL_HANDLE);

            vkCmdBindDescriptorSets(resources->commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline->layout,
                                    pair.key,
                                    1,
                                    &descriptorSet,
                                    0,      // dynamic offsets
                                    0);

            if (bindingSet->layoutFlags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT)
                resources->updateResources.Update(pair.key, pair.value);
        }
    });
    resources->unboundResources.Clear();
}

void RenderCommandEncoder::SetVertexBuffer(DKGpuBuffer* buffer, size_t offset, uint32_t index)
{
	DKASSERT_DEBUG(buffer);
	SetVertexBuffers(&buffer, &offset, index, 1);
}

void RenderCommandEncoder::SetVertexBuffers(DKGpuBuffer** buffers, const size_t* offsets, uint32_t index, size_t count)
{
	if (count > 1)
	{
		uint8_t* tmp = new uint8_t[(sizeof(VkBuffer) + sizeof(VkDeviceSize)) * count];

		VkBuffer* bufs = reinterpret_cast<VkBuffer*>(tmp);
		VkDeviceSize* ofs = reinterpret_cast<VkDeviceSize*>(&tmp[sizeof(VkBuffer) * count]);

		for (size_t i = 0; i < count; ++i)
		{
            DKGpuBuffer* buffer = buffers[i];
			DKASSERT_DEBUG(dynamic_cast<Buffer*>(buffer) != nullptr);
			bufs[i] = static_cast<Buffer*>(buffer)->buffer;
			ofs[i] = offsets[i];

            resources->buffers.Add(buffer);
		}
		vkCmdBindVertexBuffers(resources->commandBuffer, index, count, bufs, ofs);
		delete[] tmp;
	}
	else if (count > 0)
	{
        DKGpuBuffer* buffer = buffers[0];
        DKASSERT_DEBUG(dynamic_cast<Buffer*>(buffer) != nullptr);
		Buffer* buf = static_cast<Buffer*>(buffer);
		VkDeviceSize of = offsets[0];
        resources->buffers.Add(buffer);
		vkCmdBindVertexBuffers(resources->commandBuffer, index, count, &buf->buffer, &of);
	}
}
void RenderCommandEncoder::SetIndexBuffer(DKGpuBuffer* indexBuffer, size_t offset, DKIndexType type)
{
	DKASSERT_DEBUG(indexBuffer);
	DKASSERT_DEBUG(dynamic_cast<Buffer*>(indexBuffer) != nullptr);

	VkIndexType indexType;
	switch (type)
	{
	case DKIndexType::UInt16:	indexType = VK_INDEX_TYPE_UINT16; break;
	case DKIndexType::UInt32:	indexType = VK_INDEX_TYPE_UINT32; break;
	default:
		DKASSERT_DESC_DEBUG(0, "Unknown Index Type");
		DKLogE("ERROR: Unknown index type!");
		return;
	}
    resources->buffers.Add(indexBuffer);
	vkCmdBindIndexBuffer(resources->commandBuffer, static_cast<Buffer*>(indexBuffer)->buffer, offset, indexType);
}

void RenderCommandEncoder::Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance)
{
	if (numInstances > 0)
		vkCmdDraw(resources->commandBuffer, numVertices, numInstances, baseVertex, baseInstance);
}

void RenderCommandEncoder::DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance)
{
	if (numInstances > 0)
	{
		vkCmdDrawIndexed(resources->commandBuffer, numIndices, numInstances, indexOffset, vertexOffset, baseInstance);
	}
}

void RenderCommandEncoder::SetResources(uint32_t index, DKShaderBindingSet* set)
{
    ShaderBindingSet* bindingSet = nullptr;

    if (set)
    {
        DKASSERT_DEBUG(dynamic_cast<ShaderBindingSet*>(set) != nullptr);
        bindingSet = static_cast<ShaderBindingSet*>(set);
        bindingSet->UpdateDescriptorSet();

        // keep ownership 
        resources->shaderBindingSets.Add(bindingSet);
    }

    if (resources->pipelineState)
    {
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        if (bindingSet)
            descriptorSet = bindingSet->descriptorSet;

        vkCmdBindDescriptorSets(resources->commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                resources->pipelineState->layout,
                                index,
                                1,
                                &descriptorSet,
                                0,      // dynamic offsets
                                0);
    }
    else
    {
        resources->unboundResources.Update(index, bindingSet);
    }
}
#endif //#if DKGL_ENABLE_VULKAN
