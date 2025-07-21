#pragma once

#include <cstdint>
#include <d3d12.h>
#include <filesystem>
#include <string_view>

namespace bisky::gfx
{

struct ShaderModule
{
    std::filesystem::path name;
    std::wstring_view     entryPoint;
};

enum class FrontFace
{
    Clockwise        = false,
    CounterClockwise = true
};

struct FillMode
{
    static constexpr D3D12_FILL_MODE Solid     = D3D12_FILL_MODE_SOLID;
    static constexpr D3D12_FILL_MODE Wireframe = D3D12_FILL_MODE_WIREFRAME;
};

enum class CullMode
{
    None  = D3D12_CULL_MODE_NONE,
    Front = D3D12_CULL_MODE_FRONT,
    Back  = D3D12_CULL_MODE_BACK
};

enum class ComparisonFunc
{
    None         = D3D12_COMPARISON_FUNC_NONE,
    Never        = D3D12_COMPARISON_FUNC_NEVER,
    Less         = D3D12_COMPARISON_FUNC_LESS,
    LessEqual    = D3D12_COMPARISON_FUNC_LESS_EQUAL,
    Equal        = D3D12_COMPARISON_FUNC_EQUAL,
    Greater      = D3D12_COMPARISON_FUNC_GREATER,
    GreaterEqual = D3D12_COMPARISON_FUNC_GREATER_EQUAL,
    Always       = D3D12_COMPARISON_FUNC_ALWAYS
};

struct GraphicsPipelineStateDesc
{
    ID3D12RootSignature *const rootSignature{};
    ShaderModule               vertexShader{};
    ShaderModule               pixelShader{};
    uint32_t                   rtvCount = 1;
    std::span<DXGI_FORMAT>     rtvFormats;
    DXGI_FORMAT                dsvFormat = DXGI_FORMAT_UNKNOWN;
    CullMode                   cullMode;
    D3D12_FILL_MODE            fillMode = FillMode::Solid;
    FrontFace                  frontFace;
    ComparisonFunc             depthFunc = ComparisonFunc::Less;
};

struct IndexBufferView
{
    D3D12_GPU_VIRTUAL_ADDRESS bufferLocation;
    uint32_t                  sizeInBytes = 0u;
    DXGI_FORMAT               format      = DXGI_FORMAT_UNKNOWN;
};

} // namespace bisky::gfx