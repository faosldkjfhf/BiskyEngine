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

// void AssetManager::LoadImage(ID3D12GraphicsCommandList10 *cmdList, fastgltf::Asset &asset, fastgltf::Image &image)
//{
//   Ref<DX12::Texture> texture = nullptr;
//   std::visit(fastgltf::visitor([](auto &arg) {},
//                                [&](fastgltf::sources::URI &filepath) {
//                                  const std::string path(filepath.uri.path().begin(), filepath.uri.path().end());
//
//                                  DX12::Texture::ImageData data;
//                                  if (!LoadImageFromDisk(path, data))
//                                  {
//                                    LOG_WARNING("Failed to load " + path);
//                                    return;
//                                  }
//
//                                  texture = DX12::CreateTexture(cmdList, data);
//                                  texture->Name = path;
//                                  texture->CreateView();
//                                  mTextures[texture->Name] = texture;
//
//                                  LOG_INFO("Loaded " + path);
//                                },
//                                [&](fastgltf::sources::Vector &vector) { LOG_INFO("hi"); },
//                                [&](fastgltf::sources::BufferView &view) {
//                                  auto &bufferView = asset.bufferViews[view.bufferViewIndex];
//                                  auto &buffer = asset.buffers[bufferView.bufferIndex];
//
//                                  std::visit(
//                                      fastgltf::visitor([](auto &arg) {},
//                                                        [&](fastgltf::sources::Array &array) {
//                                                          DX12::Texture::ImageData data;
//                                                          if (!LoadImageFromMemory(
//                                                                  reinterpret_cast<unsigned char
//                                                                  *>(array.bytes.data()), bufferView.byteOffset,
//                                                                  bufferView.byteLength, data))
//                                                          {
//                                                            LOG_WARNING("Failed to load buffer from memory");
//                                                            return;
//                                                          }
//                                                          texture = DX12::CreateTexture(cmdList, data);
//                                                          texture->CreateView();
//                                                          texture->Name = std::string(image.name) + std::string("_") +
//                                                                          std::to_string(texture->HeapIndex);
//                                                          mTextures[texture->Name] = texture;
//                                                          LOG_INFO("Loaded " + texture->Name);
//                                                        }),
//                                      buffer.data);
//                                }),
//              image.data);
//
//   if (texture == nullptr)
//   {
//     LOG_WARNING("Failed to load texture");
//   }
// }

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

AssetManager::Error AssetManager::LoadGLTF(const std::filesystem::path &filename, ID3D12GraphicsCommandList10 *cmdList)
{
  const std::filesystem::path modelPath = mModelDirectory / filename;
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(modelPath.string(), aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded);
  if (!scene)
  {
    LOG_WARNING("Failed to import " + modelPath.string());
    return LoadFile;
  }

  std::vector<Vertex> vertices;
  std::vector<UINT32> indices;
  for (UINT i = 0; i < scene->mNumMeshes; i++)
  {
    const auto *mesh = scene->mMeshes[i];
    vertices.clear();
    indices.clear();

    MeshGeometry::SubmeshGeometry subgeo;
    subgeo.BaseVertexLocation = 0;
    subgeo.StartIndexLocation = 0;
    subgeo.IndexCount = mesh->mNumFaces * 3;

    for (UINT j = 0; j < mesh->mNumFaces; j++)
    {
      const auto &face = mesh->mFaces[j];
      for (UINT k = 0; k < face.mNumIndices; k++)
      {
        indices.push_back(face.mIndices[k]);
      }
    }

    for (UINT j = 0; j < mesh->mNumVertices; j++)
    {
      const auto &vertex = mesh->mVertices[j];

      Vertex v{};
      v.Position = XMFLOAT3(vertex.x, vertex.y, vertex.z);
      v.Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
      v.TexCoord = XMFLOAT2(0.0f, 0.0f);
      v.Tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);

      if (mesh->HasNormals())
      {
        const auto &normal = mesh->mNormals[j];
        v.Normal = XMFLOAT3(normal.x, normal.y, normal.z);
      }

      if (mesh->HasTextureCoords(0))
      {
        const auto &uv = mesh->mTextureCoords[0][j];
        v.TexCoord = XMFLOAT2(uv.x, uv.y);
      }

      if (mesh->HasTangentsAndBitangents())
      {
        const auto &tangent = mesh->mTangents[j];
        const auto &bitangent = mesh->mBitangents[j];
        v.Tangent = XMFLOAT3(tangent.x, tangent.y, tangent.z);
        v.Bitangent = XMFLOAT3(bitangent.x, bitangent.y, bitangent.z);
      }

      vertices.push_back(v);
    }

    auto keyName = filename.filename().replace_extension().string() + "_" + std::to_string(i);
    auto geo = MakeRef<MeshGeometry>();
    geo->Name = keyName;
    geo->DrawArgs.push_back(subgeo);
    geo->VertexBufferByteSize = static_cast<UINT>(vertices.size() * sizeof(Core::Vertex));
    geo->VertexByteStride = sizeof(Core::Vertex);
    geo->IndexFormat = DXGI_FORMAT_R32_UINT;
    geo->IndexBufferByteSize = static_cast<UINT>(indices.size() * sizeof(UINT32));

    D3DCreateBlob(geo->VertexBufferByteSize, &geo->VertexBufferCPU);
    D3DCreateBlob(geo->IndexBufferByteSize, &geo->IndexBufferCPU);
    CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), geo->VertexBufferCPU->GetBufferSize());
    CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), geo->IndexBufferCPU->GetBufferSize());

    geo->VertexBufferGPU = DX12::CreateBuffer(cmdList, geo->VertexBufferCPU, geo->VertexBufferUploader);
    geo->IndexBufferGPU = DX12::CreateBuffer(cmdList, geo->IndexBufferCPU, geo->IndexBufferUploader);

    mGeometries[geo->Name] = geo;
    LOG_INFO("Model added " + geo->Name);

    auto material = AddMaterial(geo->Name);
    material->NoTexture = false;
    {

      aiString path;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_BASE_COLOR, 0, &path);
      DX12::Texture::ImageData imageData;

      auto *embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());
      if (embeddedTexture)
      {
        if (embeddedTexture->mHeight == 0)
        {
          if (!LoadImageFromMemory(reinterpret_cast<unsigned char *>(embeddedTexture->pcData), 0,
                                   embeddedTexture->mWidth, imageData))
          {
            LOG_ERROR("Failed to load " + std::string(path.C_Str()) + " from memory");
          }

          Ref<DX12::Texture> texture = DX12::CreateTexture(cmdList, imageData);
          texture->Name = path.C_Str();
          texture->CreateView();
          mTextures[texture->Name] = texture;
          material->DiffuseMapHeapIndex = texture->HeapIndex;
        }
      }
      else
      {
        if (LoadImageFromDisk(path.C_Str(), imageData))
        {
          Ref<DX12::Texture> texture = DX12::CreateTexture(cmdList, imageData);
          texture->Name = path.C_Str();
          texture->CreateView();
          mTextures[texture->Name] = texture;
          material->DiffuseMapHeapIndex = texture->HeapIndex;
        }
      }
    }
    {
      aiString path;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_NORMALS, 0, &path);
      DX12::Texture::ImageData imageData;

      auto *embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());
      if (embeddedTexture)
      {
        if (embeddedTexture->mHeight == 0)
        {
          if (!LoadImageFromMemory(reinterpret_cast<unsigned char *>(embeddedTexture->pcData), 0,
                                   embeddedTexture->mWidth, imageData))
          {
            LOG_ERROR("Failed to load " + std::string(path.C_Str()) + " from memory");
          }

          Ref<DX12::Texture> texture = DX12::CreateTexture(cmdList, imageData);
          texture->Name = path.C_Str();
          texture->CreateView();
          mTextures[texture->Name] = texture;
          material->NormalMapHeapIndex = texture->HeapIndex;
        }
      }
      else
      {
        if (LoadImageFromDisk(path.C_Str(), imageData))
        {
          Ref<DX12::Texture> texture = DX12::CreateTexture(cmdList, imageData);
          texture->Name = path.C_Str();
          texture->CreateView();
          mTextures[texture->Name] = texture;
          material->NormalMapHeapIndex = texture->HeapIndex;
        }
      }
    }
  }

  return None;
}

void AssetManager::ProcessNode(aiNode *node, aiScene *scene, ID3D12GraphicsCommandList10 *cmdList)
{
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
    LOG_WARNING("Failed to create WIC Factory");
    return false;
  }

  // Load image
  ComPtr<IWICStream> wicFileStream;
  wicFactory->CreateStream(&wicFileStream);

  // Initialize stream
  if (FAILED(wicFileStream->InitializeFromFilename(texturePath.wstring().c_str(), GENERIC_READ)))
  {
    LOG_WARNING("Failed to load " + texturePath.string());
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