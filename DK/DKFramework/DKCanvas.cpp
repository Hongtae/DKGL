//
//  File: DKCanvas.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2020 Hongtae Kim. All rights reserved.
//

#include "DKCanvas.h"
#include "DKMath.h"
#include "DKAffineTransform2.h"
#include "DKCommandQueue.h"
#include "DKCommandBuffer.h"
#include "DKShader.h"
#include "DKShaderModule.h"
#include "DKShaderFunction.h"
#include "DKGraphicsDevice.h"


namespace DKFramework::Private
{
    /* //vertex shader
    #version 450

    layout (location=0) in vec2 position;
    layout (location=1) in vec2 texcoord;
    layout (location=2) in vec4 color;

    layout (location=0) out vec2 outPosition;
    layout (location=1) out vec2 outTexcoord;
    layout (location=2) out vec4 outColor;

    void main()
    {
        outPosition = position;
        outTexcoord = texcoord;
        outColor = color;

        gl_Position = vec4(position, 0, 1);
    }
    */
    const char* vsSpvCompressedBase64 =
        "XQAAAATABAAAAAAAAAABgJdesntsONM6MGcxLKsQZjMqZyK9s+I5F9rVJqCzj9BrRuMFcP"
        "pPjJTufO2GnekV4S/IzcItw0iVP+X5oQW6csvuo5GS2Ihrgt9U/0k7XYsDg2hWLwWyNRd2"
        "i5M7LA80mBfTGUL9qSH5IFJp2HmzlKr7mK0KjEZMHymQ2d17sbqWysevCH7QwIrKiTJ6X6"
        "7R3nygYevfIGd9/G1/mYy9LVUDz8qDF5yRF1XvNLB2eRYmsICZnTd8UzfX6FkcWANoWXkm"
        "8lJzw7/smwtoZ0shMDC+fR5ikvCX052fx7xhfmsNbuFO6YHWvgq/Mj+UA3LCin/xaDjXR2"
        "19dyOxOJlxm67UUTY2NJuf8NA6EK7djXgF1hRy2FRcwVTgmX8iY8VnvCXs+Q4ITlnxTBjR"
        "aKUUDxfL2Jb9nN2WefrCzIjnYM/PRYF/aCv7qXfdLT56ToL+MPguij1J7zOPSJiPi//03P"
        "JkFAwhjUgkAo2wQdwPtFLRCUvtLgfG7JWDflgWmrwEA/jsZeeg6dkiwpTC0ld0P6zuHAA="
        ;

    /* //fragment-shader: vertex color
    #version 450

    layout (location=0) in vec2 position;
    layout (location=1) in vec2 texcoord;
    layout (location=2) in vec4 color;

    layout (location=0) out vec4 outFragColor;

    void main(void)
    {
        outFragColor = color;
    }
    */
    const char* fsSpvCompressedBase64 =
        "XQAAAAQMAgAAAAAAAAABgJdesntsONM6MGcrJRJFH8eaWFhPhF/JgkVaMKDLBIehEkszc0"
        "5hwuLCi2c2uZq7XW/7AY0KbL2Rwo/dGk5XbL0h8jqJ+yz0XcClGDo6FfdY3mTBUeJufqjr"
        "3Lkng6/V1JJBTLD+nnardxsTuWH+d3kqBsEOvJvHvaVvd+3ubsYeulURrEldB0aem8ZuH4"
        "ooN+cYIQNr5N/zMd3m2O7cPoMu2fZGV9swxtT3y+umjWnu2em3LxwYLtSFak8FfL5JUx14"
        "v+OoioDBuElhe+DpDeo/UDLsl5HSSjUdWAeWXFGcRgA="
        ;
    /* //fragment-shader: vertex color, uniform texture
    #version 450

    layout (binding=0) uniform sampler2D image;

    layout (location=0) in vec2 position;
    layout (location=1) in vec2 texcoord;
    layout (location=2) in vec4 color;

    layout (location=0) out vec4 outFragColor;

    void main()
    {
        outFragColor = texture(image, texcoord) * color;
    }
    */
    const char* fsTextureSpvCompressedBase64 =
        "XQAAAATUAgAAAAAAAAABgJdesntsONM6MGctmKIB2HaeuzmgkxfX256KE1HOZIhrusCJ00"
        "Nn8k7Pi3d42hliNd0QudCMu4Bu1dAmK7j4zRV2qYZl5l8PfDaM/mz2/HgrqvHwcD7o5Eli"
        "3bGZ9xrelsWEyh6qGbUpnDohd44jZu9o/zr5ARxpz2vf4dN7QWpcWAq55iqfP/J8AlWOfV"
        "kGj7T1eUc2PTgf+YHYzY0iA+z6RLnuO6xN0PAKlU1+YizwCgEyOQBziBKKn3TdyE3n1YhA"
        "vl5+QSCCH1dDw00JUPw1MpYo5i8gMBYBWOgtTBfv8egGietEE/LzjZVup9AMgiqPl2WMif"
        "ooq/JVNDsy1RTy3hGj5UMrgWOqYseDd+b6yUjCdu3Fb0avP7HF9ZkeaUYA"
        ;
    /* //fragment-shader: vertex color ellipse
    #version 450

    layout (push_constant) uniform Ellipse
    {
        vec2 outerRadiusSqInv; // vec2(1/A^2, 1/B^2) where X^2 / A^2 + Y^2 / B^2 = 1
        vec2 innerRadiusSqInv;
        vec2 center;	// center of ellipse
    } ellipse;

    layout (location=0) in vec2 position;
    layout (location=1) in vec2 texcoord;
    layout (location=2) in vec4 color;

    layout (location=0) out vec4 outFragColor;

    void main()
    {
        vec2 vl = position - ellipse.center;
        float form = vl.x * vl.x * ellipse.outerRadiusSqInv.x + vl.y * vl.y * ellipse.outerRadiusSqInv.y;
        if (form > 1.0)
            discard;
        outFragColor = color;
    }
    */
    const char* fsEllipseSpvCompressedBase64 = 
        "XQAAAAToBQAAAAAAAAABgJdesntsONM6MGc3MEApKl6gOKaOiphx2TUpKi7V15R/VQuLn+"
        "KjvLo9ofgVgb40F/wOpzAAj5D4LefT647kouue3zFWqIW7pUYAIv+ki1qJ39xQTICjx0Ip"
        "TOBta/ZLW7g8AlzwKsoJyNt9ksrWefdG7vvwgcYUHAAOTnUw3CR5BjQqVd5+SNoBepNDaw"
        "HTdjdm0jw4Rx/oZGql67aEw4IBOS9WrFuGJbwil1gV5b4Sz5ke9hx3jGHkGBj+yKhKzdJn"
        "vnpo4hUWhgDjqhY/HBtKqk9mphpApJ/wouwoF2rx2ys5Il+StupOCeWqHWhiGOnItAV/Ja"
        "LcElcCMwZ56wHB6JaomnMF+vbO5B6930KIxxRz+9SCXakeHeYoUsVa7qIvUH/BIWwbm/j+"
        "RjIA7dMEn+gj/KP1KxBcnxGxNXecwnYsRNgWE3qwHqr7ampT9FJUscECI6DB9dp/gYk8+g"
        "5WNhsfkW0AomVWCS8eEFVHeQxyQMslUWG9oYpAnqO1BQ2+ccZal6ma7rks2lljmpD0IcN+"
        "ZCrToC10MUbUv5tpzj1/2MEE68eocMWYoqD8+RI5ietq1xqbfhOZASRNh/msOnAWJx+PjP"
        "DyR6jEEl1z8A5Br6u3SJWSMDoUm6hNoPlHvT/Npdz0eRAR5pNTh24IAA==";

    /* //fragment-shader: vertex color ellipse with inner hole
    #version 450

    layout (push_constant) uniform Ellipse
    {
        vec2 outerRadiusSqInv; // vec2(1/A^2, 1/B^2) where X^2 / A^2 + Y^2 / B^2 = 1
        vec2 innerRadiusSqInv;
        vec2 center;	// center of ellipse
    } ellipse;

    layout (location=0) in vec2 position;
    layout (location=1) in vec2 texcoord;
    layout (location=2) in vec4 color;

    layout (location=0) out vec4 outFragColor;

    void main()
    {
        vec2 vl = position - ellipse.center;
        vec2 vl2 = vl * vl;

        vec2 f1 = vl2 * ellipse.outerRadiusSqInv;
        if (f1.x + f1.y > 1.0)
            discard;

        vec2 f2 = vl2 * ellipse.innerRadiusSqInv;
        if (f2.x + f2.y < 1.0)
            discard;

        outFragColor = color;
    }
    */
    const char* fsEllipseHoleSpvCompressedBase64 = 
        "XQAAAASgBgAAAAAAAAABgJdesntsONM6MGc5Nh9bzUJKYSZtr/HmTOO3q0we73pfWtE2WY"
        "tgJOm/qvdgX3ek2zUGzNqWjozNjWHktaqHPDu6k96ZB/XsYBiGM+lW/BlRFL+Bw4zwEmv5"
        "ob9qzfy8s4/0oypdRTfaQdrdZa1hNMF1KFPv4q3X/d1EnVNSsUok/1P+7KajhIpZ7U7yiE"
        "1KzyACzpu+doxSRNOsN7hsdxzu9Wklpu0Jmxr78Mtckr/69i9twGwu3ApmzqjVA26B04b4"
        "fxC1SIorx86O7TcifOrljQoJuic91J2SE/hUCLfLQ1UxO+4u7f6wLMD/X1wxuapWYfihXj"
        "Z+U0d7620TOEp/vV0Jy6YLGFqjoaRNPekah5h73fZ3Omjzb9KHazbiDBt8+iTglrqhzOQD"
        "0QXg3FdPlCmKmUL5tABZXsSjudL9fw9NzOMQfGfkAZ4KeYb9r4eCRYw7OTPsD/VsWZcaKx"
        "F/R6mZU4vs1aGNXQjxcO4cFTzdOnVyJSrXTVR7X4miQ2M0ItJIkpOubcVwgGu6NE8u0MAy"
        "4xmWpJzu8g9fpQsIANr8FGHfmmF5lwvbWUWdFRB4/taoaoN8cfNvDjyueUXS5R48r3YzE6"
        "bvQJNA9pqyxJDmf+t+ViKu70ZaTjknZEnGbA3yx7zCIps1PaJBnCuXdmqcu4S4wWm+HruG"
        "h475/OcTSzIX8IHapndp1AhJhdwlgLIA";

    /* //fragment-shader: vertex color, uniform texture ellipse
    #version 450

    layout (binding=0) uniform sampler2D image;

    layout (push_constant) uniform Ellipse
    {
        vec2 outerRadiusSqInv; // vec2(1/A^2, 1/B^2) where X^2 / A^2 + Y^2 / B^2 = 1
        vec2 innerRadiusSqInv;
        vec2 center;	// center of ellipse
    } ellipse;

    layout (location=0) in vec2 position;
    layout (location=1) in vec2 texcoord;
    layout (location=2) in vec4 color;

    layout (location=0) out vec4 outFragColor;

    void main()
    {
        vec2 vl = position - ellipse.center;
        float form = vl.x * vl.x * ellipse.outerRadiusSqInv.x + vl.y * vl.y * ellipse.outerRadiusSqInv.y;
        if (form > 1.0)
            discard;
        outFragColor = texture(image, texcoord) * color;
    }
    */
    const char* fsTextureEllipseSpvCompressedBase64 =
        "XQAAAASwBgAAAAAAAAABgJdesntsONM6MGc5AxVbzUJKYSZtr/HmTOO3q0we73pfWtE2WY"
        "tgJOm/qvdgX3ek2zUGzNqWjozNjWHktMZeH4El3hNjzurD8LARAo6TjbRD5m2db3NGyw0k"
        "c2BbpBrZgWt+Di2oFNnhKiwzh6PTKymMty1tkLzVNwNnZt5tJQGcDWVkuqNoitvYyifKWw"
        "wC4DAn67n+BCqTeGxmNHDEndOCe7wlEOBYn7JSga1eLboG0K032YXIsoyCReMn97dAKxK5"
        "XuAETMLLAmuUrFHD7HIRVYx2oVtF67+xNNB5kJ3v5qH2lkrDddTYMsnX5HI33mnkJPnl6r"
        "8Lqi29gyk7cmcVC1C7g5dLoAp8mq9B3C9hc6FfZsRbsa9f8eA67srLYLEkPI5iPXbqSYfM"
        "cI6rGijdCqiAnZRK07d/6aTzEafRaEUwKx1MfmMQzo52UeQYa+fXjiYgTkCgR3ZJpSHMY5"
        "MDAN3Xt5ebfelwmtAYP7hDkih2Ss2qC5BqvxyoDCAJpXxZZk0cOrZ6E90uddUC6rgvG72K"
        "WCRoCRg6ihxBQp6LktPAJgK5iQFVzH+Nn0k+emqPFRm+qcuFv+RhEIY2o7iHDp0adiqWSL"
        "1tzLPVg2onjvj9PKwyp66IbNv719sI4C2swl0ldiwNz2FmPqjJyulPdUObpeMNiMsFrL+8"
        "qpzpEnskU9KoeDTqyHeEuy7X98iiH8su3EBP+62us3kT63I98zqVr64eAA==";

    /* //fragment-shader: vertex color, uniform texture alpha (r8)
    #version 450

    layout (binding=0) uniform sampler2D image;

    layout (location=0) in vec2 position;
    layout (location=1) in vec2 texcoord;
    layout (location=2) in vec4 color;

    layout (location=0) out vec4 outFragColor;

    void main(void)
    {
        outFragColor = vec4(color.rgb, texture(image, texcoord).r * color.a);
    }
    */
    const char* fsAlphaTextureSpvCompressedBase64 =
        "XQAAAATUAwAAAAAAAAABgJdesntsONM6MGcxcoMSBeaqbZCSM/1LXfg6AGNgofxumxOEb8"
        "b615/PtnqMfm/XWlm6+YN+BmCRldMPqKzeHyGTCvtXjSGTE/R59f4ITWJxQxQ2jpntrZGM"
        "+h2a2cA1ClEKyd6JekmlZv1J2/H56RGAYWG91/hL5cHNzd1V85upoxe2/2bUWjbCtzvULQ"
        "nY3L6Nxy7SJx2le0r6QBFVnrTnu5xrChvOHTmN6r+Jnt12HzXZ2bNgFrS8SEFA30Sbv/Bm"
        "9hv2EVePLpjkVC00a6II2O11Sb+VPBeQp+P+rwS9xKGxTg7niRc7jkfxfGioU3YyD5BBOH"
        "hcuk4P5w9jdcDpLI+7nws+5Tp4IuZmqM9qEqFnCqtVfnnMwDuTUJmMWk6sVkWQOVIqqgMa"
        "EueCzUTn3BOQ65MZcfYCP9dvHl0Aw+DLHG3MKJ4OY7EBBxVGX5z0lE0gM1fvyWib1a/Vnk"
        "A6r84A"
        ;


    enum class CanvasShaderIndex : uint8_t {
        DrawVertexColor = 0,
        DrawVertexColorTexture,
        DrawVertexColorEllipse,
        DrawVertexColorEllipseHole,
        DrawVertexColorTexturedEllipse,
        DrawVertexColorAlphaTexture,
    };
#pragma pack(push, 1)
    struct CanvasPipelineDescriptor
    {
        // shader
        CanvasShaderIndex shader;

        // render target format
        DKPixelFormat colorFormat = DKPixelFormat::Invalid;
        DKPixelFormat depthFormat = DKPixelFormat::Invalid;

        // render target blend factor
        DKBlendFactor sourceRGBBlendFactor = DKBlendFactor::One;
        DKBlendFactor sourceAlphaBlendFactor = DKBlendFactor::One;
        DKBlendFactor destinationRGBBlendFactor = DKBlendFactor::Zero;
        DKBlendFactor destinationAlphaBlendFactor = DKBlendFactor::Zero;
        DKBlendOperation rgbBlendOperation = DKBlendOperation::Add;
        DKBlendOperation alphaBlendOperation = DKBlendOperation::Add;
        DKColorWriteMask writeMask = DKColorWriteMaskAll;

        void SetBlendState(const DKBlendState& state)
        {
            if (state.enabled)
            {
                sourceRGBBlendFactor = state.sourceRGBBlendFactor;
                sourceAlphaBlendFactor = state.sourceAlphaBlendFactor;
                destinationRGBBlendFactor = state.destinationRGBBlendFactor;
                destinationAlphaBlendFactor = state.destinationAlphaBlendFactor;
                rgbBlendOperation = state.rgbBlendOperation;
                alphaBlendOperation = state.alphaBlendOperation;
                writeMask = state.writeMask;
            }
            else
            {
                sourceRGBBlendFactor = DKBlendFactor::One;
                sourceAlphaBlendFactor = DKBlendFactor::One;
                destinationRGBBlendFactor = DKBlendFactor::Zero;
                destinationAlphaBlendFactor = DKBlendFactor::Zero;
                rgbBlendOperation = DKBlendOperation::Add;
                alphaBlendOperation = DKBlendOperation::Add;
                writeMask = DKColorWriteMaskAll;
            }
        }
        DKBlendState BlendState() const
        {
            DKBlendState state = {};
            state.sourceAlphaBlendFactor = sourceRGBBlendFactor;
            state.sourceAlphaBlendFactor = sourceAlphaBlendFactor;
            state.destinationRGBBlendFactor = destinationRGBBlendFactor;
            state.destinationAlphaBlendFactor = destinationAlphaBlendFactor;
            state.rgbBlendOperation = rgbBlendOperation;
            state.alphaBlendOperation = alphaBlendOperation;
            state.writeMask = writeMask;
            if (sourceRGBBlendFactor == DKBlendFactor::One &&
                sourceAlphaBlendFactor == DKBlendFactor::One &&
                destinationRGBBlendFactor == DKBlendFactor::Zero &&
                destinationAlphaBlendFactor == DKBlendFactor::Zero &&
                rgbBlendOperation == DKBlendOperation::Add &&
                alphaBlendOperation == DKBlendOperation::Add &&
                writeMask == DKColorWriteMaskAll)
                state.enabled = false;
            else
                state.enabled = true;
            return state;
        }

        struct MapComparator
        {
            int operator () (const CanvasPipelineDescriptor& lhs,
                             const CanvasPipelineDescriptor& rhs) const
            {
                constexpr auto n = sizeof(CanvasPipelineDescriptor) / sizeof(int32_t);
                static_assert(n > 0);

                const int32_t* n1 = reinterpret_cast<const int32_t*>(&lhs);
                const int32_t* n2 = reinterpret_cast<const int32_t*>(&rhs);
                for (uint32_t i = 0; i < n; ++i)
                {
                    int32_t d = (*n1) - (*n2);
                    if (d != 0)
                        return d;
                    n1++;
                    n2++;
                }
                return 0;
            }
        };
        int32_t padding = 0;
    };
#pragma pack(pop)

#pragma pack(push, 4)
    struct EllipseUniformPushConstant
    {
        // vec2(1/A^2,1/B^2) value from formula X^2 / A^2 + Y^2 / B^2 = 1
        DKVector2 outerRadiusSqInv; // outer inversed squared radius
        DKVector2 innerRadiusSqInv; // inner inversed squared radius
        DKVector2 center;           // center of ellipse
    };
#pragma pack(pop)
    using TexturedVertex = DKCanvas::TexturedVertex;

    class CanvasPipelineStates : public DKUnknown
    {
        DKObject<DKShaderFunction> vertexFunction;
        DKArray<DKObject<DKShaderFunction>> fragmentFunctions;

        DKMap<CanvasPipelineDescriptor,
            DKObject<DKRenderPipelineState>,
            CanvasPipelineDescriptor::MapComparator> pipelineStates;

        DKObject<DKGraphicsDevice> device;
        static DKLock& Lock()
        {
            static DKLock lock;
            return lock;
        }

    public:
        DKRenderPipelineState* StateForDescriptor(const CanvasPipelineDescriptor& desc)
        {
            DKCriticalSection guard(Lock());

            auto p = pipelineStates.Find(desc);
            if (p)
                return p->value;

            DKRenderPipelineDescriptor pipelineDescriptor;
            pipelineDescriptor.vertexFunction = vertexFunction;
            pipelineDescriptor.fragmentFunction = fragmentFunctions[(uint8_t)desc.shader];
            pipelineDescriptor.colorAttachments.Resize(1);
            pipelineDescriptor.colorAttachments.Value(0).pixelFormat = desc.colorFormat;
            pipelineDescriptor.colorAttachments.Value(0).blendState = desc.BlendState();
            pipelineDescriptor.depthStencilAttachmentPixelFormat = desc.depthFormat;
            pipelineDescriptor.depthStencilDescriptor.depthCompareFunction = DKCompareFunctionAlways;
            pipelineDescriptor.depthStencilDescriptor.depthWriteEnabled = false;
            pipelineDescriptor.vertexDescriptor.attributes = {
                { DKVertexFormat::Float2, 0, 0, 0 },
                { DKVertexFormat::Float2, offsetof(TexturedVertex, texcoord), 0, 1 },
                { DKVertexFormat::Float4, offsetof(TexturedVertex, color), 0, 2 },
            };
            pipelineDescriptor.vertexDescriptor.layouts = {
                { DKVertexStepRate::Vertex, sizeof(TexturedVertex), 0 },
            };
            pipelineDescriptor.primitiveTopology = DKPrimitiveType::Triangle;
            pipelineDescriptor.frontFace = DKFrontFace::CCW;
            pipelineDescriptor.triangleFillMode = DKTriangleFillMode::Fill;
            pipelineDescriptor.depthClipMode = DKDepthClipMode::Clip;
            pipelineDescriptor.cullMode = DKCullMode::None;
            pipelineDescriptor.rasterizationEnabled = true;

            DKObject<DKRenderPipelineState> state = device->CreateRenderPipeline(pipelineDescriptor, nullptr);
            if (state)
            {
                pipelineStates.Update(desc, state);
            }
            return state;
        }

        // default texture (set:0, binding:0)
        DKObject<DKShaderBindingSet> defaultBindingSet;
        DKObject<DKSamplerState> defaultSampler;

        static DKObject<CanvasPipelineStates> SharedInstance(DKGraphicsDevice* device)
        {            
            static DKObject<CanvasPipelineStates>::Ref canvasPipelineStatesWeakRef;
            DKObject<CanvasPipelineStates> cps = canvasPipelineStatesWeakRef;
            if (cps == nullptr && device)
            {
                DKCriticalSection guard(Lock());
                cps = canvasPipelineStatesWeakRef;
                if (cps)
                    return cps;

                auto shaderFunctionFromCompressedB64Spv = [device](const DKStringU8& b64encoded)->DKObject<DKShaderFunction>
                {
                    DKObject<DKBuffer> compressedBuffer = DKBuffer::Base64Decode(b64encoded);
                    if (compressedBuffer)
                    {
                        DKObject<DKBuffer> data = compressedBuffer->Decompress();
                        if (data)
                        {
                            DKObject<DKShader> shader = DKOBJECT_NEW DKShader(data);
                            if (shader->Validate())
                            {
                                DKObject<DKShaderModule> module = device->CreateShaderModule(shader);
                                if (module)
                                {
                                    return module->CreateFunction(module->FunctionNames().Value(0));
                                }
                            }
                        }
                    }
                    return nullptr;
                };

                // setup 
                do
                {
                    DKObject<CanvasPipelineStates> state = DKOBJECT_NEW CanvasPipelineStates{};
                    state->device = device;

                    if (state->vertexFunction = shaderFunctionFromCompressedB64Spv(vsSpvCompressedBase64);
                        state->vertexFunction == nullptr)
                        break;

                    auto fragmentShaders = {
                        fsSpvCompressedBase64,
                        fsTextureSpvCompressedBase64,
                        fsEllipseSpvCompressedBase64,
                        fsEllipseHoleSpvCompressedBase64,
                        fsTextureEllipseSpvCompressedBase64,
                        fsAlphaTextureSpvCompressedBase64,
                    };
                    state->fragmentFunctions.Reserve(std::size(fragmentShaders));
                    for (auto fs : fragmentShaders)
                    {
                        DKObject<DKShaderFunction> fn = shaderFunctionFromCompressedB64Spv(fs);
                        if (fn == nullptr)
                            break;
                        state->fragmentFunctions.Add(fn);
                    }
                    if (state->fragmentFunctions.Count() != std::size(fragmentShaders))
                        break;

                    state->fragmentFunctions.ShrinkToFit();
                    DKShaderBindingSetLayout layout =
                    {
                        {
                            DKShaderBinding
                            {
                                0,
                                DKShader::DescriptorTypeTextureSampler,
                                1, nullptr
                            },
                        } // bindings
                    };
                    state->defaultBindingSet = device->CreateShaderBindingSet(layout);
                    if (state->defaultBindingSet == nullptr)
                        break;

                    DKSamplerDescriptor samplerDesc = {};
                    state->defaultSampler = device->CreateSamplerState(samplerDesc);
                    if (state->defaultSampler == nullptr)
                        break;

                    cps = state;
                    canvasPipelineStatesWeakRef = cps;
                } while (0);
            }
            return cps;
        }
    };
} // namespace DKFramework::Private

using namespace DKFramework;
using namespace DKFramework::Private;

const float DKCanvas::minimumScaleFactor = 0.000001f;

DKCanvas::DKCanvas(DKCommandBuffer* cb, DKTexture* rt)
    : commandBuffer(cb)
    , renderTarget(rt)
    , viewport(0, 0, 1, 1)
    , contentBounds(0, 0, 1, 1)
    , contentTransform(DKMatrix3::identity)
    , screenTransform(DKMatrix3::identity)
    , deviceOrientation(DKMatrix3::identity)
    , drawable(false)
{
    if (commandBuffer)
    {
        DKObject<CanvasPipelineStates> states = CanvasPipelineStates::SharedInstance(cb->Device());
        if (states)
        {
            pipelineStates = states.SafeCast<DKUnknown>();

            if (pipelineStates &&
                renderTarget &&
                DKPixelFormatIsColorFormat(renderTarget->PixelFormat()))
            {
                drawable = true;
            }
        }
    }

    UpdateTransform();
}

DKCanvas::~DKCanvas()
{
}

const DKRect& DKCanvas::Viewport() const
{
    return viewport;
}

void DKCanvas::SetViewport(const DKRect& rc)
{
    viewport = rc;
    this->UpdateTransform();
}

const DKRect& DKCanvas::ContentBounds() const
{
    return contentBounds;
}

void DKCanvas::SetContentBounds(const DKRect& rc)
{
    DKASSERT_DEBUG(rc.size.width > 0.0 && rc.size.height > 0.0);

    this->contentBounds.origin = rc.origin;
    this->contentBounds.size.width = Max(rc.size.width, minimumScaleFactor);
    this->contentBounds.size.height = Max(rc.size.height, minimumScaleFactor);

    this->UpdateTransform();
}

const DKMatrix3& DKCanvas::ContentTransform() const
{
    return contentTransform;
}

void DKCanvas::SetContentTransform(const DKMatrix3& tm)
{
    this->contentTransform = tm;
    this->UpdateTransform();
}

const DKMatrix3& DKCanvas::DeviceOrientation() const
{
    return deviceOrientation;
}

void DKCanvas::SetDeviceOrientation(const DKMatrix3& tm)
{
    this->deviceOrientation = tm;
    this->UpdateTransform();
}

void DKCanvas::UpdateTransform()
{
    const DKPoint& viewportOffset = this->viewport.origin;
    const DKPoint& contentOffset = this->contentBounds.origin;
    const DKSize& contentScale = this->contentBounds.size;

    DKASSERT_DEBUG(contentScale.width > 0.0 && contentScale.height > 0.0);

    DKMatrix3 targetOrient = this->DeviceOrientation();

    DKMatrix3 offset = DKAffineTransform2(-contentOffset.Vector()).Matrix3();
    DKLinearTransform2 s(1.0f / contentScale.width, 1.0f / contentScale.height);

    // transform to screen viewport space.
    DKMatrix3 normalize = DKMatrix3(DKVector3(2.0f, 0.0f, 0.0f),
                                    DKVector3(0.0f, -2.0f, 0.0f),
                                    DKVector3(-1.0f, 1.0f, 1.0f));

    this->screenTransform = this->contentTransform * offset * DKAffineTransform2(s).Multiply(targetOrient).Matrix3() * normalize;
}

void DKCanvas::Clear(const DKColor& color)
{
    DKRenderPassColorAttachmentDescriptor colorAttachmentDesc =
    {
        renderTarget,
        0,
        DKRenderPassAttachmentDescriptor::LoadActionClear,
        DKRenderPassAttachmentDescriptor::StoreActionStore,
        color
    };
    DKRenderPassDepthStencilAttachmentDescriptor depthAttachmentDesc = {};

    DKRenderPassDescriptor desc =
    {
        { colorAttachmentDesc, },
        depthAttachmentDesc,
        0
    };

    DKObject<DKRenderCommandEncoder> encoder = commandBuffer->CreateRenderCommandEncoder(desc);
    if (encoder)
        encoder->EndEncoding();
}

void DKCanvas::DrawLines(const DKPoint* points,
                         size_t numPoints,
                         const DKColor& color,
                         const DKBlendState& blendState,
                         float lineWidth)
{
    if (numPoints > 1 && lineWidth > minimumScaleFactor)
    {
        const float halfWidth = lineWidth * 0.5;

        DKArray<DKPoint> vertices; // triangle strip vertices
        vertices.Reserve(numPoints * 3);

        for (int i = 0; (i + 1) < numPoints; )
        {
            DKVector2 v0 = points[i++].Vector();
            DKVector2 v1 = points[i++].Vector();

            DKVector2 line = v1 - v0;
            float length = line.Length();
            if (length > minimumScaleFactor)
            {
                line.Normalize();
                float cosR = line.x;
                float sinR = line.y;

                const DKMatrix2 rotate(cosR, sinR, -sinR, cosR);

                DKVector2 box[4] = {
                    { 0.0f, halfWidth },
                    { 0.0f, -halfWidth },
                    { length, halfWidth },
                    { length, -halfWidth },
                };
                for (DKVector2& v : box)
                {
                    v.Transform(rotate) += v0;
                }
                vertices.Add(box[0]);
                vertices.Add(box[1]);
                vertices.Add(box[2]);
                vertices.Add(box[2]);
                vertices.Add(box[1]);
                vertices.Add(box[3]);
            }
        }
        this->DrawTriangles(vertices, vertices.Count(), color, blendState);
    }
}

void DKCanvas::DrawTriangles(const ColoredVertex* verts,
                             size_t numVerts,
                             const DKBlendState& blendState)
{
    if (numVerts > 2)
    {
        DKArray<TexturedVertex> vertices;
        vertices.Reserve(numVerts);
        for (size_t i = 0; i < numVerts; ++i)
        {
            const ColoredVertex& v = verts[i];
            DKPoint pos = v.position.Vector().Transform(screenTransform);
            vertices.Add(TexturedVertex{ pos, DKPoint::zero, v.color });
        }

        EncodeDrawCommand((uint32_t)CanvasShaderIndex::DrawVertexColor,
                          vertices, vertices.Count(),
                          nullptr,
                          blendState,
                          nullptr, 0);
    }
}

void DKCanvas::DrawTriangles(const TexturedVertex* verts,
                             size_t numVerts,
                             const DKTexture* texture,
                             const DKBlendState& blendState)
{
    if (numVerts > 2)
    {
        DKArray<TexturedVertex> vertices;
        vertices.Reserve(numVerts);
        for (size_t i = 0; i < numVerts; ++i)
        {
            const TexturedVertex& v = verts[i];
            DKPoint pos = v.position.Vector().Transform(screenTransform);
            vertices.Add(TexturedVertex{ pos, v.texcoord, v.color });
        }

        EncodeDrawCommand((uint32_t)CanvasShaderIndex::DrawVertexColorTexture,
                          vertices, vertices.Count(),
                          texture,
                          blendState,
                          nullptr, 0);
    }
}

void DKCanvas::DrawTriangles(const DKPoint* verts,
                             size_t numVerts,
                             const DKColor& color,
                             const DKBlendState& blendState)
{
    if (numVerts > 2)
    {
        DKArray<TexturedVertex> vertices;
        vertices.Reserve(numVerts);
        for (size_t i = 0; i < numVerts; ++i)
        {
            DKPoint pos = verts[i].Vector().Transform(screenTransform);
            vertices.Add(TexturedVertex{ pos, DKPoint::zero, color });
        }

        EncodeDrawCommand((uint32_t)CanvasShaderIndex::DrawVertexColor,
                          vertices, vertices.Count(),
                          nullptr,
                          blendState,
                          nullptr, 0);
    }
}

void DKCanvas::DrawLineStrip(const DKPoint* points,
                             size_t numPoints,
                             const DKColor& color,
                             const DKBlendState& blendState,
                             float lineWidth)
{
    if (numPoints > 1 && lineWidth > minimumScaleFactor)
    {       
        const float halfWidth = lineWidth * 0.5;

        DKArray<DKPoint> vertices; // triangle strip vertices
        vertices.Reserve(numPoints * 4);

        for (int i = 0; (i + 1) < numPoints; ++i)
        {
            DKVector2 v0 = points[i].Vector();
            DKVector2 v1 = points[i + 1].Vector();

            DKVector2 line = v1 - v0;
            float length = line.Length();
            if (length > minimumScaleFactor)
            {
                line.Normalize();
                float cosR = line.x;
                float sinR = line.y;

                const DKMatrix2 rotate(cosR, sinR, -sinR, cosR);

                const DKVector2 box[4] = {
                    { 0.0f, halfWidth },
                    { 0.0f, -halfWidth },
                    { length, halfWidth },
                    { length, -halfWidth },
                };
                for (const DKVector2& v : box)
                {
                    DKVector2 v2 = DKVector2(v).Transform(rotate);
                    vertices.Add(v2 + v0);
                }
            }
        }
        this->DrawTriangleStrip(vertices, vertices.Count(), color, blendState);
    }
}

void DKCanvas::DrawTriangleStrip(const DKPoint* verts,
                                 size_t numVerts,
                                 const DKColor& color,
                                 const DKBlendState& blendState)
{
    if (numVerts > 2)
    {
        DKArray<ColoredVertex> pts;
        pts.Reserve(numVerts * 3);

        for (size_t i = 0; (i + 2) < numVerts; ++i)
        {
            if (i & 1)
            {
                pts.Add({ verts[i + 1], color });
                pts.Add({ verts[i], color });
            }
            else
            {
                pts.Add({ verts[i], color });
                pts.Add({ verts[i + 1], color });
            }
            pts.Add({ verts[i + 2], color });
        }
        this->DrawTriangles(pts, pts.Count(), blendState);
    }
}

void DKCanvas::DrawTriangleStrip(const ColoredVertex* verts,
                                 size_t numVerts,
                                 const DKBlendState& blendState)
{
    if (numVerts > 2)
    {
        DKArray<ColoredVertex> pts;
        pts.Reserve(numVerts * 3);

        for (size_t i = 0; (i + 2) < numVerts; ++i)
        {
            if (i & 1)
            {
                pts.Add(verts[i + 1]);
                pts.Add(verts[i]);
            }
            else
            {
                pts.Add(verts[i]);
                pts.Add(verts[i + 1]);
            }
            pts.Add(verts[i + 2]);
        }
        this->DrawTriangles(pts, pts.Count(), blendState);
    }
}

void DKCanvas::DrawTriangleStrip(const TexturedVertex* verts,
                                 size_t numVerts,
                                 const DKTexture* texture,
                                 const DKBlendState& blendState)
{
    if (numVerts > 2 && texture)
    {
        DKArray<TexturedVertex> pts;
        pts.Reserve(numVerts * 3);

        for (size_t i = 0; (i + 2) < numVerts; ++i)
        {
            if (i & 1)
            {
                pts.Add(verts[i + 1]);
                pts.Add(verts[i]);
            }
            else
            {
                pts.Add(verts[i]);
                pts.Add(verts[i + 1]);
            }
            pts.Add(verts[i + 2]);
        }
        this->DrawTriangles(pts, pts.Count(), texture, blendState);
    }
}

void DKCanvas::DrawQuad(const DKPoint& lt,
                        const DKPoint& rt,
                        const DKPoint& lb,
                        const DKPoint& rb,
                        const DKColor& color,
                        const DKBlendState& blendState)
{
    if (IsDrawable())
    {
        const DKVector2 tpos[4] = {
            lt.Vector().Transform(this->contentTransform), // left-top
            rt.Vector().Transform(this->contentTransform), // right-top
            lb.Vector().Transform(this->contentTransform), // left-bottom
            rb.Vector().Transform(this->contentTransform), // right-bottom
        };
        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 && t2)
        {
            const DKPoint vf[6] = { lt, lb, rt, rt, lb, rb };
            DrawTriangles(vf, 6, color, blendState);
        }
        else if (t1)
        {
            const DKPoint vf[3] = { lt, lb, rt };
            DrawTriangles(vf, 3, color, blendState);
        }
        else if (t2)
        {
            const DKPoint vf[3] = { rt, lb, rb };
            DrawTriangles(vf, 3, color, blendState);
        }
    }
}

void DKCanvas::DrawQuad(const TexturedVertex& lt,
                        const TexturedVertex& rt,
                        const TexturedVertex& lb,
                        const TexturedVertex& rb,
                        const DKTexture* texture,
                        const DKBlendState& blendState)
{
    if (IsDrawable() && texture)
    {
        const DKVector2 tpos[4] = {
            lt.position.Vector().Transform(this->contentTransform), // left-top
            rt.position.Vector().Transform(this->contentTransform), // right-top
            lb.position.Vector().Transform(this->contentTransform), // left-bottom
            rb.position.Vector().Transform(this->contentTransform), // right-bottom
        };
        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 && t2)
        {
            const TexturedVertex vf[6] = { lt, lb, rt, rt, lb, rb };
            DrawTriangles(vf, 6, texture, blendState);
        }
        else if (t1)
        {
            const TexturedVertex vf[3] = { lt, lb, rt };
            DrawTriangles(vf, 3, texture, blendState);
        }
        else if (t2)
        {
            const TexturedVertex vf[3] = { rt, lb, rb };
            DrawTriangles(vf, 3, texture, blendState);
        }
    }
}

void DKCanvas::DrawRect(const DKRect& posRect,
                        const DKMatrix3& posTM,
                        const DKColor& color,
                        const DKBlendState& blendState)
{
    if (IsDrawable() && posRect.IsValid())
    {
        const DKVector2 pos[4] = {
            DKVector2(posRect.origin.x, posRect.origin.y).Transform(posTM),                                             // left-top
            DKVector2(posRect.origin.x + posRect.size.width, posRect.origin.y).Transform(posTM),                        // right-top
            DKVector2(posRect.origin.x, posRect.origin.y + posRect.size.height).Transform(posTM),                       // left-bottom
            DKVector2(posRect.origin.x + posRect.size.width, posRect.origin.y + posRect.size.height).Transform(posTM),  // right-bottom
        };

        const DKVector2 tpos[4] = {
            DKVector2(pos[0]).Transform(this->contentTransform),
            DKVector2(pos[1]).Transform(this->contentTransform),
            DKVector2(pos[2]).Transform(this->contentTransform),
            DKVector2(pos[3]).Transform(this->contentTransform),
        };

        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 && t2)
        {
            const DKPoint vf[6] = { pos[0], pos[2], pos[1], pos[1], pos[2], pos[3] };
            DrawTriangles(vf, 6, color, blendState);
        }
        else if (t1)
        {
            const DKPoint vf[3] = { pos[0], pos[2],pos[1] };
            DrawTriangles(vf, 3, color, blendState);
        }
        else if (t2)
        {
            const DKPoint vf[3] = { pos[1], pos[2], pos[3] };
            DrawTriangles(vf, 3, color, blendState);
        }
    }
}

void DKCanvas::DrawRect(const DKRect& posRect, const DKMatrix3& posTM,
                        const DKRect& texRect, const DKMatrix3& texTM,
                        const DKTexture* texture,
                        const DKColor& color,
                        const DKBlendState& blendState)
{
    if (IsDrawable() && texture && posRect.IsValid())
    {
        const DKVector2 pos[4] = {
            DKVector2(posRect.origin.x, posRect.origin.y).Transform(posTM),                                             // left-top
            DKVector2(posRect.origin.x + posRect.size.width, posRect.origin.y).Transform(posTM),                        // right-top
            DKVector2(posRect.origin.x, posRect.origin.y + posRect.size.height).Transform(posTM),                       // left-bottom
            DKVector2(posRect.origin.x + posRect.size.width, posRect.origin.y + posRect.size.height).Transform(posTM),  // right-bottom
        };
        const DKVector2 tex[4] = {
            DKVector2(texRect.origin.x, texRect.origin.y).Transform(texTM),												// left-top
            DKVector2(texRect.origin.x + texRect.size.width, texRect.origin.y).Transform(texTM),                        // right-top
            DKVector2(texRect.origin.x, texRect.origin.y + texRect.size.height).Transform(texTM),						// left-bottom
            DKVector2(texRect.origin.x + texRect.size.width, texRect.origin.y + texRect.size.height).Transform(texTM),	// right-bottom
        };

        const DKVector2 tpos[4] = {
            DKVector2(pos[0]).Transform(this->contentTransform),
            DKVector2(pos[1]).Transform(this->contentTransform),
            DKVector2(pos[2]).Transform(this->contentTransform),
            DKVector2(pos[3]).Transform(this->contentTransform),
        };

        bool t1 = this->contentBounds.IntersectTriangle(tpos[0], tpos[2], tpos[1]);
        bool t2 = this->contentBounds.IntersectTriangle(tpos[1], tpos[2], tpos[3]);
        if (t1 && t2)
        {
            const TexturedVertex vf[6] = {
                { pos[0], tex[0], color },
                { pos[2], tex[2], color },
                { pos[1], tex[1], color },
                { pos[1], tex[1], color },
                { pos[2], tex[2], color },
                { pos[3], tex[3], color }
            };
            DrawTriangles(vf, 6, texture, blendState);
        }
        else if (t1)
        {
            const TexturedVertex vf[3] = {
                { pos[0], tex[0], color },
                { pos[2], tex[2], color },
                { pos[1], tex[1], color }
            };
            DrawTriangles(vf, 3, texture, blendState);
        }
        else if (t2)
        {
            const TexturedVertex vf[3] = {
                { pos[1], tex[1], color },
                { pos[2], tex[2], color },
                { pos[3], tex[3], color }
            };
            DrawTriangles(vf, 3, texture, blendState);
        }
    }
}

void DKCanvas::DrawEllipseOutline(const DKRect& bounds,
                                  const DKMatrix3& transform,
                                  const DKColor& color,
                                  const DKBlendState& blendState,
                                  const DKSize& border)
{
    if (IsDrawable() && bounds.IsValid() && border.width > 0.0f && border.height > 0.0f)
    {
        const DKSize ibSize = bounds.size - border;  // inner bounds (hole) size
        const DKPoint ibOrigin = bounds.origin + ibSize.Vector() * 0.5f; // inner bounds origin

        if (ibSize.width < minimumScaleFactor || ibSize.height < minimumScaleFactor)
            return DrawEllipse(bounds, transform, color, blendState);

        DKMatrix3 tm = transform * screenTransform; // user transform * screen space
        const DKVector2 pos[4] = {
            DKVector2(bounds.origin.x, bounds.origin.y).Transform(tm),                                          // left-top
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y).Transform(tm),                      // right-top
            DKVector2(bounds.origin.x, bounds.origin.y + bounds.size.height).Transform(tm),                     // left-bottom
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height).Transform(tm), // right-bottom
        };

        const DKRect local(-1.0f, -1.0f, 2.0f, 2.0f); // 3d frustum space of screen.
        if (local.IntersectTriangle(pos[0], pos[2], pos[1]) || local.IntersectTriangle(pos[1], pos[2], pos[3]))
        {
            DKVector2 radiusSq = {
                (pos[1] - pos[0]).LengthSq() * 0.25f,
                (pos[0] - pos[2]).LengthSq() * 0.25f
            };
            if (radiusSq.x * radiusSq.y > minimumScaleFactor)
            {
                const DKVector2 ibpos[3] = {
                    (ibOrigin.Vector()).Transform(tm),                                  // left-top
                    (ibOrigin.Vector() + DKVector2(ibSize.width, 0.0f)).Transform(tm),  // right-top
                    (ibOrigin.Vector() + DKVector2(0.0f, ibSize.height)).Transform(tm), // left-bottom
                };

                DKVector2 ibRadiusSq = {
                    (ibpos[1] - ibpos[0]).LengthSq() * 0.25f,
                    (ibpos[0] - ibpos[2]).LengthSq() * 0.25f
                };

                // formula: X^2 / A^2 + Y^2 / B^2 = 1
                // A^2 = bounds.width/2, B^2 = bounds.height/2
                EllipseUniformPushConstant ellipseData = {};
                ellipseData.center = bounds.Center().Vector().Transform(screenTransform);
                ellipseData.outerRadiusSqInv.x = 1.0f / radiusSq.x;
                ellipseData.outerRadiusSqInv.y = 1.0f / radiusSq.y;
                ellipseData.innerRadiusSqInv.x = 1.0f / ibRadiusSq.x;
                ellipseData.innerRadiusSqInv.y = 1.0f / ibRadiusSq.y;

                const TexturedVertex vf[6] = {
                    { pos[0], DKPoint::zero, color },
                    { pos[2], DKPoint::zero, color },
                    { pos[1], DKPoint::zero, color },
                    { pos[1], DKPoint::zero, color },
                    { pos[2], DKPoint::zero, color },
                    { pos[3], DKPoint::zero, color }
                };

                EncodeDrawCommand((uint32_t)CanvasShaderIndex::DrawVertexColorEllipseHole,
                                  vf, std::size(vf),
                                  nullptr,
                                  blendState,
                                  &ellipseData, sizeof(ellipseData));
            }
        }
    }
}

void DKCanvas::DrawEllipse(const DKRect& bounds,
                           const DKMatrix3& transform,
                           const DKColor& color,
                           const DKBlendState& blendState)
{
    if (IsDrawable() && bounds.IsValid())
    {
        DKMatrix3 tm = transform * screenTransform; // user transform * screen space
        const DKVector2 pos[4] = {
            DKVector2(bounds.origin.x, bounds.origin.y).Transform(tm),                                          // left-top
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y).Transform(tm),                      // right-top
            DKVector2(bounds.origin.x, bounds.origin.y + bounds.size.height).Transform(tm),                     // left-bottom
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height).Transform(tm), // right-bottom
        };

        const DKRect local(-1.0f, -1.0f, 2.0f, 2.0f); // 3d frustum space of screen.
        if (local.IntersectTriangle(pos[0], pos[2], pos[1]) || local.IntersectTriangle(pos[1], pos[2], pos[3]))
        {
            DKVector2 radiusSq = {
                (pos[1] - pos[0]).LengthSq() * 0.25f,
                (pos[0] - pos[2]).LengthSq() * 0.25f
            };
            if (radiusSq.x * radiusSq.y > minimumScaleFactor)
            {
                // formula: X^2 / A^2 + Y^2 / B^2 = 1
                // A^2 = bounds.width/2, B^2 = bounds.height/2
                EllipseUniformPushConstant ellipseData = {};
                ellipseData.center = bounds.Center().Vector().Transform(screenTransform);
                ellipseData.outerRadiusSqInv.x = 1.0f / radiusSq.x;
                ellipseData.outerRadiusSqInv.y = 1.0f / radiusSq.y;

                const TexturedVertex vf[6] = {
                    { pos[0], DKPoint::zero, color },
                    { pos[2], DKPoint::zero, color },
                    { pos[1], DKPoint::zero, color },
                    { pos[1], DKPoint::zero, color },
                    { pos[2], DKPoint::zero, color },
                    { pos[3], DKPoint::zero, color }
                };

                EncodeDrawCommand((uint32_t)CanvasShaderIndex::DrawVertexColorEllipse,
                                  vf, std::size(vf),
                                  nullptr,
                                  blendState,
                                  &ellipseData, sizeof(ellipseData));
            }
        }
    }
}

void DKCanvas::DrawEllipse(const DKRect& bounds,
                           const DKMatrix3& transform,
                           const DKRect& uvBounds,
                           const DKMatrix3& uvTransform,
                           const DKTexture* texture,
                           const DKColor& color,
                           const DKBlendState& blendState)
{
    if (IsDrawable() && texture && bounds.IsValid())
    {
        DKMatrix3 tm = transform * screenTransform; // user transform * screen space
        const DKVector2 pos[4] = {
            DKVector2(bounds.origin.x, bounds.origin.y).Transform(tm),                                          // left-top
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y).Transform(tm),                      // right-top
            DKVector2(bounds.origin.x, bounds.origin.y + bounds.size.height).Transform(tm),                     // left-bottom
            DKVector2(bounds.origin.x + bounds.size.width, bounds.origin.y + bounds.size.height).Transform(tm), // right-bottom
        };

        const DKRect local(-1.0f, -1.0f, 2.0f, 2.0f); // 3d frustum space of screen.
        if (local.IntersectTriangle(pos[0], pos[2], pos[1]) || local.IntersectTriangle(pos[1], pos[2], pos[3]))
        {
            DKVector2 radiusSq = {
                (pos[1] - pos[0]).LengthSq() * 0.25f,
                (pos[0] - pos[2]).LengthSq() * 0.25f
            };
            if (radiusSq.x * radiusSq.y > minimumScaleFactor)
            {
                // formula: X^2 / A^2 + Y^2 / B^2 = 1
                // A^2 = bounds.width/2, B^2 = bounds.height/2
                EllipseUniformPushConstant ellipseData = {};
                ellipseData.center = bounds.Center().Vector().Transform(screenTransform);
                ellipseData.outerRadiusSqInv.x = 1.0f / radiusSq.x;
                ellipseData.outerRadiusSqInv.y = 1.0f / radiusSq.y;

                const DKVector2 uv[4] = {
                    DKVector2(uvBounds.origin.x, uvBounds.origin.y).Transform(uvTransform),                             // left-top
                    DKVector2(uvBounds.origin.x + uvBounds.size.width, uvBounds.origin.y).Transform(uvTransform),      // right-top
                    DKVector2(uvBounds.origin.x, uvBounds.origin.y + uvBounds.size.height).Transform(uvTransform),     // left-bottom
                    DKVector2(uvBounds.origin.x + uvBounds.size.width, uvBounds.origin.y + uvBounds.size.height).Transform(uvTransform), // right-bottom
                };

                const TexturedVertex vf[6] = {
                    { pos[0], uv[0], color },
                    { pos[2], uv[2], color },
                    { pos[1], uv[1], color },
                    { pos[1], uv[1], color },
                    { pos[2], uv[2], color },
                    { pos[3], uv[3], color }
                };

                EncodeDrawCommand((uint32_t)CanvasShaderIndex::DrawVertexColorTexturedEllipse,
                                  vf, std::size(vf),
                                  texture,
                                  blendState,
                                  &ellipseData, sizeof(ellipseData));
            }
        }
    }
}

void DKCanvas::DrawText(const DKRect& bounds,
                        const DKMatrix3& transform,
                        const DKString& text,
                        const DKFont* font,
                        const DKColor& color)
{
    if (!IsDrawable() || !bounds.IsValid())
        return;
    size_t textLen = text.Length();
    if (font == nullptr || !font->IsValid() || textLen == 0)
        return;

    struct Quad
    {
        TexturedVertex lt, rt, lb, rb;
        const DKTexture* texture;
    };

    DKArray<Quad> quads;
    quads.Reserve(textLen);

    DKPoint bboxMin(0, 0);
    DKPoint bboxMax(0, 0);
    float offset = 0;       // accumulated text width (pixel)

    for (size_t i = 0; i < textLen; ++i)
    {
        // get glyph info from font object
        const DKFont::GlyphData* glyph = font->GlyphDataForChar(text[i]);
        if (glyph == nullptr)
            continue;

        DKPoint posMin(offset + glyph->position.x, glyph->position.y);
        DKPoint posMax(posMin + glyph->frame.size.Vector());

        if (offset > 0)
        {
            if (bboxMin.x > posMin.x) bboxMin.x = posMin.x;
            if (bboxMin.y > posMin.y) bboxMin.y = posMin.y;
            if (bboxMax.x < posMax.x) bboxMax.x = posMax.x;
            if (bboxMax.y < posMax.y) bboxMax.y = posMax.y;
        }
        else
        {
            bboxMin = posMin;
            bboxMax = posMax;
        }

        if (glyph->texture)
        {
            uint32_t textureWidth = glyph->texture->Width();
            uint32_t textureHeight = glyph->texture->Height();
            if (textureWidth > 0 && textureHeight > 0)
            {
                float invW = 1.0f / static_cast<float>(textureWidth);
                float invH = 1.0f / static_cast<float>(textureHeight);

                DKPoint uvMin(glyph->frame.origin.x * invW, glyph->frame.origin.y * invH);
                DKPoint uvMax((glyph->frame.origin.x + glyph->frame.size.width) * invW,
                              (glyph->frame.origin.y + glyph->frame.size.height) * invH);

                const Quad q =
                {
                    TexturedVertex { DKVector2(posMin.x, posMin.y), DKVector2(uvMin.x, uvMin.y), color }, // lt
                    TexturedVertex { DKVector2(posMax.x, posMin.y), DKVector2(uvMax.x, uvMin.y), color }, // rt
                    TexturedVertex { DKVector2(posMin.x, posMax.y), DKVector2(uvMin.x, uvMax.y), color }, // lb
                    TexturedVertex { DKVector2(posMax.x, posMax.y), DKVector2(uvMax.x, uvMax.y), color }, // rb
                    glyph->texture,
                };
                quads.Add(q);
            }
        }
        offset += glyph->advance.width + font->KernAdvance(text[i], text[i + 1]).x; // text[i+1] can be null.
    }
    if (quads.Count() == 0)
        return;

    const float width = bboxMax.x - bboxMin.x;
    const float height = bboxMax.y - bboxMin.y;

    if (width <= 0.0f || height <= 0.0f)
        return;

    // sort by texture order
    quads.Sort([](const Quad& lhs, const Quad& rhs)
    {
        return reinterpret_cast<uintptr_t>(lhs.texture) > reinterpret_cast<uintptr_t>(rhs.texture);
    });

    // calculate transform matrix
    DKAffineTransform2 trans;
    trans.Translate(-bboxMin.x, -bboxMin.y);    // move origin
    trans *= DKLinearTransform2().Scale(1.0f / width, 1.0f / height); // normalize size
    trans *= DKLinearTransform2().Scale(bounds.size.width, bounds.size.height); // scale to bounds
    trans.Translate(bounds.origin.x, bounds.origin.y); // move to bounds origin

    DKMatrix3 matrix = trans.Matrix3();
    matrix *= transform; // user transform
    matrix *= screenTransform;  // transform to screen-space

    const DKTexture* lastTexture = nullptr;
    DKArray<TexturedVertex> triangles;
    triangles.Reserve(quads.Count() * 6);
    for (const Quad& q : quads)
    {
        if (q.texture != lastTexture)
        {
            if (triangles.Count() > 0)
                EncodeDrawCommand((uint32_t)CanvasShaderIndex::DrawVertexColorAlphaTexture, 
                                  triangles, triangles.Count(),
                                  lastTexture,
                                  DKBlendState::defaultAlpha,
                                  nullptr, 0);
            triangles.Clear();
            lastTexture = q.texture;
        }
        TexturedVertex vf[6] = {
            TexturedVertex { q.lt.position.Vector().Transform(matrix), q.lt.texcoord, color },
            TexturedVertex { q.lb.position.Vector().Transform(matrix), q.lb.texcoord, color },
            TexturedVertex { q.rt.position.Vector().Transform(matrix), q.rt.texcoord, color },
            TexturedVertex { q.rt.position.Vector().Transform(matrix), q.rt.texcoord, color },
            TexturedVertex { q.lb.position.Vector().Transform(matrix), q.lb.texcoord, color },
            TexturedVertex { q.rb.position.Vector().Transform(matrix), q.rb.texcoord, color },
        };
        triangles.Add(vf, 6);
    }
    if (triangles.Count() > 0)
        EncodeDrawCommand((uint32_t)CanvasShaderIndex::DrawVertexColorAlphaTexture,
                          triangles, triangles.Count(),
                          lastTexture,
                          DKBlendState::defaultAlpha,
                          nullptr, 0);
}

void DKCanvas::DrawText(const DKPoint& baselineBegin,
                        const DKPoint& baselineEnd,
                        const DKString& text,
                        const DKFont* font,
                        const DKColor& color)
{
    if (IsDrawable() == false)
        return;
    if (font == nullptr || !font->IsValid() || text.Length() == 0)
        return;

    if ((baselineEnd.Vector() - baselineBegin.Vector()).Length() < FLT_EPSILON)
        return;

    // font size, screen size in pixel units
    const float ascender = font->Ascender();
    const float lineHeight = font->LineHeight();
    const float lineWidth = font->LineWidth(text);
    const DKRect textBounds = font->Bounds(text);

    const DKSize& viewportSize = this->viewport.size;
    const DKSize& contentScale = this->contentBounds.size;

    // change local-coords to pixel-coords
    const DKSize scaleToScreen = DKSize(viewportSize.width / contentScale.width, viewportSize.height / contentScale.height);
    const DKVector2 baselinePixelBegin = DKVector2(baselineBegin.x * scaleToScreen.width, baselineBegin.y * scaleToScreen.height);
    const DKVector2 baselinePixelEnd = DKVector2(baselineEnd.x * scaleToScreen.width, baselineEnd.y * scaleToScreen.height);
    const float scale = (baselinePixelEnd - baselinePixelBegin).Length();
    const DKVector2 baselinePixelDir = (baselinePixelEnd - baselinePixelBegin).Normalize();
    const float angle = acosf(baselinePixelDir.x) * ((baselinePixelDir.y < 0) ? -1.0f : 1.0f);

    // calculate transform (matrix)
    DKAffineTransform2 transform(0, -ascender);                 // move pivot to baseline
    transform *= DKLinearTransform2()
        .Scale(scale / lineWidth)										// scale
        .Rotate(angle)													// rotate
        .Scale(1.0f / viewportSize.width, 1.0f / viewportSize.height)	// normalize (0~1)
        .Scale(contentScale.width, contentScale.height);				// apply contentScale
    transform.Translate(baselineBegin.Vector());

    DrawText(textBounds, transform.Matrix3(), text, font, color);
}

void DKCanvas::Commit()
{
    commandBuffer->Commit();
    commandBuffer = nullptr;
}

bool DKCanvas::IsDrawable(void) const
{
    return drawable;
}

bool DKCanvas::CachePipelineContext(DKGraphicsDeviceContext* context)
{
    DKGraphicsDevice* device = nullptr;
    if (context)
        device = context->Device();

    if (device)
    {
        DKObject<CanvasPipelineStates> states = CanvasPipelineStates::SharedInstance(device);
        if (DKUnknown* data = states.SafeCast<DKUnknown>(); data)
        {
            context->cachedDeviceResources.Update("DKCanvas.CanvasPipelineStates",
                                                  data);
            return true;
        }
    }
    return false;
}

void DKCanvas::EncodeDrawCommand(uint32_t materialIndex,
                                 const TexturedVertex* vertices,
                                 size_t numVerts,
                                 const DKTexture* texture,
                                 const DKBlendState& blendState,
                                 void* pushConstantData,
                                 size_t pushConstantDataLength)
{
    DKASSERT_DEBUG(commandBuffer);
    DKASSERT_DEBUG(pipelineStates);
    DKASSERT_DEBUG(IsDrawable());

    if (numVerts == 0)
        return;

    CanvasPipelineStates* states = pipelineStates.StaticCast<CanvasPipelineStates>();
    DKASSERT_DEBUG(states);

    bool textureRequired = false;
    bool pushConstantDataRequired = false;

    CanvasPipelineDescriptor desc = { static_cast<CanvasShaderIndex>(materialIndex) };
    switch (desc.shader)
    {
    case CanvasShaderIndex::DrawVertexColor:
        textureRequired = false;
        pushConstantDataRequired = false;
        break;
    case CanvasShaderIndex::DrawVertexColorTexture:
        textureRequired = true;
        pushConstantDataRequired = false;
        break;
    case CanvasShaderIndex::DrawVertexColorEllipse:
        textureRequired = false;
        pushConstantDataRequired = true;
        break;
    case CanvasShaderIndex::DrawVertexColorEllipseHole:
        textureRequired = false;
        pushConstantDataRequired = true;
        break;
    case CanvasShaderIndex::DrawVertexColorTexturedEllipse:
        textureRequired = true;
        pushConstantDataRequired = true;
        break;
    case CanvasShaderIndex::DrawVertexColorAlphaTexture:
        textureRequired = true;
        pushConstantDataRequired = false;
        break;
    default:
        DKLogE("ERROR: Unknown material");
        return;
    }

    if (textureRequired && texture == nullptr)
    {
        DKLogE("ERROR: Invalid Texture Object (Texture cannot be null)");
        return;
    }
    if (pushConstantDataRequired && 
        pushConstantDataLength != sizeof(EllipseUniformPushConstant))
    {
        DKLogE("ERROR: Invalid Ellipse (Push-Constant) Data");
        return;
    }

    desc.colorFormat = renderTarget->PixelFormat();
    desc.depthFormat = DKPixelFormat::Invalid;
    desc.SetBlendState(blendState);

    DKRenderPipelineState* pso = states->StateForDescriptor(desc);
    if (pso)
    {
        DKRenderPassColorAttachmentDescriptor colorAttachmentDesc =
        {
            renderTarget,
            0,
            DKRenderPassAttachmentDescriptor::LoadActionLoad,
            DKRenderPassAttachmentDescriptor::StoreActionStore,
            DKColor(0, 0, 0, 0)
        };
        DKRenderPassDepthStencilAttachmentDescriptor depthAttachmentDesc = {};

        DKRenderPassDescriptor desc =
        {
            { colorAttachmentDesc, },
            depthAttachmentDesc,
            0
        };

        DKGraphicsDevice* device = commandBuffer->Device();

        size_t bufferLength = sizeof(TexturedVertex) * numVerts;
        DKObject<DKGpuBuffer> vertexBuffer = device->CreateBuffer(bufferLength,
                                                                  DKGpuBuffer::StorageModeShared,
                                                                  DKCpuCacheModeWriteOnly);
        if (vertexBuffer)
        {
            memcpy(vertexBuffer->Contents(), vertices, bufferLength);
            vertexBuffer->Flush();
        }
        else
        {
            DKLogE("ERROR: Cannot create GPU-Buffer object with length:%llu", bufferLength);
            return;
        }

        DKObject<DKRenderCommandEncoder> encoder = commandBuffer->CreateRenderCommandEncoder(desc);

        if (encoder)
        {
            encoder->SetRenderPipelineState(pso);
            if (textureRequired)
            {
                states->defaultBindingSet->SetTexture(0, const_cast<DKTexture*>(texture));
                states->defaultBindingSet->SetSamplerState(0, states->defaultSampler);
                encoder->SetResources(0, states->defaultBindingSet);
            }
            if (pushConstantDataRequired)
            {
                encoder->PushConstant((uint32_t)DKShaderStage::Fragment,
                                      0,
                                      pushConstantDataLength,
                                      pushConstantData);
            }
            encoder->SetVertexBuffer(vertexBuffer, 0, 0);
            encoder->Draw(numVerts, 1, 0, 0);
            encoder->EndEncoding();
        }
    }
}
