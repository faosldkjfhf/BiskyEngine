#pragma once

#include "Common.h"
#include "DX12/Context.h"
#include "DX12/Initializers.h"
#include "DX12/Utilities.h"

namespace DX12
{

template <class T> class UploadBuffer
{
public:
  inline UploadBuffer(UINT elementCount, bool isConstantBuffer = true) : mIsConstantBuffer(isConstantBuffer)
  {
    mElementByteSize = sizeof(T);

    if (isConstantBuffer)
    {
      mElementByteSize = ConstantBufferByteSize(sizeof(T));
    }

    Context::Get().Device()->CreateCommittedResource(&HeapProperties(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
                                                     &ResourceDesc::Buffer(mElementByteSize * elementCount),
                                                     D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                     IID_PPV_ARGS(&mResource));

    mResource->Map(0, nullptr, reinterpret_cast<void **>(&mMappedData));
  }

  inline ~UploadBuffer()
  {
    if (mResource != nullptr)
    {
      mResource->Unmap(0, nullptr);
    }

    mMappedData = nullptr;
    mResource.Reset();
  }

  inline void CopyData(UINT elementIndex, const T &data)
  {
    memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
  }

  inline ID3D12Resource *Resource()
  {
    return mResource.Get();
  }

private:
  ComPtr<ID3D12Resource> mResource;
  BYTE *mMappedData = nullptr;
  UINT mElementByteSize = 0;
  bool mIsConstantBuffer = true;
};

} // namespace DX12
