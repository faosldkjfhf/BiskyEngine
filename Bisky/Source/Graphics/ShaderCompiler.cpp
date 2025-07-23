#include "Common.hpp"

#include "Core/ResourceManager.hpp"
#include "Graphics/ShaderCompiler.hpp"
#include <ranges>

namespace bisky::gfx::ShaderCompiler
{

wrl::ComPtr<IDxcCompiler3>                                     compiler;
wrl::ComPtr<IDxcUtils>                                         utils;
wrl::ComPtr<IDxcIncludeHandler>                                includeHandler;
std::unordered_map<std::string, wrl::ComPtr<IDxcBlobEncoding>> sourceEncodings;

std::optional<Shader> compile(
    const ShaderType &shaderType, const std::filesystem::path &filename, const std::wstring_view entryPoint
)
{
    if (!utils)
    {
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        utils->CreateDefaultIncludeHandler(&includeHandler);
    }

    const std::filesystem::path path        = core::ResourceManager::get().getShaderDirectory() / filename;
    const std::filesystem::path includePath = core::ResourceManager::get().getShaderDirectory() / "Include";

    const std::wstring target = [=]() {
        switch (shaderType)
        {
        case ShaderType::Vertex:
            return L"vs_6_6";
            break;
        case ShaderType::Pixel:
            return L"ps_6_6";
            break;
        case ShaderType::Compute:
            return L"cs_6_6";
            break;
        default:
            return L"";
            break;
        }
    }();

    std::vector<LPCWSTR> compilationArgs = {
        L"-E",
        entryPoint.data(),
        L"-T",
        target.c_str(),
        L"-Qstrip_debug",
        L"-Qstrip_reflect",
        DXC_ARG_PACK_MATRIX_ROW_MAJOR,
        DXC_ARG_WARNINGS_ARE_ERRORS,
        L"-I",
        includePath.c_str(),
    };

#ifdef _DEBUG
    compilationArgs.push_back(DXC_ARG_DEBUG);
#else
    compilationArgs.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
#endif

    // ----- compile source code -----
    auto it = sourceEncodings.find(path.string());
    if (it == sourceEncodings.end())
    {
        if (FAILED(utils->LoadFile(path.c_str(), nullptr, &sourceEncodings[path.string()])))
        {
            LOG_ERROR("Failed to load file " + path.string());
            return std::nullopt;
        }
    }

    // ----- create source buffer --
    const DxcBuffer sourceBuffer = {
        .Ptr      = sourceEncodings[path.string()]->GetBufferPointer(),
        .Size     = sourceEncodings[path.string()]->GetBufferSize(),
        .Encoding = 0u,
    };

    // ----- compile shader code -----
    wrl::ComPtr<IDxcResult> compiledBuffer;
    HRESULT                 hr = compiler->Compile(
        &sourceBuffer, compilationArgs.data(), static_cast<uint32_t>(compilationArgs.size()), includeHandler.Get(),
        IID_PPV_ARGS(&compiledBuffer)
    );
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to compile " + path.string());
        return std::nullopt;
    }

    // ----- get shader code -----
    Shader shader{};
    if (compiledBuffer->HasOutput(DXC_OUT_OBJECT))
    {
        compiledBuffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader.shader), nullptr);
        LOG_INFO("Compiled " + path.string());
    }

    // ----- get errors -----
    wrl::ComPtr<IDxcBlob> errors;
    if (compiledBuffer->HasOutput(DXC_OUT_ERRORS))
    {
        compiledBuffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        if (errors && errors->GetBufferSize() > 0)
        {
            LOG_WARNING(static_cast<char const *>(errors->GetBufferPointer()));
            return std::nullopt;
        }
    }

    // ----- TODO: parse bindings -----
    wrl::ComPtr<IDxcBlob> reflectionBlob{};
    if (compiledBuffer->HasOutput(DXC_OUT_REFLECTION))
    {
        compiledBuffer->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr);
    }

    // ----- get reflection buffer -----
    const DxcBuffer reflectionBuffer{
        .Ptr      = reflectionBlob->GetBufferPointer(),
        .Size     = reflectionBlob->GetBufferSize(),
        .Encoding = 0u,
    };

    // ----- create shader reflection -----
    wrl::ComPtr<ID3D12ShaderReflection> shaderReflectionBlob;
    utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflectionBlob));

    // ----- get shader description -----
    D3D12_SHADER_DESC shaderDesc{};
    shaderReflectionBlob->GetDesc(&shaderDesc);

    for (const UINT32 i : std::views::iota(0u, shaderDesc.BoundResources))
    {
        // ----- get input binding description -----
        D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{};
        shaderReflectionBlob->GetResourceBindingDesc(i, &shaderInputBindDesc);

        // ----- get constant buffer reflection -----
        auto *shaderReflectionConstantBuffer = shaderReflectionBlob->GetConstantBufferByIndex(i);

        // ----- get constant buffer shader description -----
        D3D12_SHADER_BUFFER_DESC constantBufferDesc{};
        shaderReflectionConstantBuffer->GetDesc(&constantBufferDesc);

        // ----- update root parameter index map -----
        shader.rootParameterIndexMap[std::string(shaderInputBindDesc.Name)] = shaderInputBindDesc.BindPoint;

        // ----- resource bound at 0 is always 32-bit constants -----
        if (shaderInputBindDesc.BindPoint == 0u)
        {
            const D3D12_ROOT_PARAMETER rootParameter{
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
                .Constants =
                    {
                        .ShaderRegister = shaderInputBindDesc.BindPoint,
                        .RegisterSpace  = shaderInputBindDesc.Space,
                        .Num32BitValues = constantBufferDesc.Size / 4u,
                    },
            };
            shader.rootParameters.push_back(rootParameter);
        }
        else if (shaderInputBindDesc.Type == D3D_SIT_CBUFFER && shaderInputBindDesc.BindPoint != 0u)
        {
            const D3D12_ROOT_PARAMETER rootParameter{
                .ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV,
                .Descriptor =
                    {
                        .ShaderRegister = shaderInputBindDesc.BindPoint,
                        .RegisterSpace  = shaderInputBindDesc.Space,
                    },
            };
            shader.rootParameters.push_back(rootParameter);
        }
    }

    return shader;
}

} // namespace bisky::gfx::ShaderCompiler