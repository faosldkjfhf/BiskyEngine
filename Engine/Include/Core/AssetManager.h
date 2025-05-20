#pragma once

#include "Common.h"
#include "Core/MeshGeometry.h"

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

  Error LoadGLTF(const std::filesystem::path &filename, ID3D12GraphicsCommandList10 *cmdList);
  ComPtr<ID3DBlob> LoadBinary(const std::filesystem::path &filename);
  ComPtr<ID3DBlob> CompileShader(const std::filesystem::path &filename, const D3D_SHADER_MACRO *defines, LPCSTR target,
                                 LPCSTR entryPoint = "main");

  Ref<Core::MeshGeometry> GetModel(std::string_view name);

private:
  std::filesystem::path mCurrentWorkingDirectory = std::filesystem::absolute(__FILE__).parent_path();
  std::filesystem::path mShaderDirectory = mCurrentWorkingDirectory / "Shaders";
  std::filesystem::path mTextureDirectory = mCurrentWorkingDirectory / "Textures";
  std::filesystem::path mModelDirectory = mCurrentWorkingDirectory / "Models";

  std::unordered_map<std::string_view, Ref<Core::MeshGeometry>> mGeometries{};

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
