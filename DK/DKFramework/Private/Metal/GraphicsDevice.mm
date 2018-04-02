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
#include "../../DKPropertySet.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Metal
		{
			DKGraphicsDeviceInterface* CreateInterface(void)
			{
				return new GraphicsDevice();
			}
		}
	}
}

using namespace DKFramework;
using namespace DKFramework::Private::Metal;


GraphicsDevice::GraphicsDevice(void)
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

GraphicsDevice::~GraphicsDevice(void)
{
	[device autorelease];
}

DKString GraphicsDevice::DeviceName(void) const
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

    if (shader->codeData)
    {
        DKDataReader reader(shader->codeData);
        if (reader.Length() > 0)
        {
			switch (shader->stage)
			{
				case DKShader::StageType::Vertex:					
				case DKShader::StageType::TessellationControl:		
				case DKShader::StageType::TessellationEvaluation:	
				case DKShader::StageType::Geometry:					
				case DKShader::StageType::Fragment:					
				case DKShader::StageType::Compute:		
					break;
				default:
					DKLogE("Error: Invalid shader stage!");
					return NULL;
			}

			using Compiler = spirv_cross::CompilerMSL;
			Compiler compiler(reinterpret_cast<const uint32_t*>(reader.Bytes()), reader.Length() / sizeof(uint32_t));
			std::vector<std::string> entryPointNames = compiler.get_entry_points();
			if (entryPointNames.empty())
			{
					DKLogE("Error: No entry point function!");
					return NULL;			
			}
#if 0
            Compiler::Options options = compiler.get_options();
            options.flip_vert_y = false;
            options.enable_point_size_builtin = true;
            options.entry_point_name = nullptr;
            compiler.set_options(options);
#endif
            @autoreleasepool {
                NSString* source = [NSString stringWithUTF8String:compiler.compile().c_str()];

                NSLog(@"MSL Source:\n%@\n", source);

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

					[library release];

					return module.SafeCast<DKShaderModule>();
                }
            }
        }
    }
    return NULL;
}

DKObject<DKRenderPipelineState> GraphicsDevice::CreateRenderPipeline(DKGraphicsDevice* dev, const DKRenderPipelineDescriptor& desc, DKPipelineReflection* reflection)
{
	MTLPrimitiveType primitiveType;
	switch (desc.primitiveTopology)
	{
		case DKPrimitiveTopology::Points:			primitiveType = MTLPrimitiveTypePoint;			break;
		case DKPrimitiveTopology::Lines:			primitiveType = MTLPrimitiveTypeLine;			break;
		case DKPrimitiveTopology::LineStrips:		primitiveType = MTLPrimitiveTypeLineStrip;		break;
		case DKPrimitiveTopology::Triangles:		primitiveType = MTLPrimitiveTypeTriangle;		break;
		case DKPrimitiveTopology::TriangleStrips:	primitiveType = MTLPrimitiveTypeTriangleStrip;	break;
		default:
			DKLogE("Unsupported primitive topology: %d(0x%x)", (int)desc.primitiveTopology, (int)desc.primitiveTopology);
			return NULL;
	}

	DKObject<RenderPipelineState> state = NULL;
	@autoreleasepool {
		NSError* error = nil;
		MTLRenderPipelineDescriptor* descriptor = [[[MTLRenderPipelineDescriptor alloc] init] autorelease];

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

		const ShaderFunction* vertexFunction = desc.vertexFunction.StaticCast<ShaderFunction>();
		if (vertexFunction)
			descriptor.vertexFunction = vertexFunction->function;
		const ShaderFunction* fragmentFunction = desc.fragmentFunction.StaticCast<ShaderFunction>();
		if (fragmentFunction)
			descriptor.fragmentFunction = fragmentFunction->function;

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
		if (desc.vertexDescriptor.attributes.Count() > 0 || desc.vertexDescriptor.layouts.Count() > 0)
		{
			MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
			for (const DKVertexAttributeDescriptor& attrDesc : desc.vertexDescriptor.attributes)
			{
				MTLVertexAttributeDescriptor* attr = [vertexDescriptor.attributes objectAtIndexedSubscript:attrDesc.location];
				attr.format = GetVertexFormat(attrDesc.format);
				attr.offset = attrDesc.offset;
				attr.bufferIndex = attrDesc.bufferIndex;
			}
			for (const DKVertexBufferLayoutDescriptor& layoutDesc : desc.vertexDescriptor.layouts)
			{
				MTLVertexBufferLayoutDescriptor* layout = [vertexDescriptor.layouts objectAtIndexedSubscript:layoutDesc.bufferIndex];
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
			[pipelineState autorelease];
		}
	}
	return state.SafeCast<DKRenderPipelineState>();
}

DKObject<DKComputePipelineState> GraphicsDevice::CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKPipelineReflection*)
{
    return NULL;
}

DKObject<DKGpuBuffer> GraphicsDevice::CreateBuffer(size_t, DKGpuStorageMode, DKCpuCacheMode)
{
	return NULL;
}

DKObject<DKTexture> GraphicsDevice::CreateTexture(const DKTextureDescriptor&)
{
	return NULL;
}

#endif //#if DKGL_ENABLE_METAL
