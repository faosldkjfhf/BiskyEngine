#pragma once

#include "Graphics/Texture.hpp"
#include "Scene/Material.hpp"
#include "Scene/Mesh.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>

namespace bisky::gfx
{
class Device;
}

namespace bisky::core
{

/*
 * static namespace variables.
 */

/*
 * Sets the current working directory to the given filepath.
 * This should not be called by the user, instead use the macro
 * SET_WORKING_DIRECTORY from your main() function.
 *
 * @param filepath The path to designate as the working directory.
 */
void _setWorkingDirectory(const std::filesystem::path &filepath);

/*
 * A basic resource manager class.
 *
 * TODO: Everything.
 */
class ResourceManager
{
  public:
    /*
     * Singleton pattern initializer and getter.
     * Use this to get an instance of the global ResourceManager.
     *
     * @return A reference to the global ResourceManager.
     */
    inline static ResourceManager &get()
    {
        static ResourceManager instance;
        return instance;
    }

    ~ResourceManager()                                          = default;
    ResourceManager(const ResourceManager &)                    = delete;
    const ResourceManager &operator=(const ResourceManager &)   = delete;
    ResourceManager(const ResourceManager &&)                   = delete;
    const ResourceManager &&operator=(const ResourceManager &&) = delete;

  public: // Public functions
    /*
     * Resets stored resources.
     * This MUST be called before exiting.
     */
    void reset();

    /*
     * Sets the working directory that will be used.
     *
     * @param path The path to set.
     */
    void setWorkingDirectory(const std::filesystem::path &path);

    /*
     * Sets the shader directory that will be used.
     * The path will be appended to the current working directory.
     *
     * @param path The path to set.
     */
    void setShaderDirectory(const std::filesystem::path &path);

    /*
     * Sets the model directory that will be used.
     * The path will be appended to the current working directory.
     *
     * @param path The path to set.
     */
    void setModelDirectory(const std::filesystem::path &path);

    /*
     * Sets the texture directory that will be used.
     * The path will be appended to the current working directory.
     *
     * @param path The path to set.
     */
    void setTextureDirectory(const std::filesystem::path &path);

    /*
     * Loads a mesh from a given filename.
     * Assumes the file is in the ShaderDirectory.
     *
     * @param device The device to create buffers with.
     * @param filename The file to load.
     * @return True if successfully loaded.
     */
    bool loadMesh(gfx::Device *const device, const std::filesystem::path &filename);

    bool loadDDS(gfx::Device *const device, const std::filesystem::path &filename, bool *isCubemap);

    /*
     * Adds a mesh from an already created mesh.
     * Returns the name of the mesh.
     * It's up to the user to call getMesh in order to
     * check if the mesh was added successfully or not.
     *
     * @param mesh The fully loaded mesh.
     * @return The name of the mesh.
     */
    const std::string addMesh(std::unique_ptr<scene::Mesh> mesh);

  public: // Getter functions
    scene::Mesh *const            getMesh(std::string_view name);
    std::shared_ptr<gfx::Texture> getTexture(std::string_view name);
    const std::filesystem::path  &getWorkingDirectory() const;
    const std::filesystem::path  &getShaderDirectory() const;
    const std::filesystem::path  &getTextureDirectory() const;

  private:
    explicit ResourceManager() = default;

    std::unordered_map<std::string_view, std::unique_ptr<scene::Mesh>> m_meshes;
    std::unordered_map<std::string, std::shared_ptr<gfx::Texture>>     m_textures;
    std::unordered_map<std::string, std::shared_ptr<scene::Material>>  m_materials;

    std::filesystem::path m_currentWorkingDirectory;
    std::filesystem::path m_shaderDirectory;
    std::filesystem::path m_modelDirectory;
    std::filesystem::path m_textureDirectory;
};

} // namespace bisky::core

/*
 * Call this macro from your main() function to set the working directory correctly.
 */
#define SET_DEFAULT_WORKING_DIRECTORY()                                                                                \
    bisky::core::_setWorkingDirectory(std::filesystem::absolute(__FILE__).parent_path());