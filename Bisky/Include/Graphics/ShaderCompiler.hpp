#pragma once

#include <dxcapi.h>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <wrl.h>

namespace bisky::gfx
{

enum class ShaderType
{
    Vertex,
    Pixel,
    Compute,
    Hull,
    Geometry,
};

struct Shader
{
    Microsoft::WRL::ComPtr<IDxcBlob>      shader                = {};
    std::unordered_map<std::string, UINT> rootParameterIndexMap = {};
    std::vector<D3D12_ROOT_PARAMETER>     rootParameters        = {};
};

namespace ShaderCompiler
{

std::optional<Shader> compile(
    const ShaderType &shaderType, const std::filesystem::path &filename, const std::wstring_view entryPoint
);

} // namespace ShaderCompiler

} // namespace bisky::gfx