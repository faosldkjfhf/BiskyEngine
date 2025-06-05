#pragma once

// STL
#include <array>
#include <chrono>
#include <filesystem>
#include <future>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

#include <fastgltf/core.hpp>
#include <fastgltf/dxmath_element_traits.hpp>
#include <fastgltf/tools.hpp>

#include <fmt/color.h>
#include <fmt/core.h>

#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <directx-dxc/dxcapi.h>
#include <directx/d3dx12.h>
#include <directxtk12/DDSTextureLoader.h>
#include <directxtk12/ResourceUploadBatch.h>
#include <dxgi1_6.h>

#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

#include <stb_image.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>

namespace wrl = Microsoft::WRL;
namespace dx  = DirectX;

#include "Core/Logger.hpp"
#include "Graphics/ResourceUpload.hpp"