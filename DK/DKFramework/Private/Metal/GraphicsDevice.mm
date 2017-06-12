//
//  File: GraphicsDevice.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include <stdexcept>
#include "../../../Libs/SPIRV-Cross/src/spirv_msl.hpp"
#include "GraphicsDevice.h"
#include "CommandQueue.h"
#include "ShaderModule.h"
#include "../../DKPropertySet.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			DKGraphicsDeviceInterface* CreateInterface(void)
			{
				return DKRawPtrNew<GraphicsDevice>();
			}
		}
	}
}

using namespace DKFramework;
using namespace DKFramework::Private::Metal;


GraphicsDevice::GraphicsDevice(void)
{
	@autoreleasepool {
#if !TARGET_OS_IPHONE
		NSArray<id<MTLDevice>>* devices = MTLCopyAllDevices();

		NSString* preferredDeviceName = @"";
		// get preferred device.
		DKPropertySet::SystemConfig().LookUpValueForKeyPath(preferredDeviceNameKey,
															DKFunction([&preferredDeviceName](const DKVariant& var)->bool
		{
			if (var.ValueType() == DKVariant::TypeString)
			{
				DKString prefDevName = var.String();
				if (prefDevName.Length() > 0)
					preferredDeviceName = [NSString stringWithUTF8String:(const char*)DKStringU8(prefDevName)];
				return true;
			}
			return false;
        }));

		// save device list into system config.
		DKVariant deviceList = DKVariant::TypeArray;
		uint32_t deviceIndex = 0;
		for (id<MTLDevice> dev in devices)
		{
			bool pref = false;
			if (this->device == nil && preferredDeviceName)
			{
				if ([preferredDeviceName caseInsensitiveCompare:dev.name] == NSOrderedSame)
				{
					this->device = [dev retain];
					pref = true;
				}
			}

			DKString deviceName = DKString(dev.name.UTF8String);
			deviceList.Array().Add(deviceName);
			DKLog("METAL: Device[%u]: \"%s\"%s", deviceIndex, dev.name.UTF8String, pref? " (Preferred)" : "");
			deviceIndex++;
		}
		DKPropertySet::SystemConfig().SetValue(graphicsDeviceListKey, deviceList);
#endif	//if !TARGET_OS_IPHONE
		
		if (this->device == nil)
			device = MTLCreateSystemDefaultDevice();
	}

	if (this->device == nil)
	{
		throw std::runtime_error("No metal device.");
	}
}

GraphicsDevice::~GraphicsDevice(void)
{
	[device autorelease];
}

DKString GraphicsDevice::DeviceName(void) const
{
	return DKString( device.name.UTF8String );
}

DKObject<DKCommandQueue> GraphicsDevice::CreateCommandQueue(DKGraphicsDevice* dev)
{
	id<MTLCommandQueue> q = [device newCommandQueue];
	DKObject<CommandQueue> queue = DKOBJECT_NEW CommandQueue([q autorelease], dev);
	return queue.SafeCast<DKCommandQueue>();
}

DKObject<DKShaderModule> GraphicsDevice::CreateShaderModule(DKGraphicsDevice* dev, DKShader* shader)
{
    DKASSERT_DEBUG(shader);

    DKObject<DKShaderModule> module = NULL;
    if (shader->codeData)
    {
        DKDataReader reader(shader->codeData);
        if (reader.Length() > 0)
        {
            DKStringU8 spirvEntryPoint(shader->entryPoint);

            class Compiler : public spirv_cross::CompilerMSL
            {
            public:
                using Super = spirv_cross::CompilerMSL;
                using Super::Options;
                using Super::Super;
                using Super::clean_func_name;
                using Super::ensure_valid_name;
            };
            Compiler compiler(reinterpret_cast<const uint32_t*>(reader.Bytes()), reader.Length() / sizeof(uint32_t));
            Compiler::Options options;
            options.flip_vert_y = false;
            options.is_rendering_points = true;
            options.pad_and_pack_uniform_structs = false;
            options.entry_point_name = (const char*)spirvEntryPoint;

            compiler.set_options(options);

            @autoreleasepool {
                NSString* entryPoint = [NSString stringWithUTF8String:compiler.clean_func_name((const char*)spirvEntryPoint).c_str()];
                NSString* source = [NSString stringWithUTF8String:compiler.compile().c_str()];

                NSLog(@"MSL Source: Entry-point:\"%@\" (spirv:\"%s\")\n%@", entryPoint, (const char*)spirvEntryPoint, source);

                NSError* compileError = nil;
                MTLCompileOptions* compileOptions = [[[MTLCompileOptions alloc] init] autorelease];
                compileOptions.fastMathEnabled = NO;

                id<MTLLibrary> library = [device newLibraryWithSource:source
                                                              options:compileOptions
                                                                error:&compileError];

                if (compileError)
                {
                    NSLog(@"MTLLibrary compile error: %@", [compileError localizedDescription]);
                }
                if (library)
                {
                    NSLog(@"MTLLibrary: %@", library);
                    id<MTLFunction> fn = [library newFunctionWithName:entryPoint];
                    if (fn)
                    {
                        module = DKOBJECT_NEW ShaderModule(dev, library, fn);
                        [fn release];
                    }
                    NSLog(@"MTLFunction: %@", fn);
                    [library release];
                }
            }
        }
    }
    return module;
}

DKObject<DKRenderPipelineState> GraphicsDevice::CreateRenderPipeline(DKGraphicsDevice*, const DKRenderPipelineDescriptor&, DKPipelineReflection*)
{
    return NULL;
}

DKObject<DKComputePipelineState> GraphicsDevice::CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKPipelineReflection*)
{
    return NULL;
}

#endif //#if DKGL_ENABLE_METAL
