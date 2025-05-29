#pragma once

#include "Common.h"

namespace D3D12
{

class Texture
{
public:
  struct GUIDToDXGI
  {
    GUID GUID;
    DXGI_FORMAT Format;
  };

  /*
   * Converts Source to Target
   */
  struct GUIDToGUID
  {
    GUID Source;
    GUID Target;
  };

  struct ImageData
  {
    std::vector<char *> Data;
    UINT Width;
    UINT Height;
    GUID PixelFormat;
    UINT BitsPerPixel;
    UINT ChannelCount;
    DXGI_FORMAT Format;
  };

  void CreateView();
  void DisposeUploader();

  std::string Name;
  ComPtr<ID3D12Resource> Resource;
  ComPtr<ID3D12Resource> UploadBuffer;
  UINT HeapIndex = 0;

private:
};

} // namespace D3D12