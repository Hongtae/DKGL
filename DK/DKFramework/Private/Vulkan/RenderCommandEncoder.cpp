//
//  File: RenderCommandEncoder.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_USE_VULKAN

#include "RenderCommandEncoder.h"
#include "RenderTarget.h"
#include "GraphicsDevice.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

RenderCommandEncoder::Resources::Resources(CommandBuffer* b)
	: cb(b)
	, framebuffer(VK_NULL_HANDLE)
	, renderPass(VK_NULL_HANDLE)
	, commandBuffer(VK_NULL_HANDLE)
{
}

RenderCommandEncoder::Resources::~Resources(void)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(cb->Queue()->Device());
	VkDevice device = dev->device;

	if (renderPass)
		vkDestroyRenderPass(device, renderPass, nullptr);

	if (framebuffer)
		vkDestroyFramebuffer(device, framebuffer, nullptr);

	if (commandBuffer)
		cb->ReleaseEncodingBuffer(commandBuffer);
}

RenderCommandEncoder::RenderCommandEncoder(VkCommandBuffer vcb, CommandBuffer* cb, const DKRenderPassDescriptor& desc)
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
		const RenderTarget* rt = colorAttachment.renderTarget.SafeCast<RenderTarget>();
		if (rt)
		{
			AddWaitSemaphore(rt->waitSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			AddSignalSemaphore(rt->signalSemaphore);

			VkAttachmentDescription attachment = {};
			attachment.format = rt->format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
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
		const RenderTarget* rt = depthStencilAttachment.renderTarget.SafeCast<RenderTarget>();
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

	VkResult err = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &resources->renderPass);
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
	err = vkCreateFramebuffer(device, &frameBufferCreateInfo, nullptr, &resources->framebuffer);
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

	VkViewport viewport = {};
	viewport.height = (float)frameHeight;
	viewport.width = (float)frameWidth;
	viewport.minDepth = (float) 0.0f;
	viewport.maxDepth = (float) 1.0f;
	vkCmdSetViewport(resources->commandBuffer, 0, 1, &viewport);
}

RenderCommandEncoder::~RenderCommandEncoder(void)
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

void RenderCommandEncoder::EndEncoding(void)
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

	commandBuffer->Submit(submitInfo, DKFunction([=](DKObject<Resources> res)
	{
		res = NULL;
	})->Invocation(resources));

	resources = NULL;
	semaphorePipelineStageMasks.Clear();
	signalSemaphores.Clear();
}

DKCommandBuffer* RenderCommandEncoder::Buffer(void)
{
	return commandBuffer;
}

#endif //#if DKGL_USE_VULKAN
