//
//  File: GraphicsDevice.mm
//  Platform: macOS, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2019 Hongtae Kim. All rights reserved.
//

#include "../GraphicsAPI.h"
#if DKGL_ENABLE_METAL
#include <stdexcept>
#include "../../../Libs/SPIRV-Cross/src/spirv_msl.hpp"
#include "GraphicsDevice.h"
#include "CommandQueue.h"
#include "ShaderFunction.h"
#include "ShaderModule.h"
#include "ShaderBindingSet.h"
#include "RenderPipelineState.h"
#include "ComputePipelineState.h"
#include "Buffer.h"
#include "Texture.h"
#include "SamplerState.h"
#include "Event.h"
#include "Semaphore.h"
#include "Types.h"
#include "../../DKPropertySet.h"

namespace DKFramework::Private::Metal
{
	DKGraphicsDeviceInterface* CreateInterface()
	{
		return new GraphicsDevice();
	}

    DKShaderResource ShaderResourceFromMTLArgument(MTLArgument* arg, const DKArray<ResourceBinding>& bindingMap, DKShaderStage stage)
    {
        struct StructTypeData
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
                        mb.dataType = ShaderDataType(arrayType.elementType);
                        if (arrayType.elementType == MTLDataTypeStruct)
                            memberStruct = arrayType.elementStructType;
                    }
                    else
                    {
                        mb.count = 1;
                        mb.dataType = ShaderDataType(type);
                    }

                    mb.name = member.name.UTF8String;
                    mb.offset = (uint32_t)member.offset;

                    if (type == MTLDataTypeStruct)
                        memberStruct = member.structType;
                    if (memberStruct)
                    {
                        DKString typeKey = DKString(this->name).Append(".").Append(mb.name);
                        mb.typeInfoKey = StructTypeData{ typeKey, base }.operator() (memberStruct);
                    }
                    output.members.Add(mb);
                }
                base.structTypeMemberMap.Update(name, output);
                return name;
            }
        };

        DKShaderResource res = {};

        res.stages = (uint32_t)stage;
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
                DKASSERT_DESC_DEBUG(0, "Unknown access type!");
        }

        bool indexNotFound = true;
        switch (arg.type)
        {
            case MTLArgumentTypeBuffer:
                res.type = DKShaderResource::TypeBuffer;
                res.typeInfo.buffer.size = (uint32_t)arg.bufferDataSize;
                //res.typeInfo.buffer.alignment = (uint32_t)arg.bufferAlignment;
                res.typeInfo.buffer.dataType = ShaderDataType(arg.bufferDataType);
                if (arg.bufferDataType == MTLDataTypeStruct)
                {
                    res.typeInfoKey = StructTypeData{res.name, res}.operator()(arg.bufferStructType);
                }
                else
                {
                    NSLog(@"WARNING: Unsupported buffer type: %@", arg);
                    DKASSERT_DESC_DEBUG(0, "Unsupported buffer type");
                }

                for (const ResourceBinding& binding : bindingMap)
                {
                    if (binding.bufferIndex == arg.index)
                    {
                        res.set = binding.set;
                        res.binding = binding.binding;
                        indexNotFound = false;
                        break;
                    }
                }
                break;
//            case MTLArgumentTypeThreadgroupMemory:
//                res.type = DKShaderResource::TypeThreadgroupMemory;
//                break;
            case MTLArgumentTypeTexture:
                res.type = DKShaderResource::TypeTexture;
                for (const ResourceBinding& binding : bindingMap)
                {
                    if (binding.textureIndex == arg.index)
                    {
                        res.set = binding.set;
                        res.binding = binding.binding;
                        indexNotFound = false;
                        break;
                    }
                }
                break;
            case MTLArgumentTypeSampler:
                res.type = DKShaderResource::TypeSampler;
                for (const ResourceBinding& binding : bindingMap)
                {
                    if (binding.samplerIndex == arg.index)
                    {
                        res.set = binding.set;
                        res.binding = binding.binding;
                        indexNotFound = false;
                        break;
                    }
                }
                break;
            default:
                NSLog(@"WARNING: Unsupported shader argument type: %@", arg);
                DKASSERT_DESC_DEBUG(0, "Unsupported shader argument type");
        }
        DKASSERT(!indexNotFound);
        return res;
    }
    DKShaderPushConstantLayout ShaderPushConstantLayoutFromMTLArgument(MTLArgument* arg, uint32_t offset, DKShaderStage stage)
    {
        DKShaderPushConstantLayout layout = {};
        layout.name = DKString(arg.name.UTF8String);
        layout.stages = (uint32_t)stage;
        layout.offset = offset;
        layout.size = (uint32_t)arg.bufferDataSize;
        return layout;
    }
    bool CombineShaderResource(DKArray<DKShaderResource>& resources, const DKShaderResource& res)
    {
        for (DKShaderResource& r : resources)
        {
            if (res.set == r.set && res.binding == r.binding)
            {
                if ((res.type == DKShaderResource::TypeSampler && r.type == DKShaderResource::TypeTexture) ||
                    (res.type == DKShaderResource::TypeTexture && r.type == DKShaderResource::TypeSampler))
                {
                    r.type = DKShaderResource::TypeTextureSampler;
                }
                else
                {
                    DKASSERT_DESC(res.type == r.type, "Invalid resource type!");
                }
                r.stages |= res.stages;
                return true;
            }
        }
        return false;
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
                case DKShaderStage::Vertex:
                    stage = spv::ExecutionModelVertex;
                    break;
                case DKShaderStage::Fragment:
                    stage = spv::ExecutionModelFragment;
                    break;
                case DKShaderStage::Compute:
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
                        numBuffers += res.count;
                        break;
                    case DKShaderResource::TypeTexture:
                        numTextures += res.count;
                        break;
                    case DKShaderResource::TypeSampler:
                        numSamplers += res.count;
                        break;
                    case DKShaderResource::TypeTextureSampler:
                        numSamplers += res.count;
                        numTextures += res.count;
                        break;
                }
            }
            if (const DKArray<DKShaderPushConstantLayout>& layouts = shader->PushConstantBufferLayouts(); layouts.Count() > 0)
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
                            if (b1.desc_set == spirv_cross::kPushConstDescSet &&
                                b1.binding == spirv_cross::kPushConstBinding)
                            {
                                // Only one push-constant binding is allowed.
                                DKASSERT_DEBUG(bindingMap.pushConstantIndex == 0);
                                DKASSERT_DEBUG(bindingMap.pushConstantOffset == 0);

                                const DKShaderPushConstantLayout& layout = shader->PushConstantBufferLayouts().Value(0);
                                bindingMap.pushConstantIndex = b1.msl_buffer;
                                // MTLArgument doesn't have an offset, save info for later use. (pipeline-reflection)
                                bindingMap.pushConstantOffset = layout.offset;
                            }
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

DKObject<DKShaderBindingSet> GraphicsDevice::CreateShaderBindingSet(DKGraphicsDevice* dev, const DKShaderBindingSetLayout& layout)
{
    return DKOBJECT_NEW ShaderBindingSet(dev, layout.bindings, layout.bindings.Count());
}

DKObject<DKRenderPipelineState> GraphicsDevice::CreateRenderPipeline(DKGraphicsDevice* dev, const DKRenderPipelineDescriptor& desc, DKPipelineReflection* reflection)
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
        auto compareFunction = [](DKCompareFunction fn)->MTLCompareFunction
        {
            switch (fn)
            {
                case DKCompareFunctionNever:            return MTLCompareFunctionNever;
                case DKCompareFunctionLess:             return MTLCompareFunctionLess;
                case DKCompareFunctionEqual:            return MTLCompareFunctionEqual;
                case DKCompareFunctionLessEqual:        return MTLCompareFunctionLessEqual;
                case DKCompareFunctionGreater:          return MTLCompareFunctionGreater;
                case DKCompareFunctionNotEqual:         return MTLCompareFunctionNotEqual;
                case DKCompareFunctionGreaterEqual:     return MTLCompareFunctionGreaterEqual;
                case DKCompareFunctionAlways:           return MTLCompareFunctionAlways;
            }
            DKASSERT_DEBUG(0);
            return MTLCompareFunctionNever;
        };
        auto stencilOperation = [](DKStencilOperation op)->MTLStencilOperation
        {
            switch (op)
            {
                case DKStencilOperationKeep:            return MTLStencilOperationKeep;
                case DKStencilOperationZero:            return MTLStencilOperationZero;
                case DKStencilOperationReplace:         return MTLStencilOperationReplace;
                case DKStencilOperationIncrementClamp:  return MTLStencilOperationIncrementClamp;
                case DKStencilOperationDecrementClamp:  return MTLStencilOperationDecrementClamp;
                case DKStencilOperationInvert:          return MTLStencilOperationInvert;
                case DKStencilOperationIncrementWrap:   return MTLStencilOperationIncrementWrap;
                case DKStencilOperationDecrementWrap:   return MTLStencilOperationDecrementWrap;
            }
            DKASSERT_DEBUG(0);
            return MTLStencilOperationKeep;
        };
        auto setStencilDescriptor = [&](MTLStencilDescriptor* stencil, const DKStencilDescriptor& desc)
        {
            stencil.stencilFailureOperation = stencilOperation(desc.stencilFailureOperation);
            stencil.depthFailureOperation = stencilOperation(desc.depthFailOperation);
            stencil.depthStencilPassOperation = stencilOperation(desc.depthStencilPassOperation);
            stencil.stencilCompareFunction = compareFunction(desc.stencilCompareFunction);
            stencil.readMask = desc.readMask;
            stencil.writeMask = desc.writeMask;
        };

        // Create MTLDepthStencilState object.
        MTLDepthStencilDescriptor* depthStencilDescriptor = [[[MTLDepthStencilDescriptor alloc] init] autorelease];
        depthStencilDescriptor.depthCompareFunction = compareFunction(desc.depthStencilDescriptor.depthCompareFunction);
        depthStencilDescriptor.depthWriteEnabled = desc.depthStencilDescriptor.depthWriteEnabled;
        setStencilDescriptor(depthStencilDescriptor.frontFaceStencil, desc.depthStencilDescriptor.frontFaceStencil);
        setStencilDescriptor(depthStencilDescriptor.backFaceStencil, desc.depthStencilDescriptor.backFaceStencil);

        id<MTLDepthStencilState> depthStencilState = [device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
        DKASSERT_DEBUG(depthStencilState);
        [depthStencilState autorelease];

        // Create MTLRenderPipelineState object.
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

		auto vertexFormat = [](DKVertexFormat f)->MTLVertexFormat
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

		auto vertexStepFunction = [](DKVertexStepRate fn)->MTLVertexStepFunction
		{
			switch (fn)
			{
				case DKVertexStepRate::Vertex:		return MTLVertexStepFunctionPerVertex;
				case DKVertexStepRate::Instance:	return MTLVertexStepFunctionPerInstance;
			}
			DKASSERT_DESC_DEBUG(0, "Unknown value!");
			return MTLVertexStepFunctionConstant;
		};

		auto blendFactor = [](DKBlendFactor f)->MTLBlendFactor
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
		auto blendOperation = [](DKBlendOperation o)->MTLBlendOperation
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
		auto colorWriteMask = [](DKColorWriteMask mask)->MTLColorWriteMask
		{
			MTLColorWriteMask value = MTLColorWriteMaskNone;
			if (mask & DKColorWriteMaskRed)		value |= MTLColorWriteMaskRed;
			if (mask & DKColorWriteMaskGreen)	value |= MTLColorWriteMaskGreen;
			if (mask & DKColorWriteMaskBlue)	value |= MTLColorWriteMaskBlue;
			if (mask & DKColorWriteMaskAlpha)	value |= MTLColorWriteMaskAlpha;
			return value;
		};

        // setup color-attachments
		for (const DKRenderPipelineColorAttachmentDescriptor& attachment : desc.colorAttachments)
		{
			MTLRenderPipelineColorAttachmentDescriptor* colorAttachmentDesc = [descriptor.colorAttachments objectAtIndexedSubscript:attachment.index];
			colorAttachmentDesc.pixelFormat = PixelFormat(attachment.pixelFormat);
            colorAttachmentDesc.writeMask = colorWriteMask(attachment.blendState.writeMask);
			colorAttachmentDesc.blendingEnabled = attachment.blendState.enabled;
			colorAttachmentDesc.alphaBlendOperation = blendOperation(attachment.blendState.alphaBlendOperation);
			colorAttachmentDesc.rgbBlendOperation = blendOperation(attachment.blendState.rgbBlendOperation);
			colorAttachmentDesc.sourceRGBBlendFactor = blendFactor(attachment.blendState.sourceRGBBlendFactor);
			colorAttachmentDesc.sourceAlphaBlendFactor = blendFactor(attachment.blendState.sourceAlphaBlendFactor);
			colorAttachmentDesc.destinationRGBBlendFactor = blendFactor(attachment.blendState.destinationRGBBlendFactor);
			colorAttachmentDesc.destinationAlphaBlendFactor = blendFactor(attachment.blendState.destinationAlphaBlendFactor);
		}

        // setup depth attachment.
        descriptor.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
        descriptor.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
        if (DKPixelFormatIsDepthFormat(desc.depthStencilAttachmentPixelFormat))
            descriptor.depthAttachmentPixelFormat = PixelFormat(desc.depthStencilAttachmentPixelFormat);
        if (DKPixelFormatIsStencilFormat(desc.depthStencilAttachmentPixelFormat))
            descriptor.stencilAttachmentPixelFormat = PixelFormat(desc.depthStencilAttachmentPixelFormat);

        // setup vertex buffer and attributes.
        if (desc.vertexDescriptor.attributes.Count() > 0 || desc.vertexDescriptor.layouts.Count() > 0)
		{
			MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
			for (const DKVertexAttributeDescriptor& attrDesc : desc.vertexDescriptor.attributes)
			{
				MTLVertexAttributeDescriptor* attr = [vertexDescriptor.attributes objectAtIndexedSubscript:attrDesc.location];
				attr.format = vertexFormat(attrDesc.format);
				attr.offset = attrDesc.offset;
				NSUInteger bufferIndex = vertexAttributeOffset + attrDesc.bufferIndex;
				attr.bufferIndex = bufferIndex;
			}
			for (const DKVertexBufferLayoutDescriptor& layoutDesc : desc.vertexDescriptor.layouts)
			{
				NSUInteger bufferIndex = vertexAttributeOffset + layoutDesc.bufferIndex;
				MTLVertexBufferLayoutDescriptor* layout = [vertexDescriptor.layouts objectAtIndexedSubscript:bufferIndex];
				layout.stepFunction = vertexStepFunction(layoutDesc.step);
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

                DKArray<DKShaderResource> resources;
                DKArray<DKShaderAttribute> inputAttrs;
                DKArray<DKShaderPushConstantLayout> pushConstants;

                if (vertexFunction)
                    inputAttrs = vertexFunction->stageInputAttributes;

                inputAttrs.Reserve(pipelineReflection.vertexArguments.count);
                resources.Reserve(pipelineReflection.vertexArguments.count + pipelineReflection.fragmentArguments.count);
                if (pipelineReflection.vertexArguments.count)
                {
                    const ShaderModule* module = vertexFunction->module.StaticCast<ShaderModule>();
                    const StageResourceBindingMap& bindingMap = module->bindings;

                    for (MTLArgument* arg in pipelineReflection.vertexArguments)
                    {
                        if (!arg.active)
                            continue;

                        if (arg.type == MTLArgumentTypeBuffer && arg.index >= bindingMap.inputAttributeIndexOffset)
                        {
                            for (const DKShaderAttribute& attr : vertexFunction->stageInputAttributes)
                            {
                                if (attr.location == (arg.index - bindingMap.inputAttributeIndexOffset))
                                {
                                    DKShaderAttribute attr2 = attr;
                                    attr2.enabled = true;
                                    inputAttrs.Add(attr2);
                                    break;
                                }
                            }
                        }
                        else if (arg.type == MTLArgumentTypeBuffer && arg.index == bindingMap.pushConstantIndex)
                        {
                            DKShaderPushConstantLayout layout = ShaderPushConstantLayoutFromMTLArgument(arg, bindingMap.pushConstantOffset, DKShaderStage::Vertex);
                            pushConstants.Add(layout);
                        }
                        else
                        {
                            DKShaderResource res = ShaderResourceFromMTLArgument(arg, bindingMap.resourceBindings, DKShaderStage::Vertex);
                            if (!CombineShaderResource(resources, res))
                                resources.Add(res);
                        }
                    }
                }
                if (pipelineReflection.fragmentArguments.count)
                {
                    uint32_t stageMask = static_cast<uint32_t>(fragmentFunction->Stage());
                    const ShaderModule* module = fragmentFunction->module.StaticCast<ShaderModule>();
                    const StageResourceBindingMap& bindingMap = module->bindings;

                    for (MTLArgument* arg in pipelineReflection.fragmentArguments)
                    {
                        if (!arg.active)
                            continue;

                        if (arg.type == MTLArgumentTypeBuffer && arg.index == bindingMap.pushConstantIndex)
                        {
                            DKShaderPushConstantLayout layout = ShaderPushConstantLayoutFromMTLArgument(arg, bindingMap.pushConstantOffset, DKShaderStage::Fragment);
                            bool exist = false;
                            for (DKShaderPushConstantLayout& l2 : pushConstants)
                            {
                                if (l2.offset == layout.offset && l2.size == layout.size)
                                {
                                    l2.stages |= stageMask;
                                    exist = true;
                                }
                            }
                            if (!exist)
                                pushConstants.Add(layout);
                        }
                        else
                        {
                            DKShaderResource res = ShaderResourceFromMTLArgument(arg, bindingMap.resourceBindings, DKShaderStage::Fragment);
                            if (!CombineShaderResource(resources, res))
                                resources.Add(res);
                        }
                    }
                }

                resources.ShrinkToFit();
                inputAttrs.ShrinkToFit();
                pushConstants.ShrinkToFit();

                reflection->resources = std::move(resources);
                reflection->inputAttributes = std::move(inputAttrs);
                reflection->pushConstantLayouts = std::move(pushConstants);
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
			state = DKOBJECT_NEW RenderPipelineState(dev, pipelineState, depthStencilState);
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

DKObject<DKComputePipelineState> GraphicsDevice::CreateComputePipeline(DKGraphicsDevice* dev, const DKComputePipelineDescriptor& desc, DKPipelineReflection* reflection)
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

            DKArray<DKShaderResource> resources;
            DKArray<DKShaderPushConstantLayout> pushConstants;
            resources.Reserve(pipelineReflection.arguments.count);
            pushConstants.Reserve(pipelineReflection.arguments.count);

            const ShaderModule* module = func->module.StaticCast<ShaderModule>();
            const StageResourceBindingMap& bindingMap = module->bindings;

            for (MTLArgument* arg in pipelineReflection.arguments)
            {
                if (arg.type == MTLArgumentTypeBuffer && arg.index == bindingMap.pushConstantIndex)
                {
                    DKShaderPushConstantLayout layout = ShaderPushConstantLayoutFromMTLArgument(arg, bindingMap.pushConstantOffset, DKShaderStage::Compute);
                    pushConstants.Add(layout);
                }
                else
                {
                    DKShaderResource res = ShaderResourceFromMTLArgument(arg, bindingMap.resourceBindings, DKShaderStage::Compute);
                    if (!CombineShaderResource(resources, res))
                        resources.Add(res);
                }
            }

            resources.ShrinkToFit();
            pushConstants.ShrinkToFit();

            reflection->resources = std::move(resources);
            reflection->pushConstantLayouts = std::move(pushConstants);
        }

        if (error)
        {
            DKLogE("GraphicsDevice::CreateComputePipeline Error: %s", (const char*)error.localizedDescription.UTF8String);
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
			case DKCpuCacheModeReadWrite:
				options |= MTLResourceCPUCacheModeDefaultCache;
				break;
			case DKCpuCacheModeWriteOnly:
				options |= MTLResourceCPUCacheModeWriteCombined;
				break;
		}

        @autoreleasepool {
            id<MTLBuffer> buffer = [device newBufferWithLength:size options:options];
            if (buffer)
            {
                return DKOBJECT_NEW Buffer(dev, [buffer autorelease]);
            }
            else
            {
                DKLogE("GraphicsDevice::CreateBuffer Error!");
            }
        }
	}
	return NULL;
}

DKObject<DKTexture> GraphicsDevice::CreateTexture(DKGraphicsDevice* dev, const DKTextureDescriptor& desc)
{
    MTLPixelFormat pixelFormat = PixelFormat(desc.pixelFormat);
    uint32_t arrayLength = desc.arrayLength;

    if (arrayLength == 0)
    {
        DKLogE("Invalid array length!");
        return NULL;
    }
    if (pixelFormat == MTLPixelFormatInvalid)
    {
        DKLogE("Invalid pixel format!");
        return NULL;
    }

    @autoreleasepool {
        MTLTextureDescriptor* texDesc = [[[MTLTextureDescriptor alloc] init] autorelease];

        switch (desc.textureType)
        {
            case DKTexture::Type1D:
                texDesc.textureType = (arrayLength > 1) ? MTLTextureType1DArray : MTLTextureType1D;
                break;
            case DKTexture::Type2D:
                texDesc.textureType = (arrayLength > 1) ? MTLTextureType2DArray : MTLTextureType2D;
                break;
            case DKTexture::TypeCube:
                texDesc.textureType = (arrayLength > 1) ? MTLTextureTypeCubeArray : MTLTextureTypeCube;
                break;
            case DKTexture::Type3D:
                texDesc.textureType = MTLTextureType3D;
                break;
        }
        texDesc.pixelFormat = pixelFormat;
        texDesc.width = desc.width;
        texDesc.height = desc.height;
        texDesc.depth = desc.depth;
        texDesc.mipmapLevelCount = desc.mipmapLevels;
        texDesc.sampleCount = desc.sampleCount;
        texDesc.arrayLength = arrayLength;
        //texDesc.resourceOptions = MTLResourceStorageModePrivate;
        texDesc.cpuCacheMode = MTLCPUCacheModeDefaultCache;
        texDesc.storageMode = MTLStorageModePrivate;
        texDesc.allowGPUOptimizedContents = YES;

        texDesc.usage = 0; // MTLTextureUsageUnknown;
        if (desc.usage & (DKTexture::UsageSampled | DKTexture::UsageShaderRead))
            texDesc.usage |= MTLTextureUsageShaderRead;
        if (desc.usage & (DKTexture::UsageStorage | DKTexture::UsageShaderWrite))
            texDesc.usage |= MTLTextureUsageShaderWrite;
        if (desc.usage & DKTexture::UsageRenderTarget)
            texDesc.usage |= MTLTextureUsageRenderTarget;
        if (desc.usage & DKTexture::UsagePixelFormatView)
            texDesc.usage |= MTLTextureUsagePixelFormatView; // view with a different pixel format.

        id<MTLTexture> texture = [device newTextureWithDescriptor:texDesc];
        if (texture)
        {
            return DKOBJECT_NEW Texture(dev, [texture autorelease]);
        }
        else
        {
            DKLogE("GraphicsDevice::CreateTexture Error!");
        }
    }
    return NULL;
}

DKObject<DKSamplerState> GraphicsDevice::CreateSamplerState(DKGraphicsDevice* dev, const DKSamplerDescriptor& desc)
{
    auto addressMode = [](DKSamplerDescriptor::AddressMode m)->MTLSamplerAddressMode
    {
        switch (m)
        {
            case DKSamplerDescriptor::AddressModeClampToEdge:
                return MTLSamplerAddressModeClampToEdge;
            case DKSamplerDescriptor::AddressModeRepeat:
                return MTLSamplerAddressModeRepeat;
            case DKSamplerDescriptor::AddressModeMirrorRepeat:
                return MTLSamplerAddressModeMirrorRepeat;
            case DKSamplerDescriptor::AddressModeClampToZero:
                return MTLSamplerAddressModeClampToZero;
        }
        return MTLSamplerAddressModeClampToEdge;
    };
    auto minMagFilter = [](DKSamplerDescriptor::MinMagFilter f)->MTLSamplerMinMagFilter
    {
        switch (f)
        {
            case DKSamplerDescriptor::MinMagFilterNearest:
                return MTLSamplerMinMagFilterNearest;
            case DKSamplerDescriptor::MinMagFilterLinear:
                return MTLSamplerMinMagFilterLinear;
        }
        return MTLSamplerMinMagFilterNearest;
    };
    auto mipFilter = [](DKSamplerDescriptor::MipFilter f)->MTLSamplerMipFilter
    {
        switch (f)
        {
            case DKSamplerDescriptor::MipFilterNotMipmapped:
                return MTLSamplerMipFilterNotMipmapped;
            case DKSamplerDescriptor::MipFilterNearest:
                return MTLSamplerMipFilterNearest;
            case DKSamplerDescriptor::MipFilterLinear:
                return MTLSamplerMipFilterLinear;
        }
        return MTLSamplerMipFilterNotMipmapped;
    };
    auto compareFunction = [](DKCompareFunction fn)->MTLCompareFunction
    {
        switch (fn)
        {
            case DKCompareFunctionNever:            return MTLCompareFunctionNever;
            case DKCompareFunctionLess:             return MTLCompareFunctionLess;
            case DKCompareFunctionEqual:            return MTLCompareFunctionEqual;
            case DKCompareFunctionLessEqual:        return MTLCompareFunctionLessEqual;
            case DKCompareFunctionGreater:          return MTLCompareFunctionGreater;
            case DKCompareFunctionNotEqual:         return MTLCompareFunctionNotEqual;
            case DKCompareFunctionGreaterEqual:     return MTLCompareFunctionGreaterEqual;
            case DKCompareFunctionAlways:           return MTLCompareFunctionAlways;
        }
        return MTLCompareFunctionNever;
    };

    @autoreleasepool {
        MTLSamplerDescriptor* samplerDesc = [[[MTLSamplerDescriptor alloc] init] autorelease];
        samplerDesc.sAddressMode = addressMode(desc.addressModeU);
        samplerDesc.tAddressMode = addressMode(desc.addressModeV);
        samplerDesc.rAddressMode = addressMode(desc.addressModeW);
        samplerDesc.minFilter = minMagFilter(desc.minFilter);
        samplerDesc.magFilter = minMagFilter(desc.magFilter);
        samplerDesc.mipFilter = mipFilter(desc.mipFilter);
        samplerDesc.lodMinClamp = desc.minLod;
        samplerDesc.lodMaxClamp = desc.maxLod;
        samplerDesc.maxAnisotropy = desc.maxAnisotropy;
        samplerDesc.normalizedCoordinates = desc.normalizedCoordinates;
        if (!desc.normalizedCoordinates)
        {
            samplerDesc.magFilter = samplerDesc.minFilter;
            samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
            samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
            samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
            samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
            samplerDesc.maxAnisotropy = 1;
        }
        samplerDesc.compareFunction = compareFunction(desc.compareFunction);

        id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:samplerDesc];
        if (sampler)
        {
            return DKOBJECT_NEW SamplerState(dev, [sampler autorelease]);
        }
        else
        {
            DKLogE("GraphicsDevice::CreateSamplerState Error!");
        }
    }

    return NULL;
}

DKObject<DKGpuEvent> GraphicsDevice::CreateEvent(DKGraphicsDevice* dev)
{
    @autoreleasepool {
        id<MTLEvent> event = [device newEvent];
        if (event)
        {
            return DKOBJECT_NEW Event(dev, [event autorelease]);
        }
        else
        {
            DKLogE("GraphicsDevice::CreateEvent Error!");
        }
    }
    return NULL;
}

DKObject<DKGpuSemaphore> GraphicsDevice::CreateSemaphore(DKGraphicsDevice* dev)
{
    @autoreleasepool {
        id<MTLEvent> event = [device newEvent];
        if (event)
        {
            return DKOBJECT_NEW Semaphore(dev, [event autorelease]);
        }
        else
        {
            DKLogE("GraphicsDevice::CreateEvent Error!");
        }
    }
    return NULL;
}

#endif //#if DKGL_ENABLE_METAL
