#pragma once

#include "Common.h"
#include "Core/Material.h"
#include "Core/MeshGeometry.h"
#include "DX12/Texture.h"

namespace Core
{

class AssetManager
{
public:
  enum Error
  {
    None,
    LoadFile,
    ParseFile
  };

  void Shutdown();
  void DisposeUploaders();

  void SetCurrentWorkingDirectory(const std::filesystem::path &cwd);
  void SetShaderDirectory(const std::filesystem::path &path);
  void SetModelDirectory(const std::filesystem::path &path);
  void SetTextureDirectory(const std::filesystem::path &path);

  // GLTF
  Error LoadGLTF(const std::filesystem::path &filename, ID3D12GraphicsCommandList10 *cmdList,
                 fastgltf::Options flags = fastgltf::Options::None);
  Ref<Core::MeshGeometry> GetModel(std::string_view name);
  void LoadImage(ID3D12GraphicsCommandList10 *cmdList, fastgltf::Asset &asset, fastgltf::Image &image);
  Ref<DX12::Texture> GetTexture(std::string_view name);

  // Shaders
  ComPtr<ID3DBlob> LoadBinary(const std::filesystem::path &filename);
  ComPtr<ID3DBlob> CompileShader(const std::filesystem::path &filename, const D3D_SHADER_MACRO *defines, LPCSTR target,
                                 LPCSTR entryPoint = "main");

  // Materials
  Ref<Core::Material> AddMaterial(std::string_view name);
  Ref<Core::Material> GetMaterial(std::string_view name);
  inline UINT NumMaterials() const
  {
    return static_cast<UINT>(mMaterials.size());
  }

  inline std::unordered_map<std::string_view, Ref<Core::Material>> &Materials()
  {
    return mMaterials;
  }

private:
  bool LoadImageFromDisk(const std::filesystem::path &filepath, DX12::Texture::ImageData &data);
  bool LoadImageFromMemory(unsigned char *bytes, size_t byteOffset, size_t bufferSize, DX12::Texture::ImageData &data);

  std::filesystem::path mCurrentWorkingDirectory = std::filesystem::absolute(__FILE__).parent_path();
  std::filesystem::path mShaderDirectory = mCurrentWorkingDirectory / "Shaders";
  std::filesystem::path mTextureDirectory = mCurrentWorkingDirectory / "Textures";
  std::filesystem::path mModelDirectory = mCurrentWorkingDirectory / "Models";

  std::unordered_map<std::string_view, Ref<Core::MeshGeometry>> mGeometries{};
  std::unordered_map<std::string_view, Ref<Core::Material>> mMaterials{};
  std::unordered_map<std::string_view, Ref<DX12::Texture>> mTextures{};

  // FIXME: Change the names
  static const std::vector<DX12::Texture::GUIDToDXGI> mLookupTable;
  static const std::vector<DX12::Texture::GUIDToGUID> mFixLookupTable;

public:
  AssetManager(const AssetManager &) = delete;
  const AssetManager &operator=(const AssetManager &) = delete;
  inline static AssetManager &Get()
  {
    static AssetManager instance;
    return instance;
  }

private:
  AssetManager() = default;
};

} // namespace Core