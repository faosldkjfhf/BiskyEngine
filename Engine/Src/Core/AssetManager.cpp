#include "Common.h"

#include "Core/AssetManager.h"
#include "Core/Logger.h"
#include "Core/Vertex.h"
#include "DX12/Context.h"
#include "DX12/Initializers.h"
#include "DX12/Utilities.h"
#include <fstream>

namespace Core
{

const std::vector<DX12::Texture::GUIDToDXGI> AssetManager::mLookupTable = {
    {GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM},
    {GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM}};

const std::vector<DX12::Texture::GUIDToGUID> AssetManager::mFixLookupTable = {
    {GUID_WICPixelFormat24bppBGR, GUID_WICPixelFormat32bppRGBA}};

void AssetManager::Shutdown()
{
  for (auto &[_, geometry] : mGeometries)
  {
    geometry.reset();
  } // namespace Core

  for (auto &[_, material] : mMaterials)
  {
    material.reset();
  }

  for (auto &[_, texture] : mTextures)
  {
    texture.reset();
  }
}

void AssetManager::DisposeUploaders()
{
  for (auto &[_, geometry] : mGeometries)
  {
    geometry->DisposeUploaders();
  }

  for (auto &[_, texture] : mTextures)
  {
    texture->DisposeUploader();
  }
}

void AssetManager::SetCurrentWorkingDirectory(const std::filesystem::path &cwd)
{
  mCurrentWorkingDirectory = cwd;
  LOG_INFO("Set current working directory to " + mCurrentWorkingDirectory.string());
}

void AssetManager::SetShaderDirectory(const std::filesystem::path &path)
{
  mShaderDirectory = mCurrentWorkingDirectory / path;
  LOG_INFO("Set shader directory to " + mShaderDirectory.string());
};

void AssetManager::SetModelDirectory(const std::filesystem::path &path)
{
  mModelDirectory = mCurrentWorkingDirectory / path;
  LOG_INFO("Set model directory to " + mModelDirectory.string());
}

void AssetManager::SetTextureDirectory(const std::filesystem::path &path)
{
  mTextureDirectory = mCurrentWorkingDirectory / path;
  LOG_INFO("Set texture directory to " + mTextureDirectory.string());
}

AssetManager::Error AssetManager::LoadGLTF(const std::filesystem::path &filename, ID3D12GraphicsCommandList10 *cmdList,
                                           fastgltf::Options flags)
{
  std::filesystem::path modelPath = mModelDirectory / filename;
  std::string strPath = modelPath.string();

  auto data = fastgltf::GltfDataBuffer::FromPath(modelPath);
  if (data.error() != fastgltf::Error::None)
  {
    LOG_WARNING("Failed to load " + strPath);
    return LoadFile;
  }

  fastgltf::Parser parser;
  auto asset = parser.loadGltf(data.get(), modelPath.parent_path(), flags);
  if (auto error = asset.error(); error != fastgltf::Error::None)
  {
    LOG_WARNING("Failed to parse " + strPath);
    return ParseFile;
  }

  std::vector<Ref<Core::MeshGeometry>> meshes;
  std::vector<Core::Vertex> vertices;
  std::vector<UINT32> indices;

  // load the textures
  for (auto &image : asset->images)
  {
    LoadImage(cmdList, asset.get(), image);
  }

  for (auto &mesh : asset->meshes)
  {
    Core::MeshGeometry newMesh{};
    newMesh.Name = mesh.name;
    if (newMesh.Name == "")
    {
      newMesh.Name = filename.string();
    }

    vertices.clear();
    indices.clear();
    bool tangentsFound = false;
    for (auto &&p : mesh.primitives)
    {
      Core::MeshGeometry::SubmeshGeometry submesh;
      submesh.StartIndexLocation = (UINT)indices.size();
      submesh.BaseVertexLocation = (UINT)vertices.size();
      submesh.IndexCount = (UINT)asset->accessors[p.indicesAccessor.value()].count;
      size_t initialVtx = vertices.size();

      {
        auto &indexAccessor = asset->accessors[p.indicesAccessor.value()];
        indices.reserve(indices.size() + indexAccessor.count);
        fastgltf::iterateAccessor<UINT32>(asset.get(), indexAccessor,
                                          [&](UINT32 index) { indices.push_back((UINT32)initialVtx + index); });
      }
      {
        auto &posAccessor = asset->accessors[p.findAttribute("POSITION")->accessorIndex];
        vertices.resize(vertices.size() + posAccessor.count);
        fastgltf::iterateAccessorWithIndex<XMFLOAT3>(asset.get(), posAccessor, [&](XMFLOAT3 pos, size_t index) {
          Core::Vertex vertex;
          vertex.Position = pos;
          vertex.Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
          vertex.TexCoord = XMFLOAT2(0.0f, 0.0f);
          vertex.Tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
          vertices[index + initialVtx] = vertex;
        });
      }

      // TODO: TEXCOORDS, NORMAL, TANGENT
      auto normals = p.findAttribute("NORMAL");
      if (normals != p.attributes.end())
      {
        auto &accessor = asset->accessors[normals->accessorIndex];
        fastgltf::iterateAccessorWithIndex<XMFLOAT3>(asset.get(), accessor, [&](XMFLOAT3 normal, size_t index) {
          vertices[index + initialVtx].Normal = normal;
        });
      }

      auto texcoords = p.findAttribute("TEXCOORD_0");
      if (texcoords != p.attributes.end())
      {
        auto &accessor = asset->accessors[texcoords->accessorIndex];
        fastgltf::iterateAccessorWithIndex<XMFLOAT2>(asset.get(), accessor, [&](XMFLOAT2 texcoord, size_t index) {
          vertices[index + initialVtx].TexCoord = texcoord;
        });
      }

      auto tangents = p.findAttribute("TANGENT");
      if (tangents != p.attributes.end())
      {
        tangentsFound = true;
        auto &accessor = asset->accessors[tangents->accessorIndex];
        fastgltf::iterateAccessorWithIndex<XMFLOAT4>(asset.get(), accessor, [&](XMFLOAT4 tangent, size_t index) {
          vertices[index + initialVtx].Tangent = XMFLOAT3(tangent.x, tangent.y, tangent.z);
        });
      }

      newMesh.DrawArgs.push_back(submesh);
    }

    if (!tangentsFound)
    {
      for (size_t idx = 0; idx < indices.size(); idx += 3)
      {
        UINT32 i = indices[idx];
        UINT32 j = indices[idx + 1];
        UINT32 k = indices[idx + 2];

        auto pos1 = vertices[i].Position;
        auto pos2 = vertices[j].Position;
        auto pos3 = vertices[k].Position;
        auto uv1 = vertices[i].TexCoord;
        auto uv2 = vertices[j].TexCoord;
        auto uv3 = vertices[k].TexCoord;

        XMFLOAT3 edge1, edge2;
        XMFLOAT2 duv1, duv2;
        XMStoreFloat3(&edge1, XMVectorSubtract(XMLoadFloat3(&pos2), XMLoadFloat3(&pos1)));
        XMStoreFloat3(&edge2, XMVectorSubtract(XMLoadFloat3(&pos3), XMLoadFloat3(&pos1)));
        XMStoreFloat2(&duv1, XMVectorSubtract(XMLoadFloat2(&uv2), XMLoadFloat2(&uv1)));
        XMStoreFloat2(&duv2, XMVectorSubtract(XMLoadFloat2(&uv3), XMLoadFloat2(&uv1)));

        XMFLOAT3 tangent{};
        float f = 1.0f / (duv1.x * duv2.y - duv1.y * duv2.x);
        tangent.x = f * (duv2.y * edge1.x - duv1.y * edge2.x);
        tangent.y = f * (duv2.y * edge1.y - duv1.y * edge2.y);
        tangent.z = f * (duv2.y * edge1.z - duv1.y * edge2.z);

        vertices[i].Tangent = tangent;
        vertices[j].Tangent = tangent;
        vertices[k].Tangent = tangent;
      }
    }

    newMesh.VertexBufferByteSize = static_cast<UINT>(vertices.size() * sizeof(Core::Vertex));
    newMesh.VertexByteStride = sizeof(Core::Vertex);
    newMesh.IndexFormat = DXGI_FORMAT_R32_UINT;
    newMesh.IndexBufferByteSize = static_cast<UINT>(indices.size() * sizeof(UINT32));

    D3DCreateBlob(newMesh.VertexBufferByteSize, &newMesh.VertexBufferCPU);
    D3DCreateBlob(newMesh.IndexBufferByteSize, &newMesh.IndexBufferCPU);
    CopyMemory(newMesh.VertexBufferCPU->GetBufferPointer(), vertices.data(), newMesh.VertexBufferCPU->GetBufferSize());
    CopyMemory(newMesh.IndexBufferCPU->GetBufferPointer(), indices.data(), newMesh.IndexBufferCPU->GetBufferSize());

    newMesh.VertexBufferGPU = DX12::CreateBuffer(cmdList, newMesh.VertexBufferCPU, newMesh.VertexBufferUploader);
    newMesh.IndexBufferGPU = DX12::CreateBuffer(cmdList, newMesh.IndexBufferCPU, newMesh.IndexBufferUploader);

    LOG_INFO("Model added: " + newMesh.Name);
    meshes.emplace_back(MakeRef<Core::MeshGeometry>(std::move(newMesh)));
  }

  for (auto &mesh : meshes)
  {
    mGeometries[mesh->Name] = std::move(mesh);
  }

  LOG_INFO("Loaded " + strPath);
  return None;
}

Ref<Core::MeshGeometry> AssetManager::GetModel(std::string_view name)
{
  auto it = mGeometries.find(name);
  if (it == mGeometries.end())
  {
    LOG_ERROR("Couldn't find " + std::string(name) + ". Was it loaded?");
    return nullptr;
  }

  LOG_INFO("Returning " + std::string(name));
  return it->second;
}

void AssetManager::LoadImage(ID3D12GraphicsCommandList10 *cmdList, fastgltf::Asset &asset, fastgltf::Image &image)
{
  Ref<DX12::Texture> texture = nullptr;
  std::visit(fastgltf::visitor([](auto &arg) {},
                               [&](fastgltf::sources::URI &filepath) {
                                 const std::string path(filepath.uri.path().begin(), filepath.uri.path().end());

                                 DX12::Texture::ImageData data;
                                 if (!LoadImageFromDisk(path, data))
                                 {
                                   LOG_WARNING("Failed to load " + path);
                                   return;
                                 }

                                 texture = DX12::CreateTexture(cmdList, data);
                                 texture->Name = path;
                                 texture->CreateView();
                                 mTextures[texture->Name] = texture;

                                 LOG_INFO("Loaded " + path);
                               },
                               [&](fastgltf::sources::Vector &vector) { LOG_INFO("hi"); },
                               [&](fastgltf::sources::BufferView &view) {
                                 auto &bufferView = asset.bufferViews[view.bufferViewIndex];
                                 auto &buffer = asset.buffers[bufferView.bufferIndex];

                                 std::visit(fastgltf::visitor(
                                                [](auto &arg) {},
                                                [&](fastgltf::sources::Array &array) {
                                                  DX12::Texture::ImageData data;
                                                  if (!LoadImageFromMemory(
                                                          reinterpret_cast<unsigned char *>(array.bytes.data()),
                                                          bufferView.byteOffset, bufferView.byteLength, data))
                                                  {
                                                    LOG_WARNING("Failed to load buffer from memory");
                                                    return;
                                                  }
                                                  texture = DX12::CreateTexture(cmdList, data);
                                                  texture->CreateView();
                                                  texture->Name = "Material " + std::to_string(texture->HeapIndex);
                                                  mTextures[texture->Name] = texture;
                                                  LOG_INFO("Loaded " + texture->Name);
                                                }),
                                            buffer.data);
                               }),
             image.data);

  if (texture == nullptr)
  {
    LOG_WARNING("Failed to load texture");
  }
}

Ref<DX12::Texture> AssetManager::GetTexture(std::string_view name)
{
  auto it = mTextures.find(name);
  if (it == mTextures.end())
  {
    LOG_ERROR("Failed to find texture " + std::string(name));
    return nullptr;
  }

  LOG_INFO("Returning texture " + std::string(name));
  return mTextures[name];
}

ComPtr<ID3DBlob> AssetManager::LoadBinary(const std::filesystem::path &filename)
{
  std::filesystem::path shaderPath = mShaderDirectory / filename;

  std::ifstream fin(shaderPath, std::ios::binary);
  if (!fin.is_open())
  {
    LOG_ERROR("Failed to open " + shaderPath.string());
    return nullptr;
  }

  fin.seekg(0, std::ios_base::end);
  std::ifstream::pos_type size = (int)fin.tellg();
  fin.seekg(0, std::ios_base::beg);

  ComPtr<ID3DBlob> blob;
  HRESULT hr = D3DCreateBlob(size, &blob);
  if (FAILED(hr))
  {
    LOG_ERROR("Failed to create blob");
    return nullptr;
  }

  fin.read((char *)blob->GetBufferPointer(), size);
  fin.close();
  LOG_INFO("Loaded " + shaderPath.string());
  return blob;
}

ComPtr<ID3DBlob> AssetManager::CompileShader(const std::filesystem::path &filename, const D3D_SHADER_MACRO *defines,
                                             LPCSTR target, LPCSTR entryPoint)
{
  std::filesystem::path shaderPath = mShaderDirectory / filename;

  UINT compileFlags = 0;
#ifdef _DEBUG
  compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ComPtr<ID3DBlob> shader;
  ComPtr<ID3DBlob> errors;
  HRESULT hr = D3DCompileFromFile(shaderPath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, target,
                                  compileFlags, 0, &shader, &errors);
  if (FAILED(hr))
  {
    if (errors)
      LOG_ERROR((char *)errors->GetBufferPointer());

    LOG_ERROR("Failed to compile " + shaderPath.string());
    return nullptr;
  }

  LOG_INFO("Loaded " + shaderPath.string());
  return shader;
}

Ref<Core::Material> AssetManager::AddMaterial(std::string_view name)
{
  auto it = mMaterials.find(name);
  if (it != mMaterials.end())
  {
    LOG_WARNING("Found existing material named " + std::string(name));
    return it->second;
  }

  LOG_INFO("Added material " + std::string(name));

  mMaterials[name] = MakeRef<Core::Material>();
  mMaterials[name]->ConstantBufferIndex = static_cast<UINT>(mMaterials.size()) - 1;
  return mMaterials[name];
}

Ref<Core::Material> AssetManager::GetMaterial(std::string_view name)
{
  auto it = mMaterials.find(name);
  if (it == mMaterials.end())
  {
    LOG_ERROR("Failed to find material " + std::string(name));
    return nullptr;
  }

  return mMaterials[name];
}

// FIXME: Code's a little messy - fixed
// TODO: Clean up this holy moly
// Right now, seems it was decoded as 24bit but it needs to be decoded as 32bit
bool AssetManager::LoadImageFromDisk(const std::filesystem::path &filepath, DX12::Texture::ImageData &data)
{
  std::filesystem::path texturePath = mTextureDirectory / filepath;

  // Factory
  ComPtr<IWICImagingFactory> wicFactory;
  HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
  if (FAILED(hr))
  {
    LOG_ERROR("Failed to create WIC Factory");
    return false;
  }

  // Load image
  ComPtr<IWICStream> wicFileStream;
  wicFactory->CreateStream(&wicFileStream);

  // Initialize stream
  if (FAILED(wicFileStream->InitializeFromFilename(texturePath.wstring().c_str(), GENERIC_READ)))
  {
    LOG_ERROR("Failed to load " + texturePath.string());
    return false;
  }

  // Decode the image
  ComPtr<IWICBitmapDecoder> wicDecoder;
  wicFactory->CreateDecoderFromStream(wicFileStream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &wicDecoder);
  ComPtr<IWICBitmapFrameDecode> wicFrameDecode;
  wicDecoder->GetFrame(0, &wicFrameDecode);

  wicFrameDecode->GetSize(&data.Width, &data.Height);

  // this is the original pixel format that we found
  GUID pixelFormat;
  wicFrameDecode->GetPixelFormat(&pixelFormat);

  // this is eventually what we want to convert to
  GUID convertGUID;
  memcpy(&convertGUID, &pixelFormat, sizeof(GUID));

  auto it = std::find_if(mLookupTable.begin(), mLookupTable.end(), [&](const DX12::Texture::GUIDToDXGI &entry) {
    return memcmp(&entry.GUID, &pixelFormat, sizeof(GUID)) == 0;
  });
  if (it == mLookupTable.end())
  {
    auto fix =
        std::find_if(mFixLookupTable.begin(), mFixLookupTable.end(), [&](const DX12::Texture::GUIDToGUID &entry) {
          return memcmp(&entry.Source, &pixelFormat, sizeof(GUID)) == 0;
        });
    if (fix == mFixLookupTable.end())
    {
      LOG_WARNING("Failed to find suitable DXGI conversion format");
      return false;
    }

    memcpy(&convertGUID, &fix->Target, sizeof(GUID));
  }

  ComPtr<IWICComponentInfo> componentInfo;
  wicFactory->CreateComponentInfo(convertGUID, &componentInfo);

  ComPtr<IWICPixelFormatInfo> pixelFormatInfo;
  componentInfo->QueryInterface(IID_PPV_ARGS(&pixelFormatInfo));
  pixelFormatInfo->GetBitsPerPixel(&data.BitsPerPixel);
  pixelFormatInfo->GetChannelCount(&data.ChannelCount);

  // FIXME: Something wrong with the images
  // Should be guaranteed to work now that we are converting incorrect types to a valid one
  it = std::find_if(mLookupTable.begin(), mLookupTable.end(), [&](const DX12::Texture::GUIDToDXGI &entry) {
    return memcmp(&entry.GUID, &convertGUID, sizeof(GUID)) == 0;
  });
  data.Format = it->Format;

  if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0)
  {
    UINT32 stride = ((data.BitsPerPixel + 7) / 8) * data.Width;
    UINT32 size = stride * data.Height;
    data.Data.resize(size);

    WICRect copyRect{};
    copyRect.X = 0;
    copyRect.Y = 0;
    copyRect.Width = data.Width;
    copyRect.Height = data.Height;

    wicFrameDecode->CopyPixels(0, stride, size, (BYTE *)data.Data.data());
  }
  else
  {
    ComPtr<IWICFormatConverter> fc;
    wicFactory->CreateFormatConverter(&fc);
    fc->Initialize(wicFrameDecode.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0,
                   WICBitmapPaletteTypeCustom);

    UINT32 stride = ((data.BitsPerPixel + 7) / 8) * data.Width;
    UINT32 size = stride * data.Height;
    data.Data.resize(size);

    WICRect copyRect{};
    copyRect.X = 0;
    copyRect.Y = 0;
    copyRect.Width = data.Width;
    copyRect.Height = data.Height;

    fc->CopyPixels(0, stride, size, (BYTE *)data.Data.data());
  }

  return true;
}

bool AssetManager::LoadImageFromMemory(unsigned char *bytes, size_t byteOffset, size_t bufferSize,
                                       DX12::Texture::ImageData &data)
{
  // Factory
  ComPtr<IWICImagingFactory> wicFactory;
  HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
  if (FAILED(hr))
  {
    LOG_ERROR("Failed to create WIC Factory");
    return false;
  }

  // Load image
  ComPtr<IWICStream> wicStream;
  wicFactory->CreateStream(&wicStream);

  if (FAILED(wicStream->InitializeFromMemory(bytes + byteOffset, (DWORD)bufferSize)))
  {
    return false;
  }

  // Decode the image
  ComPtr<IWICBitmapDecoder> wicDecoder;
  wicFactory->CreateDecoderFromStream(wicStream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &wicDecoder);
  ComPtr<IWICBitmapFrameDecode> wicFrameDecode;
  wicDecoder->GetFrame(0, &wicFrameDecode);

  wicFrameDecode->GetSize(&data.Width, &data.Height);

  // this is the original pixel format that we found
  GUID pixelFormat;
  wicFrameDecode->GetPixelFormat(&pixelFormat);

  // this is eventually what we want to convert to
  GUID convertGUID;
  memcpy(&convertGUID, &pixelFormat, sizeof(GUID));

  auto it = std::find_if(mLookupTable.begin(), mLookupTable.end(), [&](const DX12::Texture::GUIDToDXGI &entry) {
    return memcmp(&entry.GUID, &pixelFormat, sizeof(GUID)) == 0;
  });
  if (it == mLookupTable.end())
  {
    auto fix =
        std::find_if(mFixLookupTable.begin(), mFixLookupTable.end(), [&](const DX12::Texture::GUIDToGUID &entry) {
          return memcmp(&entry.Source, &pixelFormat, sizeof(GUID)) == 0;
        });
    if (fix == mFixLookupTable.end())
    {
      LOG_WARNING("Failed to find suitable DXGI conversion format");
      return false;
    }

    memcpy(&convertGUID, &fix->Target, sizeof(GUID));
  }

  ComPtr<IWICComponentInfo> componentInfo;
  wicFactory->CreateComponentInfo(convertGUID, &componentInfo);

  ComPtr<IWICPixelFormatInfo> pixelFormatInfo;
  componentInfo->QueryInterface(IID_PPV_ARGS(&pixelFormatInfo));
  pixelFormatInfo->GetBitsPerPixel(&data.BitsPerPixel);
  pixelFormatInfo->GetChannelCount(&data.ChannelCount);

  // FIXME: Something wrong with the images
  // Should be guaranteed to work now that we are converting incorrect types to a valid one
  it = std::find_if(mLookupTable.begin(), mLookupTable.end(), [&](const DX12::Texture::GUIDToDXGI &entry) {
    return memcmp(&entry.GUID, &convertGUID, sizeof(GUID)) == 0;
  });
  data.Format = it->Format;

  if (memcmp(&convertGUID, &pixelFormat, sizeof(GUID)) == 0)
  {
    UINT32 stride = ((data.BitsPerPixel + 7) / 8) * data.Width;
    UINT32 size = stride * data.Height;
    data.Data.resize(size);

    WICRect copyRect{};
    copyRect.X = 0;
    copyRect.Y = 0;
    copyRect.Width = data.Width;
    copyRect.Height = data.Height;

    wicFrameDecode->CopyPixels(0, stride, size, (BYTE *)data.Data.data());
  }
  else
  {
    ComPtr<IWICFormatConverter> fc;
    wicFactory->CreateFormatConverter(&fc);
    fc->Initialize(wicFrameDecode.Get(), convertGUID, WICBitmapDitherTypeErrorDiffusion, 0, 0,
                   WICBitmapPaletteTypeCustom);

    UINT32 stride = ((data.BitsPerPixel + 7) / 8) * data.Width;
    UINT32 size = stride * data.Height;
    data.Data.resize(size);

    WICRect copyRect{};
    copyRect.X = 0;
    copyRect.Y = 0;
    copyRect.Width = data.Width;
    copyRect.Height = data.Height;

    fc->CopyPixels(0, stride, size, (BYTE *)data.Data.data());
  }

  return true;
}

} // namespace Core