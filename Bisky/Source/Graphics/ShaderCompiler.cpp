#include "Common.hpp"

#include "Core/ResourceManager.hpp"
#include "Graphics/ShaderCompiler.hpp"

namespace bisky::gfx::ShaderCompiler
{

wrl::ComPtr<IDxcCompiler3>      compiler;
wrl::ComPtr<IDxcUtils>          utils;
wrl::ComPtr<IDxcIncludeHandler> includeHandler;

bool compile(
    const ShaderType &shaderType, const std::filesystem::path &filename, const std::wstring_view entryPoint,
    wrl::ComPtr<IDxcBlob> &resultBlob, wrl::ComPtr<IDxcBlob> &errorBlob
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
        case ShaderType::Vertex: {
            return L"vs_6_6";
        }
        break;
        case ShaderType::Pixel: {
            return L"ps_6_6";
        }
        break;
        default: {
            return L"";
        }
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

    wrl::ComPtr<IDxcBlobEncoding> sourceBlob;
    if (FAILED(utils->LoadFile(path.c_str(), nullptr, &sourceBlob)))
    {
        LOG_ERROR("Failed to load file " + path.string());
        return false;
    }

    const DxcBuffer sourceBuffer = {
        .Ptr      = sourceBlob->GetBufferPointer(),
        .Size     = sourceBlob->GetBufferSize(),
        .Encoding = 0u,
    };

    wrl::ComPtr<IDxcResult> compiledBuffer;
    HRESULT                 hr = compiler->Compile(
        &sourceBuffer, compilationArgs.data(), static_cast<uint32_t>(compilationArgs.size()), includeHandler.Get(),
        IID_PPV_ARGS(&compiledBuffer)
    );
    if (FAILED(hr))
    {
        LOG_ERROR("Failed to compile " + path.string());
        return false;
    }

    bool result = false;
    if (compiledBuffer->HasOutput(DXC_OUT_OBJECT))
    {
        compiledBuffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&resultBlob), nullptr);
        LOG_INFO("Compiled " + path.string());
        result = true;
    }
    if (compiledBuffer->HasOutput(DXC_OUT_ERRORS))
    {
        compiledBuffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorBlob), nullptr);
    }

    return result;
}

} // namespace bisky::gfx::ShaderCompiler