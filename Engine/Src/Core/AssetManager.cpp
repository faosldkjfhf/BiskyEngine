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

void AssetManager::Shutdown()
{
  for (auto &[_, geometry] : mGeometries)
  {
    geometry.reset();
  }
}

void AssetManager::DisposeUploaders()
{
  for (auto &[_, geometry] : mGeometries)
  {
    geometry->DisposeUploaders();
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
          vertices[index + initialVtx] = vertex;
        });
      }

      // TODO: TEXCOORDS, NORMAL
      auto normals = p.findAttribute("NORMAL");
      if (normals != p.attributes.end())
      {
        auto &accessor = asset->accessors[normals->accessorIndex];
        fastgltf::iterateAccessorWithIndex<XMFLOAT3>(asset.get(), accessor, [&](XMFLOAT3 normal, size_t index) {
          vertices[index + initialVtx].Normal = normal;
        });
      }

      newMesh.DrawArgs.push_back(submesh);
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

} // namespace Core