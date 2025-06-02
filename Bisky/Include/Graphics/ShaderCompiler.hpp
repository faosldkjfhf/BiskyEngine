#pragma once

#include "Common.hpp"

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

namespace ShaderCompiler
{

bool compile(const ShaderType &shaderType, const std::filesystem::path &filename, const std::wstring_view entryPoint,
             wrl::ComPtr<IDxcBlob> &resultBlob, wrl::ComPtr<IDxcBlob> &errorBlob);

}

} // namespace bisky::gfx