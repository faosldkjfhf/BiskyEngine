#pragma once

#include "DX12/RootParameters.h"
#include "DX12/Texture.h"

namespace DX12
{

UINT ConstantBufferByteSize(UINT bytes);

ComPtr<ID3D12RootSignature> CreateRootSignature(
    const DX12::RootParameters &parameters,
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

ComPtr<ID3D12Resource> CreateBuffer(ID3D12GraphicsCommandList10 *cmdList, ComPtr<ID3DBlob> &data,
                                    ComPtr<ID3D12Resource> &uploadBuffer);

Ref<DX12::Texture> CreateTexture(ID3D12GraphicsCommandList10 *cmdList, const DX12::Texture::ImageData &imageData);

} // namespace DX12
