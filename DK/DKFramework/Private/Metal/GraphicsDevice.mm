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

    if (shader->codeData)
    {
        DKDataReader reader(shader->codeData);
        if (reader.Length() > 0)
        {
			switch (shader->stage)
			{
				case DKShader::StageType::Vertex:					
				case DKShader::StageType::Fragment:
				case DKShader::StageType::Compute:		
					break;
				default:
					DKLogE("Error: Invalid shader stage!");
					return NULL;
			}

			using Compiler = spirv_cross::CompilerMSL;
			Compiler compiler(reinterpret_cast<const uint32_t*>(reader.Bytes()), reader.Length() / sizeof(uint32_t));
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
		DKSet<NSUInteger> vertexAttributeIndices; // buffer index converted for device
		if (desc.vertexDescriptor.attributes.Count() > 0 || desc.vertexDescriptor.layouts.Count() > 0)
		{
			MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
			for (const DKVertexAttributeDescriptor& attrDesc : desc.vertexDescriptor.attributes)
			{
				MTLVertexAttributeDescriptor* attr = [vertexDescriptor.attributes objectAtIndexedSubscript:attrDesc.location];
				attr.format = GetVertexFormat(attrDesc.format);
				attr.offset = attrDesc.offset;
				NSUInteger bufferIndex = VertexAttributeIndexForDevice(attrDesc.bufferIndex);
				attr.bufferIndex = bufferIndex;
				vertexAttributeIndices.Insert(bufferIndex);
			}
			for (const DKVertexBufferLayoutDescriptor& layoutDesc : desc.vertexDescriptor.layouts)
			{
				NSUInteger bufferIndex = VertexAttributeIndexForDevice(layoutDesc.bufferIndex);
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
							if (type == MTLDataTypeArray)
							{
								MTLArrayType* arrayType = member.arrayType;
								type = arrayType.elementType;
								mb.count = arrayType.arrayLength;
								mb.stride = arrayType.stride;
							}
							else
							{
								mb.count = 1;
							}
							mb.dataType = ShaderDataType::To(type);
							mb.name = member.name.UTF8String;
							mb.offset = member.offset;

							if (type == MTLDataTypeStruct)
							{
								DKString typeKey = DKString(this->name).Append(".").Append(mb.name);
								mb.typeInfoKey = GetStructTypeData{ typeKey, base }.operator() (member.structType);
							}
							output.members.Add(mb);
						}
						base.structTypeMemberMap.Update(name, output);
						return name;
					}
				};

				auto shaderResourceFromArgument = [&vertexAttributeIndices](MTLArgument* arg, DKShaderResource& res)->bool
				{
					if (arg.type == MTLArgumentTypeBuffer && vertexAttributeIndices.Contains(arg.index))
						return false;

					res.set = 0;
					res.binding = (uint32_t)arg.index;
					res.name = DKStringU8(arg.name.UTF8String);
					res.count = arg.arrayLength;
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
							res.typeInfo.buffer.size = arg.bufferDataSize;
							res.typeInfo.buffer.alignment = arg.bufferAlignment;
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
						case MTLArgumentTypeThreadgroupMemory:	res.type = DKShaderResource::TypeThreadgroupMemory; break;
						case MTLArgumentTypeTexture:			res.type = DKShaderResource::TypeTexture; break;
						case MTLArgumentTypeSampler:			res.type = DKShaderResource::TypeSampler; break;
						default:
							NSLog(@"WARNING: Unsupported shader argument type: %@", arg);
							return false;
					}
					return true;
				};

				NSLog(@"RenderPipelineReflection: %@", pipelineReflection);

                DKArray<DKShaderResource> vertexResources, fragmentResources;
                struct PipelineReflectionCopy
                {
                    NSArray<MTLArgument*> *arguments;
                    DKArray<DKShaderResource>& resources;
                } reflectionCopyItems[2] =
                {
                    { pipelineReflection.vertexArguments, vertexResources },
                    { pipelineReflection.fragmentArguments, fragmentResources },
                };

                for (auto& item : reflectionCopyItems )
                {
                    item.resources.Clear();
                    item.resources.Reserve(item.arguments.count);
                    for (MTLArgument* arg in item.arguments)
                    {
                        DKShaderResource res;
                        if (shaderResourceFromArgument(arg, res))
                        {
                            bool combined = false;
                            if (res.type == DKShaderResource::TypeSampler ||
                                res.type == DKShaderResource::TypeTexture)
                            {
                                for (DKShaderResource& r : item.resources)
                                {
                                    if (r.binding == res.binding)
                                    {
                                        if ((res.type == DKShaderResource::TypeSampler && r.type == DKShaderResource::TypeTexture) ||
                                            (res.type == DKShaderResource::TypeTexture && r.type == DKShaderResource::TypeSampler))
                                        {
                                            DKASSERT_DEBUG(r.typeInfo.buffer.alignment == res.typeInfo.buffer.alignment);
                                            DKASSERT_DEBUG(r.typeInfo.buffer.size == res.typeInfo.buffer.size);

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
                                item.resources.Add(res);
                        }
                    }

                    item.resources.ShrinkToFit();
                }
                reflection->vertexResources = std::move(vertexResources);
                reflection->fragmentResources = std::move(fragmentResources);
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
			[pipelineState autorelease];
		}
	}
	return state.SafeCast<DKRenderPipelineState>();
}

DKObject<DKComputePipelineState> GraphicsDevice::CreateComputePipeline(DKGraphicsDevice*, const DKComputePipelineDescriptor&, DKComputePipelineReflection*)
{
    return NULL;
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

#endif //#if DKGL_ENABLE_METAL
