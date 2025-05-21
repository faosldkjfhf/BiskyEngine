#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include <fmt/color.h>
#include <fmt/core.h>

#include <fastgltf/core.hpp>
#include <fastgltf/dxmath_element_traits.hpp>
#include <fastgltf/tools.hpp>

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wincodec.h>
#include <windowsx.h>

#include <wrl.h>
using namespace Microsoft::WRL;

#include <DirectXMath.h>
using namespace DirectX;

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>

#ifndef NDEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

template <typename T> using Owner = std::unique_ptr<T>;
template <typename T, typename... Args> auto MakeOwner(Args &&...args)
{
  return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T> using Ref = std::shared_ptr<T>;
template <typename T, typename... Args> auto MakeRef(Args &&...args)
{
  return std::make_shared<T>(std::forward<Args>(args)...);
}
