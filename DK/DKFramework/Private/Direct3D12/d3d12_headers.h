#pragma once

#if !DKGL_USE_DIRECT3D
#error DKGL_USE_DIRECT3D must be set to 1
#endif

#include <wrl.h>
#include <D3D12.h>
#include <dxgi1_5.h>
#include <D3Dcompiler.h>
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;
