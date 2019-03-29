#pragma once

#if !DKGL_ENABLE_DIRECT3D12
#error DKGL_ENABLE_DIRECT3D12 must be set to 1
#endif

#include <wrl.h>
#include <D3D12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;
