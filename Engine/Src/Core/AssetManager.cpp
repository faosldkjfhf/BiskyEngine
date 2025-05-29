#include "Common.h"

#include "Core/AssetManager.h"
#include "Core/Logger.h"
#include "Core/Vertex.h"
#include "D3D12/Context.h"
#include "D3D12/Initializers.h"
#include "D3D12/Utilities.h"
#include <fstream>

namespace Core
{

const std::vector<D3D12::Texture::GUIDToDXGI> AssetManager::mLookupTable = {
    {GUID_WICPixelFormat128bppRGBAFloat, DXGI_FORMAT_R32G32B32A32_FLOAT},

    {GUID_WICPixelFormat64bppRGBAHalf, DXGI_FORMAT_R16G16B16A16_FLOAT},
    {GUID_WICPixelFormat64bppRGBA, DXGI_FORMAT_R16G16B16A16_UNORM},

    {GUID_WICPixelFormat32bppRGBA, DXGI_FORMAT_R8G8B8A8_UNORM},
    {GUID_WICPixelFormat32bppBGRA, DXGI_FORMAT_B8G8R8A8_UNORM}, // DXGI 1.1
    {GUID_WICPixelFormat32bppBGR, DXGI_FORMAT_B8G8R8X8_UNORM},  // DXGI 1.1

    {GUID_WICPixelFormat32bppRGBA1010102XR, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM}, // DXGI 1.1
    {GUID_WICPixelFormat32bppRGBA1010102, DXGI_FORMAT_R10G10B10A2_UNORM},
    {GUID_WICPixelFormat32bppRGBE, DXGI_FORMAT_R9G9B9E5_SHAREDEXP},

#ifdef DXGI_1_2_FORMATS

    {GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT_B5G5R5A1_UNORM},
    {GUID_WICPixelFormat16bppBGR565, DXGI_FORMAT_B5G6R5_UNORM},

#endif // DXGI_1_2_FORMATS

    {GUID_WICPixelFormat32bppGrayFloat, DXGI_FORMAT_R32_FLOAT},
    {GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT_R16_FLOAT},
    {GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R16_UNORM},
    {GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM},

    {GUID_WICPixelFormat8bppAlpha, DXGI_FORMAT_A8_UNORM},

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
    {GUID_WICPixelFormat96bppRGBFloat, DXGI_FORMAT_R32G32B32_FLOAT},
#endif
};

const std::vector<D3D12::Texture::GUIDToGUID> AssetManager::mFixLookupTable = {
    // Note target GUID in this conversion table must be one of those directly supported formats (above).

    {GUID_WICPixelFormatBlackWhite, GUID_WICPixelFormat8bppGray}, // DXGI_FORMAT_R8_UNORM

    {GUID_WICPixelFormat1bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
    {GUID_WICPixelFormat2bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
    {GUID_WICPixelFormat4bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
    {GUID_WICPixelFormat8bppIndexed, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM

    {GUID_WICPixelFormat2bppGray, GUID_WICPixelFormat8bppGray}, // DXGI_FORMAT_R8_UNORM
    {GUID_WICPixelFormat4bppGray, GUID_WICPixelFormat8bppGray}, // DXGI_FORMAT_R8_UNORM

    {GUID_WICPixelFormat16bppGrayFixedPoint, GUID_WICPixelFormat16bppGrayHalf},  // DXGI_FORMAT_R16_FLOAT
    {GUID_WICPixelFormat32bppGrayFixedPoint, GUID_WICPixelFormat32bppGrayFloat}, // DXGI_FORMAT_R32_FLOAT

#ifdef DXGI_1_2_FORMATS

    {GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat16bppBGRA5551}, // DXGI_FORMAT_B5G5R5A1_UNORM

#else

    {GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM
    {GUID_WICPixelFormat16bppBGRA5551, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
    {GUID_WICPixelFormat16bppBGR565, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM

#endif // DXGI_1_2_FORMATS

    {GUID_WICPixelFormat32bppBGR101010, GUID_WICPixelFormat32bppRGBA1010102}, // DXGI_FORMAT_R10G10B10A2_UNORM

    {GUID_WICPixelFormat24bppBGR, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM
    {GUID_WICPixelFormat24bppRGB, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM
    {GUID_WICPixelFormat32bppPBGRA, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
    {GUID_WICPixelFormat32bppPRGBA, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM

    {GUID_WICPixelFormat48bppRGB, GUID_WICPixelFormat64bppRGBA},   // DXGI_FORMAT_R16G16B16A16_UNORM
    {GUID_WICPixelFormat48bppBGR, GUID_WICPixelFormat64bppRGBA},   // DXGI_FORMAT_R16G16B16A16_UNORM
    {GUID_WICPixelFormat64bppBGRA, GUID_WICPixelFormat64bppRGBA},  // DXGI_FORMAT_R16G16B16A16_UNORM
    {GUID_WICPixelFormat64bppPRGBA, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM
    {GUID_WICPixelFormat64bppPBGRA, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM

    {GUID_WICPixelFormat48bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},  // DXGI_FORMAT_R16G16B16A16_FLOAT
    {GUID_WICPixelFormat48bppBGRFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},  // DXGI_FORMAT_R16G16B16A16_FLOAT
    {GUID_WICPixelFormat64bppRGBAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
    {GUID_WICPixelFormat64bppBGRAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
    {GUID_WICPixelFormat64bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf},  // DXGI_FORMAT_R16G16B16A16_FLOAT
    {GUID_WICPixelFormat64bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf},        // DXGI_FORMAT_R16G16B16A16_FLOAT
    {GUID_WICPixelFormat48bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf},        // DXGI_FORMAT_R16G16B16A16_FLOAT

    {GUID_WICPixelFormat96bppRGBFixedPoint, GUID_WICPixelFormat128bppRGBAFloat},   // DXGI_FORMAT_R32G32B32A32_FLOAT
    {GUID_WICPixelFormat128bppPRGBAFloat, GUID_WICPixelFormat128bppRGBAFloat},     // DXGI_FORMAT_R32G32B32A32_FLOAT
    {GUID_WICPixelFormat128bppRGBFloat, GUID_WICPixelFormat128bppRGBAFloat},       // DXGI_FORMAT_R32G32B32A32_FLOAT
    {GUID_WICPixelFormat128bppRGBAFixedPoint, GUID_WICPixelFormat128bppRGBAFloat}, // DXGI_FORMAT_R32G32B32A32_FLOAT
    {GUID_WICPixelFormat128bppRGBFixedPoint, GUID_WICPixelFormat128bppRGBAFloat},  // DXGI_FORMAT_R32G32B32A32_FLOAT

    {GUID_WICPixelFormat32bppCMYK, GUID_WICPixelFormat32bppRGBA},      // DXGI_FORMAT_R8G8B8A8_UNORM
    {GUID_WICPixelFormat64bppCMYK, GUID_WICPixelFormat64bppRGBA},      // DXGI_FORMAT_R16G16B16A16_UNORM
    {GUID_WICPixelFormat40bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM
    {GUID_WICPixelFormat80bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA}, // DXGI_FORMAT_R16G16B16A16_UNORM

#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
    {GUID_WICPixelFormat32bppRGB, GUID_WICPixelFormat32bppRGBA},           // DXGI_FORMAT_R8G8B8A8_UNORM
    {GUID_WICPixelFormat64bppRGB, GUID_WICPixelFormat64bppRGBA},           // DXGI_FORMAT_R16G16B16A16_UNORM
    {GUID_WICPixelFormat64bppPRGBAHalf, GUID_WICPixelFormat64bppRGBAHalf}, // DXGI_FORMAT_R16G16B16A16_FLOAT
#endif
};

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

Ref<D3D12::Texture> AssetManager::GetTexture(std::string_view name)
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

// TODO: Make helper methods, too long right now
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

    geo->VertexBufferGPU = D3D12::CreateBuffer(cmdList, geo->VertexBufferCPU, geo->VertexBufferUploader);
    geo->IndexBufferGPU = D3D12::CreateBuffer(cmdList, geo->IndexBufferCPU, geo->IndexBufferUploader);

    mGeometries[geo->Name] = geo;
    LOG_INFO("Model added " + geo->Name);

    auto material = AddMaterial(geo->Name);
    material->NoTexture = false;
    {
      aiString path;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_BASE_COLOR, 0, &path);
      D3D12::Texture::ImageData imageData;

      // albedo map
      auto *embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());
      if (embeddedTexture)
      {
        if (embeddedTexture->mHeight == 0)
        {
          if (LoadImageFromMemory(reinterpret_cast<unsigned char *>(embeddedTexture->pcData), 0,
                                  embeddedTexture->mWidth, imageData))
          {
            Ref<D3D12::Texture> texture = D3D12::CreateTexture(cmdList, imageData);
            texture->Name = path.C_Str();
            texture->CreateView();
            mTextures[texture->Name] = texture;
            material->DiffuseMapHeapIndex = texture->HeapIndex;
          }
        }
      }
      else
      {
        if (LoadImageFromDisk(path.C_Str(), imageData))
        {
          Ref<D3D12::Texture> texture = D3D12::CreateTexture(cmdList, imageData);
          texture->Name = path.C_Str();
          texture->CreateView();
          mTextures[texture->Name] = texture;
          material->DiffuseMapHeapIndex = texture->HeapIndex;
        }
      }
    }

    // normal map
    {
      aiString path;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_NORMALS, 0, &path);
      D3D12::Texture::ImageData imageData;

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

          Ref<D3D12::Texture> texture = D3D12::CreateTexture(cmdList, imageData);
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
          Ref<D3D12::Texture> texture = D3D12::CreateTexture(cmdList, imageData);
          texture->Name = path.C_Str();
          texture->CreateView();
          mTextures[texture->Name] = texture;
          material->NormalMapHeapIndex = texture->HeapIndex;
        }
      }
    }

    // ao map
    {
      aiString path;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &path);
      D3D12::Texture::ImageData imageData;

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

          Ref<D3D12::Texture> texture = D3D12::CreateTexture(cmdList, imageData);
          texture->Name = path.C_Str();
          texture->CreateView();
          mTextures[texture->Name] = texture;
          material->AmbientOcclusionMapHeapIndex = texture->HeapIndex;
        }
      }
      else
      {
        if (LoadImageFromDisk(path.C_Str(), imageData))
        {
          Ref<D3D12::Texture> texture = D3D12::CreateTexture(cmdList, imageData);
          texture->Name = path.C_Str();
          texture->CreateView();
          mTextures[texture->Name] = texture;
          material->AmbientOcclusionMapHeapIndex = texture->HeapIndex;
        }
      }
    }

    // metal roughness map
    {
      aiString path;
      scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_UNKNOWN, 0, &path);
      D3D12::Texture::ImageData imageData;

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

          Ref<D3D12::Texture> texture = D3D12::CreateTexture(cmdList, imageData);
          texture->Name = path.C_Str();
          texture->CreateView();
          mTextures[texture->Name] = texture;
          material->MetalRoughnessMapHeapIndex = texture->HeapIndex;
        }
      }
      else
      {
        if (LoadImageFromDisk(path.C_Str(), imageData))
        {
          Ref<D3D12::Texture> texture = D3D12::CreateTexture(cmdList, imageData);
          texture->Name = path.C_Str();
          texture->CreateView();
          mTextures[texture->Name] = texture;
          material->MetalRoughnessMapHeapIndex = texture->HeapIndex;
        }
      }
    }
  }

  return None;
}

AssetManager::Error AssetManager::LoadCubeMap(const std::filesystem::path &filename)
{
  std::filesystem::path texturePath = mTextureDirectory / filename;
  Ref<D3D12::Texture> texture = MakeRef<D3D12::Texture>();
  texture->Name = filename.string();
  ResourceUploadBatch upload(D3D12::Context::Get().Device().Get());
  bool isCubemap;

  upload.Begin();

  if (FAILED(CreateDDSTextureFromFile(D3D12::Context::Get().Device().Get(), upload, texturePath.c_str(),
                                      &texture->Resource, false, 0, nullptr, &isCubemap)))
  {
    LOG_WARNING("Failed to load " + texture->Name);
    return LoadFile;
  }

  if (!isCubemap)
  {
    LOG_WARNING(texture->Name + " is not a cubemap");
    return LoadFile;
  }

  auto finish = upload.End(D3D12::Context::Get().CommandQueue().Get());
  finish.wait();

  mTextures[texture->Name] = texture;

  // create SRV
  texture->HeapIndex = D3D12::Context::Get().ShaderResourceHeap()->AddDescriptor();
  auto handle = D3D12::Context::Get().ShaderResourceHeap()->CPUDescriptorHandle();
  handle.ptr += texture->HeapIndex * D3D12::Context::Get().CbvSrvUavDescriptorSize();

  D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
  srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
  srv.TextureCube.MipLevels = texture->Resource->GetDesc().MipLevels;
  srv.TextureCube.MostDetailedMip = 0;
  srv.TextureCube.ResourceMinLODClamp = 0.0f;
  D3D12::Context::Get().Device()->CreateShaderResourceView(texture->Resource.Get(), &srv, handle);

  LOG_INFO("Loaded " + texture->Name);
  return None;
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
bool AssetManager::LoadImageFromDisk(const std::filesystem::path &filepath, D3D12::Texture::ImageData &data)
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

  auto it = std::find_if(mLookupTable.begin(), mLookupTable.end(), [&](const D3D12::Texture::GUIDToDXGI &entry) {
    return memcmp(&entry.GUID, &pixelFormat, sizeof(GUID)) == 0;
  });
  if (it == mLookupTable.end())
  {
    auto fix =
        std::find_if(mFixLookupTable.begin(), mFixLookupTable.end(), [&](const D3D12::Texture::GUIDToGUID &entry) {
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
  it = std::find_if(mLookupTable.begin(), mLookupTable.end(), [&](const D3D12::Texture::GUIDToDXGI &entry) {
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
                                       D3D12::Texture::ImageData &data)
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

  auto it = std::find_if(mLookupTable.begin(), mLookupTable.end(), [&](const D3D12::Texture::GUIDToDXGI &entry) {
    return memcmp(&entry.GUID, &pixelFormat, sizeof(GUID)) == 0;
  });
  if (it == mLookupTable.end())
  {
    auto fix =
        std::find_if(mFixLookupTable.begin(), mFixLookupTable.end(), [&](const D3D12::Texture::GUIDToGUID &entry) {
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
  it = std::find_if(mLookupTable.begin(), mLookupTable.end(), [&](const D3D12::Texture::GUIDToDXGI &entry) {
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