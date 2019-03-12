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

RenderCommandEncoder::Encoder::Encoder(class CommandBuffer* cb, const DKRenderPassDescriptor& desc)
    : commandBuffer(cb)
    , renderPassDescriptor(desc)
    , framebuffer(VK_NULL_HANDLE)
    , renderPass(VK_NULL_HANDLE)
{
}

RenderCommandEncoder::Encoder::~Encoder()
{
    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(commandBuffer->Queue()->Device());
    VkDevice device = dev->device;

    if (renderPass)
        vkDestroyRenderPass(device, renderPass, dev->allocationCallbacks);

    if (framebuffer)
        vkDestroyFramebuffer(device, framebuffer, dev->allocationCallbacks);
}

bool RenderCommandEncoder::Encoder::Encode(VkCommandBuffer commandBuffer)
{
    // initialize render pass
    uint32_t frameWidth = 0;
    uint32_t frameHeight = 0;

    DKArray<VkAttachmentDescription> attachments;
    attachments.Reserve(renderPassDescriptor.colorAttachments.Count() + 1);

    DKArray<VkAttachmentReference> colorReferences;
    colorReferences.Reserve(renderPassDescriptor.colorAttachments.Count());

    DKArray<VkImageView> framebufferImageViews;
    framebufferImageViews.Reserve(renderPassDescriptor.colorAttachments.Count() + 1);

    DKArray<VkClearValue> attachmentClearValues;
    attachmentClearValues.Reserve(renderPassDescriptor.colorAttachments.Count() + 1);

    for (const DKRenderPassColorAttachmentDescriptor& colorAttachment : renderPassDescriptor.colorAttachments)
    {
        const Texture* rt = colorAttachment.renderTarget.SafeCast<Texture>();
        if (rt)
        {
            this->commandBuffer->AddWaitSemaphore(rt->waitSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            this->commandBuffer->AddSignalSemaphore(rt->signalSemaphore);

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
                attachment.initialLayout = rt->SetLayerLayout(0, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, nullptr);
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

    if (renderPassDescriptor.depthStencilAttachment.renderTarget)
    {
        const DKRenderPassDepthStencilAttachmentDescriptor& depthStencilAttachment = renderPassDescriptor.depthStencilAttachment;
        const Texture* rt = depthStencilAttachment.renderTarget.SafeCast<Texture>();
        if (rt)
        {
            this->commandBuffer->AddWaitSemaphore(rt->waitSemaphore, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            this->commandBuffer->AddSignalSemaphore(rt->signalSemaphore);

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
                rt->SetLayerLayout(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, nullptr);
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


    GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(this->commandBuffer->Queue()->Device());
    VkDevice device = dev->device;

    VkResult err = vkCreateRenderPass(device, &renderPassCreateInfo, dev->allocationCallbacks, &this->renderPass);
    if (err != VK_SUCCESS)
    {
        DKLogE("ERROR: vkCreateRenderPass failed: %s", VkResultCStr(err));
        return false;
    }

    VkFramebufferCreateInfo frameBufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    frameBufferCreateInfo.renderPass = this->renderPass;
    frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(framebufferImageViews.Count());
    frameBufferCreateInfo.pAttachments = framebufferImageViews;
    frameBufferCreateInfo.width = frameWidth;
    frameBufferCreateInfo.height = frameHeight;
    frameBufferCreateInfo.layers = 1;
    err = vkCreateFramebuffer(device, &frameBufferCreateInfo, dev->allocationCallbacks, &this->framebuffer);
    if (err != VK_SUCCESS)
    {
        DKLogE("ERROR: vkCreateFramebuffer failed: %s", VkResultCStr(err));
        return false;
    }

    // begin render pass
    VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    renderPassBeginInfo.renderPass = this->renderPass;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(attachmentClearValues.Count());
    renderPassBeginInfo.pClearValues = attachmentClearValues;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = frameWidth;
    renderPassBeginInfo.renderArea.extent.height = frameHeight;
    renderPassBeginInfo.framebuffer = this->framebuffer;
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

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

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    // setup scissor
    VkRect2D scissorRect = { {0, 0},{frameWidth,frameHeight} };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);

    // recording commands
    EncodingState state = {};
    for (EncoderCommand* c : commands)
    {
        c->Invoke(commandBuffer, state);
    }
    // end render pass
    vkCmdEndRenderPass(commandBuffer);
    return true;
}

RenderCommandEncoder::RenderCommandEncoder(class CommandBuffer* cb, const DKRenderPassDescriptor& desc)
    : commandBuffer(cb)
{
    encoder = DKOBJECT_NEW Encoder(cb, desc);
}

void RenderCommandEncoder::EndEncoding()
{
    commandBuffer->EndEncoder(this, encoder);
    encoder = nullptr;
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
    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    });
    encoder->commands.Add(command);
}

void RenderCommandEncoder::SetRenderPipelineState(DKRenderPipelineState* ps)
{
	DKASSERT_DEBUG(dynamic_cast<RenderPipelineState*>(ps));
	RenderPipelineState* pipeline = static_cast<RenderPipelineState*>(ps);

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer buffer, EncodingState& state) mutable
    {
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
        state.pipelineState = pipeline;
    });
    encoder->commands.Add(command);
    encoder->pipelineStateObjects.Add(pipeline);
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
        DKArray<VkBuffer> bufferObjects;
        DKArray<VkDeviceSize> bufferOffsets;

        bufferObjects.Reserve(count);
        bufferOffsets.Reserve(count);

        for (size_t i = 0; i < count; ++i)
        {
            DKGpuBuffer* buffer = buffers[i];
            DKASSERT_DEBUG(dynamic_cast<Buffer*>(buffer) != nullptr);
            bufferObjects.Add(static_cast<Buffer*>(buffer)->buffer);
            bufferOffsets.Add(offsets[i]);

            encoder->buffers.Add(buffer);
        }
        DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
        {
            size_t numBuffers = bufferObjects.Count();
            size_t numOffsets = bufferOffsets.Count();
            DKASSERT_DEBUG(numBuffers == count);
            DKASSERT_DEBUG(numBuffers == count);

            vkCmdBindVertexBuffers(commandBuffer, index, count, (VkBuffer*)bufferObjects, (VkDeviceSize*)bufferOffsets);
        });
        encoder->commands.Add(command);
    }
	else if (count > 0)
	{
        DKGpuBuffer* buffer = buffers[0];
        DKASSERT_DEBUG(dynamic_cast<Buffer*>(buffer) != nullptr);
        Buffer* buf = static_cast<Buffer*>(buffer);
        VkDeviceSize of = offsets[0];
        DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
        {
            vkCmdBindVertexBuffers(commandBuffer, index, count, &buf->buffer, &of);
        });
        encoder->commands.Add(command);
        encoder->buffers.Add(buffer);
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
    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& es) mutable
    {
        vkCmdBindIndexBuffer(commandBuffer, static_cast<Buffer*>(indexBuffer)->buffer, offset, indexType);
    });
    encoder->commands.Add(command);
    encoder->buffers.Add(indexBuffer);
}

void RenderCommandEncoder::Draw(uint32_t numVertices, uint32_t numInstances, uint32_t baseVertex, uint32_t baseInstance)
{
    if (numInstances > 0)
    {
        DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
        {
            vkCmdDraw(commandBuffer, numVertices, numInstances, baseVertex, baseInstance);
        });
        encoder->commands.Add(command);
    }
}

void RenderCommandEncoder::DrawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t indexOffset, int32_t vertexOffset, uint32_t baseInstance)
{
	if (numInstances > 0)
	{
        DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
        {
            vkCmdDrawIndexed(commandBuffer, numIndices, numInstances, indexOffset, vertexOffset, baseInstance);
        });
        encoder->commands.Add(command);
	}
}

void RenderCommandEncoder::SetResources(uint32_t index, DKShaderBindingSet* set)
{
    ShaderBindingSet* bindingSet = nullptr;

    if (set)
    {
        DKASSERT_DEBUG(dynamic_cast<ShaderBindingSet*>(set) != nullptr);
        bindingSet = static_cast<ShaderBindingSet*>(set);

        // keep ownership 
        encoder->shaderBindingSets.Add(bindingSet);
    }

    DKObject<EncoderCommand> command = DKFunction([=](VkCommandBuffer commandBuffer, EncodingState& state) mutable
    {
        if (state.pipelineState)
        {
            VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
            if (bindingSet)
            {
                bindingSet->UpdateDescriptorSet();
                descriptorSet = bindingSet->descriptorSet;
            }

            vkCmdBindDescriptorSets(commandBuffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    state.pipelineState->layout,
                                    index,
                                    1,
                                    &descriptorSet,
                                    0,      // dynamic offsets
                                    0);
        }
    });
    encoder->commands.Add(command);
}

#endif //#if DKGL_ENABLE_VULKAN
