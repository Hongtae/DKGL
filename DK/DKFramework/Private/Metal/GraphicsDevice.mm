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
#include "ShaderFunction.h"
#include "ShaderModule.h"
#include "PixelFormat.h"
#include "RenderPipelineState.h"
#include "ComputePipelineState.h"
#include "Buffer.h"
#include "Texture.h"
#include "Types.h"
#include "../../DKPropertySet.h"

namespace DKFramework::Private::Metal
{
	DKGraphicsDeviceInterface* CreateInterface()
	{
		return new GraphicsDevice();
	}
}

using namespace DKFramework;
using namespace DKFramework::Private::Metal;


GraphicsDevice::GraphicsDevice()
: device(nil)
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
			DKLog("MTLDevice[%u]: \"%s\" (depth24Stencil8PixelFormatSupported:%d, headless:%d, lowPower:%d, maxThreadsPerThreadgroup:%dx%dx%d, recommendedMaxWorkingSetSize:%s%s)",
                  deviceIndex,
                  dev.name.UTF8String,
                  (int)dev.depth24Stencil8PixelFormatSupported,
                  (int)dev.headless,
                  (int)dev.lowPower,
                  (int)dev.maxThreadsPerThreadgroup.width,
                  (int)dev.maxThreadsPerThreadgroup.height,
                  (int)dev.maxThreadsPerThreadgroup.depth,
                  [NSByteCountFormatter stringFromByteCount:dev.recommendedMaxWorkingSetSize
                                                 countStyle:NSByteCountFormatterCountStyleMemory].UTF8String,
                  pref? ", Preferred" : "");
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

GraphicsDevice::~GraphicsDevice()
{
	[device autorelease];
}

DKString GraphicsDevice::DeviceName() const
{
	return DKString( device.name.UTF8String );
}

DKObject<DKCommandQueue> GraphicsDevice::CreateCommandQueue(DKGraphicsDevice* dev, uint32_t)
{
	id<MTLCommandQueue> q = [device newCommandQueue];
	DKObject<CommandQueue> queue = DKOBJECT_NEW CommandQueue(dev, [q autorelease]);
	return queue.SafeCast<DKCommandQueue>();
}

DKObject<DKShaderModule> GraphicsDevice::CreateShaderModule(DKGraphicsDevice* dev, DKShader* shader)
{
    DKASSERT_DEBUG(shader);

    if (DKData* data = shader->Data(); data)
    {
        DKDataReader reader(data);
        if (reader.Length() > 0)
        {
            spv::ExecutionModel stage;
			switch (shader->Stage())
			{
                case DKShader::StageType::Vertex:
                    stage = spv::ExecutionModelVertex;
                    break;
                case DKShader::StageType::Fragment:
                    stage = spv::ExecutionModelFragment;
                    break;
                case DKShader::StageType::Compute:
                    stage = spv::ExecutionModelGLCompute;
					break;
				default:
					DKLogE("Error: Invalid shader stage!");
					return NULL;
			}

            uint32_t numBuffers = 0;
            uint32_t numTextures = 0;
            uint32_t numSamplers = 0;
            DKArray<spirv_cross::MSLResourceBinding> bindings1;
            DKArray<ResourceBinding> bindings2;

            const DKArray<DKShaderResource>& resources = shader->Resources();
            bindings1.Reserve(resources.Count() + 1);
            bindings2.Reserve(resources.Count() + 1);

            for (const DKShaderResource& res : resources)
            {
                spirv_cross::MSLResourceBinding b1 = { stage };
                ResourceBinding b2 = {};

                b1.desc_set = b2.set = res.set;
                b1.binding = b2.binding = res.binding;
                b1.msl_buffer = b2.bufferIndex = numBuffers;
                b1.msl_texture = b2.textureIndex = numTextures;
                b1.msl_sampler = b2.samplerIndex = numSamplers;
                b2.type = res.type;

                bindings1.Add(b1);
                bindings2.Add(b2);

                switch (res.type)
                {
                    case DKShaderResource::TypeBuffer:
                        numBuffers++;
                        break;
                    case DKShaderResource::TypeTexture:
                        numTextures++;
                        break;
                    case DKShaderResource::TypeSampler:
                        numSamplers++;
                        break;
                    case DKShaderResource::TypeSampledTexture:
                        numSamplers++;
                        numTextures++;
                        break;
                }
            }
            if (const DKArray<DKShader::PushConstantLayout>& layouts = shader->PushConstantBufferLayouts(); layouts.Count() > 0)
            {
                DKASSERT_DESC_DEBUG(layouts.Count() == 1, "There can be only one push constant block!");

                // add push-constant
                spirv_cross::MSLResourceBinding b1 = { stage };
                ResourceBinding b2 = {};

                b1.desc_set = b2.set = spirv_cross::kPushConstDescSet;
                b1.binding = b2.binding = spirv_cross::kPushConstBinding;
                b1.msl_buffer = b2.bufferIndex = numBuffers;
                b1.msl_texture = b2.textureIndex = numTextures;
                b1.msl_sampler = b2.samplerIndex = numSamplers;
                b2.type = DKShaderResource::TypeBuffer;

                bindings1.Add(b1);
                bindings2.Add(b2);

                numBuffers++;
            }

			using Compiler = spirv_cross::CompilerMSL;
			Compiler compiler(reinterpret_cast<const uint32_t*>(reader.Bytes()), reader.Length() / sizeof(uint32_t),
                              nullptr, 0,
                              bindings1, bindings1.Count());
			std::vector<spirv_cross::EntryPoint> entryPoints = compiler.get_entry_points_and_stages();
			if (entryPoints.empty())
			{
					DKLogE("Error: No entry point function!");
					return NULL;			
			}

            Compiler::Options mslOptions = compiler.get_msl_options();
#if TARGET_OS_IPHONE
			mslOptions.platform = Compiler::Options::iOS;
#else
			mslOptions.platform = Compiler::Options::macOS;
#endif
			mslOptions.set_msl_version(2, 1);
            mslOptions.enable_point_size_builtin = true;
            mslOptions.resolve_specialized_array_lengths = true;
            compiler.set_msl_options(mslOptions);

//			auto commonOptions = compiler.get_common_options();
//			compiler.set_common_options(commonOptions);

            @autoreleasepool {
                NSString* source = [NSString stringWithUTF8String:compiler.compile().c_str()];

                NSLog(@"MSL Source:\n%@\n", source);

                auto workgroupSize = shader->ThreadgroupSize();

                NSError* compileError = nil;
                MTLCompileOptions* compileOptions = [[[MTLCompileOptions alloc] init] autorelease];
                compileOptions.fastMathEnabled = YES;

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

					DKObject<ShaderModule> module = DKOBJECT_NEW ShaderModule(dev, library);
                    module->workgroupSize = MTLSizeMake(workgroupSize.x, workgroupSize.y, workgroupSize.z);

                    DKASSERT_DEBUG(bindings1.Count() == bindings2.Count());

                    StageResourceBindingMap bindingMap = {};
                    bindingMap.resourceBindings.Reserve(bindings1.Count());
                    for (size_t i = 0, n = bindings1.Count(); i < n; ++i)
                    {
                        const spirv_cross::MSLResourceBinding& b1 = bindings1.Value(i);
                        const ResourceBinding& b2 = bindings2.Value(i);

                        if (b1.used_by_shader)
                        {
                            if (b1.desc_set == spirv_cross::kPushConstDescSet)
                                bindingMap.pushConstantIndex = b1.msl_buffer;
                            else
                                bindingMap.resourceBindings.Add(b2);
                        }
                    }

                    bindingMap.resourceBindings.Sort([](auto& a, auto& b) {
                        if (a.set == b.set)
                            return a.binding < b.binding;
                        return a.set < b.set;
                    });
                    bindingMap.resourceBindings.ShrinkToFit();
                    bindingMap.inputAttributeIndexOffset = numBuffers;

                    module->bindings = std::move(bindingMap);

					[library release];

					return module.SafeCast<DKShaderModule>();
                }
            }
        }
    }
    return NULL;
}

DKObject<DKRenderPipelineState> GraphicsDevice::CreateRenderPipeline(DKGraphicsDevice* dev, const DKRenderPipelineDescriptor& desc, DKRenderPipelineReflection* reflection)
{
	MTLPrimitiveType primitiveType;
	switch (desc.primitiveTopology)
	{
		case DKPrimitiveType::Point:			primitiveType = MTLPrimitiveTypePoint;			break;
		case DKPrimitiveType::Line:				primitiveType = MTLPrimitiveTypeLine;			break;
		case DKPrimitiveType::LineStrip:		primitiveType = MTLPrimitiveTypeLineStrip;		break;
		case DKPrimitiveType::Triangle:			primitiveType = MTLPrimitiveTypeTriangle;		break;
		case DKPrimitiveType::TriangleStrip:	primitiveType = MTLPrimitiveTypeTriangleStrip;	break;
		default:
			DKLogE("Unsupported primitive topology: %d(0x%x)", (int)desc.primitiveTopology, (int)desc.primitiveTopology);
			return NULL;
	}

    if (desc.vertexFunction)
    {
        DKASSERT_DEBUG(desc.vertexFunction.SafeCast<ShaderFunction>());
        DKASSERT_DEBUG(desc.vertexFunction.SafeCast<ShaderFunction>()->function.functionType == MTLFunctionTypeVertex);
    }
    if (desc.fragmentFunction)
    {
        DKASSERT_DEBUG(desc.fragmentFunction.SafeCast<ShaderFunction>());
        DKASSERT_DEBUG(desc.fragmentFunction.SafeCast<ShaderFunction>()->function.functionType == MTLFunctionTypeFragment);
    }

	DKObject<RenderPipelineState> state = NULL;
	@autoreleasepool {
		NSError* error = nil;
		MTLRenderPipelineDescriptor* descriptor = [[[MTLRenderPipelineDescriptor alloc] init] autorelease];

        NSUInteger vertexAttributeOffset = 0;

        const ShaderFunction* vertexFunction = desc.vertexFunction.StaticCast<ShaderFunction>();
        const ShaderFunction* fragmentFunction = desc.fragmentFunction.StaticCast<ShaderFunction>();

        if (vertexFunction)
        {
            DKASSERT_DEBUG(desc.vertexFunction.SafeCast<ShaderFunction>() != nullptr);
            DKASSERT_DEBUG(vertexFunction->function.functionType == MTLFunctionTypeVertex);

            const ShaderModule* module = vertexFunction->module.StaticCast<ShaderModule>();
            descriptor.vertexFunction = vertexFunction->function;
            vertexAttributeOffset = module->bindings.inputAttributeIndexOffset;
        }
        if (fragmentFunction)
        {
            DKASSERT_DEBUG(desc.fragmentFunction.SafeCast<ShaderFunction>() != nullptr);
            DKASSERT_DEBUG(fragmentFunction->function.functionType == MTLFunctionTypeFragment);

            descriptor.fragmentFunction = fragmentFunction->function;
        }

		auto GetVertexFormat = [](DKVertexFormat f)->MTLVertexFormat
		{
			switch (f)
			{
				case DKVertexFormat::UChar2:				return MTLVertexFormatUChar2;
				case DKVertexFormat::UChar3:				return MTLVertexFormatUChar3;
				case DKVertexFormat::UChar4:				return MTLVertexFormatUChar4;
				case DKVertexFormat::Char2:					return MTLVertexFormatChar2;
				case DKVertexFormat::Char3:					return MTLVertexFormatChar3;
				case DKVertexFormat::Char4:					return MTLVertexFormatChar4;
				case DKVertexFormat::UChar2Normalized:		return MTLVertexFormatUChar2Normalized;
				case DKVertexFormat::UChar3Normalized:		return MTLVertexFormatUChar3Normalized;
				case DKVertexFormat::UChar4Normalized:		return MTLVertexFormatUChar4Normalized;
				case DKVertexFormat::Char2Normalized:		return MTLVertexFormatChar2Normalized;
				case DKVertexFormat::Char3Normalized:		return MTLVertexFormatChar3Normalized;
				case DKVertexFormat::Char4Normalized:		return MTLVertexFormatChar4Normalized;
				case DKVertexFormat::UShort2:				return MTLVertexFormatUShort2;
				case DKVertexFormat::UShort3:				return MTLVertexFormatUShort3;
				case DKVertexFormat::UShort4:				return MTLVertexFormatUShort4;
				case DKVertexFormat::Short2:				return MTLVertexFormatShort2;
				case DKVertexFormat::Short3:				return MTLVertexFormatShort3;
				case DKVertexFormat::Short4:				return MTLVertexFormatShort4;
				case DKVertexFormat::UShort2Normalized:		return MTLVertexFormatUShort2Normalized;
				case DKVertexFormat::UShort3Normalized:		return MTLVertexFormatUShort3Normalized;
				case DKVertexFormat::UShort4Normalized:		return MTLVertexFormatUShort4Normalized;
				case DKVertexFormat::Short2Normalized:		return MTLVertexFormatShort2Normalized;
				case DKVertexFormat::Short3Normalized:		return MTLVertexFormatShort3Normalized;
				case DKVertexFormat::Short4Normalized:		return MTLVertexFormatShort4Normalized;
				case DKVertexFormat::Half2:					return MTLVertexFormatHalf2;
				case DKVertexFormat::Half3:					return MTLVertexFormatHalf3;
				case DKVertexFormat::Half4:					return MTLVertexFormatHalf4;
				case DKVertexFormat::Float:					return MTLVertexFormatFloat;
				case DKVertexFormat::Float2:				return MTLVertexFormatFloat2;
				case DKVertexFormat::Float3:				return MTLVertexFormatFloat3;
				case DKVertexFormat::Float4:				return MTLVertexFormatFloat4;
				case DKVertexFormat::Int:					return MTLVertexFormatInt;
				case DKVertexFormat::Int2:					return MTLVertexFormatInt2;
				case DKVertexFormat::Int3:					return MTLVertexFormatInt3;
				case DKVertexFormat::Int4:					return MTLVertexFormatInt4;
				case DKVertexFormat::UInt:					return MTLVertexFormatUInt;
				case DKVertexFormat::UInt2:					return MTLVertexFormatUInt2;
				case DKVertexFormat::UInt3:					return MTLVertexFormatUInt3;
				case DKVertexFormat::UInt4:					return MTLVertexFormatUInt4;
				case DKVertexFormat::Int1010102Normalized:	return MTLVertexFormatInt1010102Normalized;
				case DKVertexFormat::UInt1010102Normalized:	return MTLVertexFormatUInt1010102Normalized;
			}
			return MTLVertexFormatInvalid;
		};

		auto GetVertexStepFunction = [](DKVertexStepRate fn)->MTLVertexStepFunction
		{
			switch (fn)
			{
				case DKVertexStepRate::Vertex:		return MTLVertexStepFunctionPerVertex;
				case DKVertexStepRate::Instance:	return MTLVertexStepFunctionPerInstance;
			}
			DKASSERT_DESC_DEBUG(0, "Unknown value!");
			return MTLVertexStepFunctionConstant;
		};

		auto GetBlendFactor = [](DKBlendFactor f)->MTLBlendFactor
		{
			switch (f)
			{
				case DKBlendFactor::Zero:						return MTLBlendFactorZero;
				case DKBlendFactor::One:						return MTLBlendFactorOne;
				case DKBlendFactor::SourceColor:				return MTLBlendFactorSourceColor;
				case DKBlendFactor::OneMinusSourceColor:		return MTLBlendFactorOneMinusSourceColor;
				case DKBlendFactor::SourceAlpha:				return MTLBlendFactorSourceAlpha;
				case DKBlendFactor::OneMinusSourceAlpha:		return MTLBlendFactorOneMinusSourceAlpha;
				case DKBlendFactor::DestinationColor:			return MTLBlendFactorDestinationColor;
				case DKBlendFactor::OneMinusDestinationColor:	return MTLBlendFactorOneMinusDestinationColor;
				case DKBlendFactor::DestinationAlpha:			return MTLBlendFactorDestinationAlpha;
				case DKBlendFactor::OneMinusDestinationAlpha:	return MTLBlendFactorOneMinusDestinationAlpha;
				case DKBlendFactor::SourceAlphaSaturated:		return MTLBlendFactorSourceAlphaSaturated;
				case DKBlendFactor::BlendColor:					return MTLBlendFactorBlendColor;
				case DKBlendFactor::OneMinusBlendColor:			return MTLBlendFactorOneMinusBlendColor;
				case DKBlendFactor::BlendAlpha:					return MTLBlendFactorBlendAlpha;
				case DKBlendFactor::OneMinusBlendAlpha:			return MTLBlendFactorOneMinusBlendAlpha;
			}
		};
		auto GetBlendOperation = [](DKBlendOperation o)->MTLBlendOperation
		{
			switch (o)
			{
				case DKBlendOperation::Add:				return MTLBlendOperationAdd;
				case DKBlendOperation::Subtract:		return MTLBlendOperationSubtract;
				case DKBlendOperation::ReverseSubtract:	return MTLBlendOperationReverseSubtract;
				case DKBlendOperation::Min:				return MTLBlendOperationMin;
				case DKBlendOperation::Max:				return MTLBlendOperationMax;
			}
		};
		auto GetColorWriteMask = [](DKColorWriteMask mask)->MTLColorWriteMask
		{
			MTLColorWriteMask value = MTLColorWriteMaskNone;
			if (mask & DKColorWriteMaskRed)		value |= MTLColorWriteMaskRed;
			if (mask & DKColorWriteMaskGreen)	value |= MTLColorWriteMaskGreen;
			if (mask & DKColorWriteMaskBlue)	value |= MTLColorWriteMaskBlue;
			if (mask & DKColorWriteMaskAlpha)	value |= MTLColorWriteMaskAlpha;
			return value;
		};

		for (const DKRenderPipelineColorAttachmentDescriptor& attachment : desc.colorAttachments)
		{
			MTLRenderPipelineColorAttachmentDescriptor* colorAttachmentDesc = [descriptor.colorAttachments objectAtIndexedSubscript:attachment.index];
			colorAttachmentDesc.pixelFormat = PixelFormat::From(attachment.pixelFormat);
			colorAttachmentDesc.writeMask = GetColorWriteMask(attachment.writeMask);
			colorAttachmentDesc.blendingEnabled = attachment.blendingEnabled;
			colorAttachmentDesc.alphaBlendOperation = GetBlendOperation(attachment.alphaBlendOperation);
			colorAttachmentDesc.rgbBlendOperation = GetBlendOperation(attachment.rgbBlendOperation);
			colorAttachmentDesc.sourceRGBBlendFactor = GetBlendFactor(attachment.sourceRGBBlendFactor);
			colorAttachmentDesc.sourceAlphaBlendFactor = GetBlendFactor(attachment.sourceAlphaBlendFactor);
			colorAttachmentDesc.destinationRGBBlendFactor = GetBlendFactor(attachment.destinationRGBBlendFactor);
			colorAttachmentDesc.destinationAlphaBlendFactor = GetBlendFactor(attachment.destinationAlphaBlendFactor);
		}
		DKSet<NSUInteger> vertexAttributeIndices; // buffer index converted for device
		if (desc.vertexDescriptor.attributes.Count() > 0 || desc.vertexDescriptor.layouts.Count() > 0)
		{
			MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
			for (const DKVertexAttributeDescriptor& attrDesc : desc.vertexDescriptor.attributes)
			{
				MTLVertexAttributeDescriptor* attr = [vertexDescriptor.attributes objectAtIndexedSubscript:attrDesc.location];
				attr.format = GetVertexFormat(attrDesc.format);
				attr.offset = attrDesc.offset;
				NSUInteger bufferIndex = vertexAttributeOffset + attrDesc.bufferIndex;
				attr.bufferIndex = bufferIndex;
				vertexAttributeIndices.Insert(bufferIndex);
			}
			for (const DKVertexBufferLayoutDescriptor& layoutDesc : desc.vertexDescriptor.layouts)
			{
				NSUInteger bufferIndex = vertexAttributeOffset + layoutDesc.bufferIndex;
				MTLVertexBufferLayoutDescriptor* layout = [vertexDescriptor.layouts objectAtIndexedSubscript:bufferIndex];
				layout.stepFunction = GetVertexStepFunction(layoutDesc.step);
				layout.stepRate = 1;
				layout.stride = layoutDesc.stride;
			}
			descriptor.vertexDescriptor = vertexDescriptor;
		}

		id<MTLRenderPipelineState> pipelineState = nil;
		if (reflection)
		{
			MTLPipelineOption options = MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo;
			MTLRenderPipelineReflection* pipelineReflection = nil;
			pipelineState = [device newRenderPipelineStateWithDescriptor:descriptor
																 options:options
															  reflection:&pipelineReflection
																   error:&error];
			if (pipelineReflection)
			{
				NSLog(@"RenderPipelineReflection: %@", pipelineReflection);

                NSArray<MTLArgument*>* fragmentArguments = pipelineReflection.fragmentArguments;
                NSArray<MTLArgument*>* vertexArguments = [&vertexAttributeIndices](NSArray<MTLArgument*>* args)->NSArray<MTLArgument*>*
                {
                    NSMutableArray<MTLArgument*>* array = [[[NSMutableArray alloc] initWithCapacity:args.count] autorelease];
                    for (MTLArgument* arg in args)
                    {
                        if (arg.type == MTLArgumentTypeBuffer && vertexAttributeIndices.Contains(arg.index))
                            continue;

                        [array addObject:arg];
                    }
                    return array;
                }(pipelineReflection.vertexArguments);

                reflection->vertexResources = this->ShaderResourceArrayFromArguments(vertexArguments);
                reflection->fragmentResources = this->ShaderResourceArrayFromArguments(fragmentArguments);
			}
		}
		else
			pipelineState = [device newRenderPipelineStateWithDescriptor:descriptor error:&error];

		if (error)
		{
			DKLogE("GraphicsDevice::CreateRenderPipeline Error: %s", (const char*)error.localizedDescription.UTF8String);
		}

		if (pipelineState)
		{
			state = DKOBJECT_NEW RenderPipelineState(dev, pipelineState);
			state->primitiveType = primitiveType;
            if (vertexFunction)
                state->vertexBindings = vertexFunction->module.StaticCast<ShaderModule>()->bindings;
            if (fragmentFunction)
                state->fragmentBindings = fragmentFunction->module.StaticCast<ShaderModule>()->bindings;
			[pipelineState autorelease];
		}
	}
	return state.SafeCast<DKRenderPipelineState>();
}

DKObject<DKComputePipelineState> GraphicsDevice::CreateComputePipeline(DKGraphicsDevice* dev, const DKComputePipelineDescriptor& desc, DKComputePipelineReflection* reflection)
{
    if (!desc.computeFunction)
        return NULL;

    DKASSERT_DEBUG(desc.computeFunction.SafeCast<ShaderFunction>());
    DKASSERT_DEBUG(desc.computeFunction.SafeCast<ShaderFunction>()->function.functionType == MTLFunctionTypeKernel);

    DKObject<ComputePipelineState> state = NULL;
    @autoreleasepool {
        NSError* error = nil;
        MTLComputePipelineDescriptor* descriptor = [[[MTLComputePipelineDescriptor alloc] init] autorelease];

        const ShaderFunction* func = desc.computeFunction.StaticCast<ShaderFunction>();
        descriptor.computeFunction = func->function;

        id<MTLComputePipelineState> pipelineState = nil;

        MTLPipelineOption options = 0;
        if (reflection)
            options = MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo;
        MTLComputePipelineReflection* pipelineReflection = nil;
        pipelineState = [device newComputePipelineStateWithDescriptor:descriptor
                                                              options:options
                                                           reflection:&pipelineReflection
                                                                error:&error];
        if (reflection && pipelineReflection)
        {
            NSLog(@"ComputePipelineReflection: %@", pipelineReflection);

            reflection->resources = this->ShaderResourceArrayFromArguments(pipelineReflection.arguments);
        }

        if (error)
        {
            DKLogE("GraphicsDevice::CreateRenderPipeline Error: %s", (const char*)error.localizedDescription.UTF8String);
        }

        if (pipelineState)
        {
            state = DKOBJECT_NEW ComputePipelineState(dev, pipelineState, func->workgroupSize);
            state->bindings = func->module.StaticCast<ShaderModule>()->bindings;

            [pipelineState autorelease];
        }
    }
    return state.SafeCast<DKComputePipelineState>();
}

DKObject<DKGpuBuffer> GraphicsDevice::CreateBuffer(DKGraphicsDevice* dev, size_t size, DKGpuBuffer::StorageMode storage, DKCpuCacheMode cache)
{
	if (size > 0)
	{
		MTLResourceOptions options = 0;
		switch (storage)
		{
			case DKGpuBuffer::StorageModeShared:
				options |= MTLResourceStorageModeShared;
				break;
			case DKGpuBuffer::StorageModePrivate:
				options |= MTLResourceStorageModePrivate;
				break;
		}
		switch (cache)
		{
			case DKCpuCacheModeDefault:
				options |= MTLResourceCPUCacheModeDefaultCache;
				break;
			case DKCpuCacheModeWriteCombined:
				options |= MTLResourceCPUCacheModeWriteCombined;
				break;
		}

		id<MTLBuffer> buffer = [device newBufferWithLength:size options:options];
		if (buffer)
		{
			return DKOBJECT_NEW Buffer(dev, buffer);
		}
		else
		{
			DKLogE("GraphicsDevice::CreateBuffer Error!");
		}
	}
	return NULL;
}

DKObject<DKTexture> GraphicsDevice::CreateTexture(DKGraphicsDevice*, const DKTextureDescriptor&)
{
	return NULL;
}

DKArray<DKShaderResource> GraphicsDevice::ShaderResourceArrayFromArguments(NSArray<MTLArgument*>* arguments) const
{
    struct GetStructTypeData
    {
        DKString name;
        DKShaderResource& base;
        DKString operator () (MTLStructType* st)
        {
            DKShaderResourceStruct output;
            output.members.Reserve(st.members.count);
            for (MTLStructMember* member in st.members)
            {
                DKShaderResourceStructMember mb;
                MTLDataType type = member.dataType;
                MTLStructType* memberStruct = nil;

                if (type == MTLDataTypeArray)
                {
                    MTLArrayType* arrayType = member.arrayType;

                    mb.count = (uint32_t)arrayType.arrayLength;
                    mb.stride = (uint32_t)arrayType.stride;
                    mb.dataType = ShaderDataType::To(arrayType.elementType);
                    if (arrayType.elementType == MTLDataTypeStruct)
                        memberStruct = arrayType.elementStructType;
                }
                else
                {
                    mb.count = 1;
                    mb.dataType = ShaderDataType::To(type);
                }

                mb.name = member.name.UTF8String;
                mb.offset = (uint32_t)member.offset;

                if (type == MTLDataTypeStruct)
                    memberStruct = member.structType;
                if (memberStruct)
                {
                    DKString typeKey = DKString(this->name).Append(".").Append(mb.name);
                    mb.typeInfoKey = GetStructTypeData{ typeKey, base }.operator() (memberStruct);
                }
                output.members.Add(mb);
            }
            base.structTypeMemberMap.Update(name, output);
            return name;
        }
    };

    auto shaderResourceFromArgument = [](MTLArgument* arg, DKShaderResource& res)->bool
    {
        res.set = 0;
        res.binding = (uint32_t)arg.index;
        res.name = DKStringU8(arg.name.UTF8String);
        res.count = (uint32_t)arg.arrayLength;
        res.enabled = arg.active;
        switch (arg.access)
        {
            case MTLArgumentAccessReadOnly:
                res.access = DKShaderResource::AccessReadOnly; break;
            case MTLArgumentAccessWriteOnly:
                res.access = DKShaderResource::AccessWriteOnly; break;
            case MTLArgumentAccessReadWrite:
                res.access = DKShaderResource::AccessReadWrite; break;
            default:
                NSLog(@"ERROR: Unknown access type!");
                return false;
        }

        switch (arg.type)
        {
            case MTLArgumentTypeBuffer:
                res.type = DKShaderResource::TypeBuffer;
                res.typeInfo.buffer.size = (uint32_t)arg.bufferDataSize;
                //res.typeInfo.buffer.alignment = (uint32_t)arg.bufferAlignment;
                res.typeInfo.buffer.dataType = ShaderDataType::To(arg.bufferDataType);
                if (arg.bufferDataType == MTLDataTypeStruct)
                {
                    res.typeInfoKey = GetStructTypeData{res.name, res}.operator()(arg.bufferStructType);
                }
                else
                {
                    NSLog(@"WARNING: Unsupported buffer type: %@", arg);
                }
                break;
//            case MTLArgumentTypeThreadgroupMemory:
//                res.type = DKShaderResource::TypeThreadgroupMemory;
//                break;
            case MTLArgumentTypeTexture:
                res.type = DKShaderResource::TypeTexture;
                break;
            case MTLArgumentTypeSampler:
                res.type = DKShaderResource::TypeSampler;
                break;
            default:
                NSLog(@"WARNING: Unsupported shader argument type: %@", arg);
                return false;
        }
        return true;
    };

    DKArray<DKShaderResource> resources;
    resources.Reserve(arguments.count);
    for (MTLArgument* arg in arguments)
    {
        DKShaderResource res;
        if (shaderResourceFromArgument(arg, res))
        {
            bool combined = false;
            if (res.type == DKShaderResource::TypeSampler ||
                res.type == DKShaderResource::TypeTexture)
            {
                for (DKShaderResource& r : resources)
                {
                    if (r.binding == res.binding)
                    {
                        if ((res.type == DKShaderResource::TypeSampler && r.type == DKShaderResource::TypeTexture) ||
                            (res.type == DKShaderResource::TypeTexture && r.type == DKShaderResource::TypeSampler))
                        {
                            r.type = DKShaderResource::TypeSampledTexture;
                            combined = true;
                            break;
                        }
                        else
                        {
                            DKASSERT_DESC_DEBUG(0, "Invalid resource type!");
                        }
                    }
                }
            }
            if (!combined)
                resources.Add(res);
        }
    }
    resources.ShrinkToFit();
    return resources;
}

#endif //#if DKGL_ENABLE_METAL
