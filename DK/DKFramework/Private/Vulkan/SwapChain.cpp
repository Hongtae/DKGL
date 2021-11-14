//
//  File: SwapChain.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include "Extensions.h"
#include "SwapChain.h"
#include "GraphicsDevice.h"
#include "Image.h"
#include "ImageView.h"
#include "Semaphore.h"

using namespace DKFramework;
using namespace DKFramework::Private::Vulkan;

SwapChain::SwapChain(CommandQueue* q, DKWindow* w)
	: queue(q)
	, window(w)
	, surface(NULL)
	, swapchain(NULL)
	, enableVSync(false)
	, deviceReset(false)
	, frameReadySemaphore(VK_NULL_HANDLE)
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkDevice device = dev->device;

	window->AddEventHandler(this, DKFunction(this, &SwapChain::OnWindowEvent), nullptr, nullptr);

	VkResult err;
	// create semaphore
	VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
	err = vkCreateSemaphore(device, &semaphoreCreateInfo, dev->allocationCallbacks, &frameReadySemaphore);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkCreateSemaphore failed: %s", VkResultCStr(err));
		DKASSERT_DEBUG(0);
	}
}

SwapChain::~SwapChain()
{
	window->RemoveEventHandler(this);

	queue->WaitIdle();

	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkInstance instance = dev->instance;
	VkDevice device = dev->device;

	for (ImageView* imageView : imageViews)
	{
        imageView->image->image= VK_NULL_HANDLE;
        imageView->image = nullptr;
        imageView->waitSemaphore = VK_NULL_HANDLE;
        imageView->signalSemaphore = VK_NULL_HANDLE;
        DKASSERT_DEBUG(imageView->imageView);
	}
	imageViews.Clear();

	if (swapchain)
		vkDestroySwapchainKHR(device, swapchain, dev->allocationCallbacks);

	if (surface)
		vkDestroySurfaceKHR(instance, surface, dev->allocationCallbacks);

	vkDestroySemaphore(device, frameReadySemaphore, dev->allocationCallbacks);
}

bool SwapChain::Setup()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkInstance instance = dev->instance;
	VkPhysicalDevice physicalDevice = dev->physicalDevice;
	VkDevice device = dev->device;

	uint32_t queueFamilyIndex = queue->family->FamilyIndex();

	VkResult err = VK_SUCCESS;

	// create VkSurfaceKHR
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
	surfaceCreateInfo.window = (ANativeWindow*)window->PlatformHandle();
	err = vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, dev->allocationCallbacks, &surface);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkCreateAndroidSurfaceKHR failed: %s", VkResultCStr(err));
		return false;
	}	
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
	surfaceCreateInfo.hinstance = (HINSTANCE)GetModuleHandleW(NULL);
	surfaceCreateInfo.hwnd = (HWND)window->PlatformHandle();
	err = dev->iproc.vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, dev->allocationCallbacks, &surface);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkCreateWin32SurfaceKHR failed: %s", VkResultCStr(err));
		return false;
	}
#endif

	VkBool32 surfaceSupported = VK_FALSE;
	err = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &surfaceSupported);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkGetPhysicalDeviceSurfaceSupportKHR failed: %s", VkResultCStr(err));
		return false;
	}
	if (!surfaceSupported)
	{
		DKLogE("ERROR: VkSurfaceKHR not support with QueueFamily at index:%lu", queueFamilyIndex);
		return false;
	}
	
	// get color format, color space
	// Get list of supported surface formats
	uint32_t surfaceFormatCount;
	err = dev->iproc.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, NULL);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkGetPhysicalDeviceSurfaceFormatsKHR failed: %s", VkResultCStr(err));
		return false;
	}
	if (surfaceFormatCount == 0)
	{
		DKLogE("ERROR: vkGetPhysicalDeviceSurfaceFormatsKHR returns 0 surface format count");
		return false;
	}

	availableSurfaceFormats.Clear();
	availableSurfaceFormats.Resize(surfaceFormatCount);
	err = dev->iproc.vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, availableSurfaceFormats);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkGetPhysicalDeviceSurfaceFormatsKHR failed: %s", VkResultCStr(err));
		return false;
	}

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	if ((surfaceFormatCount == 1) && (availableSurfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		this->surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		// Always select the first available color format
		// If you need a specific format (e.g. SRGB) you'd need to
		// iterate over the list of available surface format and
		// check for it's presence
		this->surfaceFormat.format = availableSurfaceFormats[0].format;
	}
	this->surfaceFormat.colorSpace = availableSurfaceFormats[0].colorSpace;

	// create swapchain
	return this->Update();
}

bool SwapChain::Update()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkPhysicalDevice physicalDevice = dev->physicalDevice;
	VkDevice device = dev->device;

	DKRect windowContentRect = this->window->ContentRect();
	uint32_t width = static_cast<uint32_t>(floor(windowContentRect.size.width + 0.5));
	uint32_t height = static_cast<uint32_t>(floor(windowContentRect.size.height + 0.5));

	VkResult err = VK_SUCCESS;
	VkSwapchainKHR swapchainOld = this->swapchain;

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfaceCaps;
	err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed: %s", VkResultCStr(err));
		return false;
	}

	// Get available present modes
	uint32_t presentModeCount;
	err = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkGetPhysicalDeviceSurfacePresentModesKHR failed: %s", VkResultCStr(err));
		return false;
	}
	if (presentModeCount == 0)
	{
		DKLogE("ERROR: vkGetPhysicalDeviceSurfacePresentModesKHR returns 0 present mode count");
		return false;
	}

	DKArray<VkPresentModeKHR> presentModes;
	presentModes.Resize(presentModeCount);

	err = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkGetPhysicalDeviceSurfacePresentModesKHR failed: %s", VkResultCStr(err));
		return false;
	}

	VkExtent2D swapchainExtent = {};
	// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	if (surfaceCaps.currentExtent.width == (uint32_t)-1)
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = width;
		swapchainExtent.height = height;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfaceCaps.currentExtent;
		width = surfaceCaps.currentExtent.width;
		height = surfaceCaps.currentExtent.height;
	}
	
	// Select a present mode for the swapchain

	// VK_PRESENT_MODE_IMMEDIATE_KHR
	// VK_PRESENT_MODE_MAILBOX_KHR
	// VK_PRESENT_MODE_FIFO_KHR
	// VK_PRESENT_MODE_FIFO_RELAXED_KHR

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
	if (!this->enableVSync)
	{
		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
			{
				swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	// Determine the number of images
	uint32_t desiredNumberOfSwapchainImages = surfaceCaps.minImageCount + 1;
	if ((surfaceCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCaps.maxImageCount))
	{
		desiredNumberOfSwapchainImages = surfaceCaps.maxImageCount;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfaceCaps.currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchainCI = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	swapchainCI.surface = surface;
	swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
	swapchainCI.imageFormat = this->surfaceFormat.format;
	swapchainCI.imageColorSpace = this->surfaceFormat.colorSpace;
	swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
	swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainCI.imageArrayLayers = 1;
	swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCI.queueFamilyIndexCount = 0;
	swapchainCI.pQueueFamilyIndices = NULL;
	swapchainCI.presentMode = swapchainPresentMode;
	swapchainCI.oldSwapchain = swapchainOld;
	// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
	swapchainCI.clipped = VK_TRUE;
	swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	// Set additional usage flag for blitting from the swapchain images if supported
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, this->surfaceFormat.format, &formatProps);
	if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) {
		swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	err = vkCreateSwapchainKHR(device, &swapchainCI, dev->allocationCallbacks, &this->swapchain);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkCreateSwapchainKHR failed: %s", VkResultCStr(err));
		return false;
	}

	DKLogI("VkSwapchainKHR created. (%u x %u, V-sync:%d, %s)",
		   swapchainExtent.width, swapchainExtent.height,
		   this->enableVSync,
		   [](VkPresentModeKHR mode)->const char*
	{
		switch (mode)
		{
		case VK_PRESENT_MODE_IMMEDIATE_KHR:			return "VK_PRESENT_MODE_IMMEDIATE_KHR";
		case VK_PRESENT_MODE_MAILBOX_KHR:			return "VK_PRESENT_MODE_MAILBOX_KHR";
		case VK_PRESENT_MODE_FIFO_KHR:				return "VK_PRESENT_MODE_FIFO_KHR";
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:		return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
		}
		return "## UNKNOWN ##";
	}(swapchainPresentMode));

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (swapchainOld)
	{
		vkDestroySwapchainKHR(device, swapchainOld, dev->allocationCallbacks);
	}

	for (ImageView* imageView : imageViews)
	{
        imageView->image->image = VK_NULL_HANDLE;
        imageView->image = nullptr;
        imageView->waitSemaphore = VK_NULL_HANDLE;
        imageView->signalSemaphore = VK_NULL_HANDLE;
        DKASSERT_DEBUG(imageView->imageView);
	}
    imageViews.Clear();

	uint32_t swapchainImageCount = 0;
	err = vkGetSwapchainImagesKHR(device, this->swapchain, &swapchainImageCount, NULL);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkGetSwapchainImagesKHR failed: %s", VkResultCStr(err));
		return false;
	}

	// Get the swap chain images
	DKArray<VkImage> swapchainImages(VkImage(NULL), swapchainImageCount);
	err = vkGetSwapchainImagesKHR(device, this->swapchain, &swapchainImageCount, swapchainImages);
	if (err != VK_SUCCESS)
	{
		DKLogE("ERROR: vkGetSwapchainImagesKHR failed: %s", VkResultCStr(err));
		return false;
	}

	// Get the swap chain buffers containing the image and imageview
	this->imageViews.Reserve(swapchainImages.Count());
	for (VkImage image : swapchainImages)
	{
		VkImageViewCreateInfo imageViewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageViewCreateInfo.format = this->surfaceFormat.format;
        imageViewCreateInfo.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.image = image;

		VkImageView imageView = NULL;
		err = vkCreateImageView(device, &imageViewCreateInfo, dev->allocationCallbacks, &imageView);
		if (err != VK_SUCCESS)
		{
			DKLogE("ERROR: vkCreateImageView failed: %s", VkResultCStr(err));
			return false;
		}

        DKObject<Image> swapChainImage = DKOBJECT_NEW Image(queue->Device(), image);
		swapChainImage->imageType = VK_IMAGE_TYPE_2D;
		swapChainImage->format = swapchainCI.imageFormat;
		swapChainImage->extent = { swapchainExtent.width, swapchainExtent.height, 1 };
		swapChainImage->mipLevels = 1;
		swapChainImage->arrayLayers = swapchainCI.imageArrayLayers;
		swapChainImage->usage = swapchainCI.imageUsage;
        swapChainImage->SetLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        DKObject<ImageView> swapChainImageView = DKOBJECT_NEW ImageView(queue->Device(), imageView);
        swapChainImageView->image = swapChainImage;
        swapChainImageView->waitSemaphore = frameReadySemaphore;
        swapChainImageView->signalSemaphore = frameReadySemaphore;

		this->imageViews.Add(swapChainImageView);
	}

	return true;
}

void SwapChain::SetPixelFormat(DKPixelFormat pf)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	VkFormat format = Vulkan::PixelFormat(pf);

	if (format != this->surfaceFormat.format)
	{
		if (DKPixelFormatIsColorFormat(pf))
		{
			bool formatChanged = false;
			if (availableSurfaceFormats.Count() == 1 && availableSurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
			{
				formatChanged = true;
				this->surfaceFormat.format = format;
				this->surfaceFormat.colorSpace = availableSurfaceFormats[0].colorSpace;
			}
			else
			{
				for (const VkSurfaceFormatKHR& fmt : availableSurfaceFormats)
				{
					if (fmt.format == format)
					{
						formatChanged = true;
						this->surfaceFormat = fmt;
						break;
					}
				}
			}
			if (formatChanged)
			{
				this->deviceReset = true;
				DKLogW("SwapChain::SetColorPixelFormat value changed!");
			}
			else
			{
				DKLogE("SwapChain::SetDepthStencilPixelFormat failed! (not supported format)");
			}
		}
		else
		{
			DKLogE("SwapChain::SetDepthStencilPixelFormat failed! (invalid format)");
		}
	}
}

DKPixelFormat SwapChain::PixelFormat() const
{
	DKCriticalSection<DKSpinLock> guard(lock);
	return Vulkan::PixelFormat(this->surfaceFormat.format);
}

DKRenderPassDescriptor SwapChain::CurrentRenderPassDescriptor()
{
	if (renderPassDescriptor.colorAttachments.Count() == 0)
		this->SetupFrame();

	return renderPassDescriptor;
}

size_t SwapChain::MaximumBufferCount() const
{
    DKCriticalSection<DKSpinLock> guard(lock);
    return imageViews.Count();
}

void SwapChain::SetupFrame()
{
	GraphicsDevice* dev = (GraphicsDevice*)DKGraphicsDeviceInterface::Instance(queue->Device());
	VkDevice device = dev->device;

	if (true)
	{
		lock.Lock();
		bool resetSwapChain = this->deviceReset;
		lock.Unlock();

		if (resetSwapChain)
		{
			vkDeviceWaitIdle(device);
			lock.Lock();
			this->deviceReset = false;
			lock.Unlock();
			this->Update();
		}
	}

	vkAcquireNextImageKHR(device, this->swapchain, UINT64_MAX, frameReadySemaphore, VK_NULL_HANDLE, &this->frameIndex);

	DKRenderPassColorAttachmentDescriptor colorAttachment = {};
	colorAttachment.renderTarget = imageViews.Value(frameIndex);
	colorAttachment.clearColor = DKColor(0, 0, 0, 0);
	colorAttachment.loadAction = DKRenderPassAttachmentDescriptor::LoadActionClear;
	colorAttachment.storeAction = DKRenderPassAttachmentDescriptor::StoreActionStore;

	this->renderPassDescriptor.colorAttachments.Clear();
	this->renderPassDescriptor.colorAttachments.Add(colorAttachment);
}

bool SwapChain::Present(DKGpuEvent** waitEvents, size_t numEvents)
{
    DKArray<VkSemaphore> waitSemaphores;
    waitSemaphores.Reserve(numEvents + 1);

    for (size_t i = 0; i < numEvents; ++i)
    {
        DKGpuEvent* event = waitEvents[i];
        DKASSERT_DEBUG(dynamic_cast<Semaphore*>(event) != nullptr);
        Semaphore* s = static_cast<Semaphore*>(event);
        waitSemaphores.Add(s->semaphore);
    }
    waitSemaphores.Add(frameReadySemaphore);

	VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &this->swapchain;
	presentInfo.pImageIndices = &this->frameIndex;

	// Check if a wait semaphore has been specified to wait for before presenting the image
    presentInfo.pWaitSemaphores = waitSemaphores;
    presentInfo.waitSemaphoreCount = waitSemaphores.Count();

    VkResult err = vkQueuePresentKHR(queue->queue, &presentInfo);
	if (err != VK_SUCCESS)
	{
		DKLogE("vkQueuePresentKHR ERROR: %s", VkResultCStr(err));
		//DKASSERT_DEBUG(err == VK_SUCCESS);
	}

	renderPassDescriptor.colorAttachments.Clear();
	return err == VK_SUCCESS;
}

void SwapChain::OnWindowEvent(const DKWindow::WindowEvent& e)
{
	if (e.type == DKWindow::WindowEvent::WindowResized)
	{
		DKCriticalSection<DKSpinLock> guard(lock);
		this->deviceReset = true;
	}
}

#endif //#if DKGL_ENABLE_VULKAN
