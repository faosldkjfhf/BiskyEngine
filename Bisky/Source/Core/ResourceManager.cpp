#include "Common.hpp"

#include "Core/ResourceManager.hpp"
#include "Graphics/Device.hpp"
#include "Scene/Material.hpp"
#include "Scene/Vertex.hpp"

namespace bisky::core
{

void _setWorkingDirectory(const std::filesystem::path &filepath)
{
    ResourceManager::get().setWorkingDirectory(filepath);
    ResourceManager::get().setShaderDirectory("Shaders");
    ResourceManager::get().setModelDirectory("Assets\\Models");
    ResourceManager::get().setTextureDirectory("Assets\\Textures");
}

void ResourceManager::reset()
{
    m_meshes.clear();
    m_textures.clear();
}

void ResourceManager::setWorkingDirectory(const std::filesystem::path &path)
{
    m_currentWorkingDirectory = path;
    LOG_INFO("Working directory set to " + m_currentWorkingDirectory.string());
}

void ResourceManager::setShaderDirectory(const std::filesystem::path &path)
{
    m_shaderDirectory = m_currentWorkingDirectory / path;
    LOG_INFO("Shader directory set to " + m_shaderDirectory.string());
}

void ResourceManager::setModelDirectory(const std::filesystem::path &path)
{
    m_modelDirectory = m_currentWorkingDirectory / path;
    LOG_INFO("Model directory set to " + m_modelDirectory.string());
}

void ResourceManager::setTextureDirectory(const std::filesystem::path &path)
{
    m_textureDirectory = m_currentWorkingDirectory / path;
    LOG_INFO("Texture directory set to " + m_textureDirectory.string());
}

bool ResourceManager::loadMesh(gfx::Device *const device, const std::filesystem::path &filename)
{
    const std::filesystem::path path = m_modelDirectory / filename;

    // -------------- use no flags if .glb, other load buffers --------------
    fastgltf::Options flags;
    if (path.extension() == ".glb")
        flags = fastgltf::Options::None;
    else
        flags = fastgltf::Options::LoadExternalBuffers;

    // -------------- attempt to load the file --------------
    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None)
    {
        LOG_WARNING("Failed to find " + path.string());
        return false;
    }

    // -------------- attempt to parse the .gltf or .glb file --------------
    fastgltf::Parser parser;
    auto             asset = parser.loadGltf(data.get(), path.parent_path(), flags);
    if (auto error = asset.error(); error != fastgltf::Error::None)
    {
        LOG_WARNING("Failed to parse " + path.string());
        return false;
    }

    // -------------- validate the file --------------
    auto error = fastgltf::validate(asset.get());
    if (error != fastgltf::Error::None)
    {
        LOG_WARNING("File is not a valid glTF file");
        return false;
    }

    // -------------- load textures --------------
    std::vector<std::shared_ptr<gfx::Texture>> textures;
    for (auto &image : asset->images)
    {
        std::shared_ptr<gfx::Texture> texture = std::make_shared<gfx::Texture>();
        std::visit(
            fastgltf::visitor(
                [&](auto &arg) {},
                [&](fastgltf::sources::BufferView &view) {
                    auto &bufferView = asset->bufferViews[view.bufferViewIndex];
                    auto &buffer     = asset->buffers[bufferView.bufferIndex];
                    std::visit(
                        fastgltf::visitor(
                            [&](auto &arg) {}, [&](fastgltf::sources::URI &filePath) {},
                            [&](fastgltf::sources::Array &array) {
                                auto texture = device->createImageFromMemory(
                                    reinterpret_cast<stbi_uc *>(array.bytes.data() + bufferView.byteOffset),
                                    array.bytes.size()
                                );

                                if (!texture)
                                {
                                    LOG_WARNING("Failed to create image from memory");
                                    return;
                                }

                                std::string name = image.name.data();
                                if (name.empty())
                                    name = "texture_" + std::to_string(m_textures.size());

                                m_textures[name] = texture;
                                textures.push_back(texture);
                                LOG_INFO("Loaded texture: " + name);
                            }
                        ),
                        buffer.data
                    );
                }
            ),
            image.data
        );
    }

    // -------------- create materials --------------
    std::vector<std::shared_ptr<scene::Material>> materials;
    for (auto &&mat : asset->materials)
    {
        std::shared_ptr<scene::Material> newMat = std::make_shared<scene::Material>();
        if (mat.pbrData.baseColorTexture.has_value())
        {
            size_t image = asset->textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
            newMat->diffuseTexture = textures[image].get();
        }
        if (mat.pbrData.metallicRoughnessTexture.has_value())
        {
            size_t image =
                asset->textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex].imageIndex.value();
            newMat->metallicRoughnessTexture = textures[image].get();
        }

        materials.push_back(newMat);
        m_materials[mat.name.data()] = newMat;
    }

    // -------------- load the vertices and indices --------------
    std::vector<scene::Vertex> vertices;
    std::vector<uint32_t>      indices;
    for (auto &&mesh : asset->meshes)
    {
        // -------------- reset vertices and indices --------------
        vertices.clear();
        indices.clear();

        // -------------- create a mesh --------------
        std::unique_ptr<scene::Mesh> newMesh = std::make_unique<scene::Mesh>();
        newMesh->name                        = mesh.name;
        newMesh->indexFormat                 = DXGI_FORMAT_R32_UINT;
        newMesh->vertexByteStride            = sizeof(scene::Vertex);

        // -------------- if mesh exists, skip over it --------------
        auto it = m_meshes.find(newMesh->name);
        if (it != m_meshes.end())
        {
            continue;
        }

        // -------------- load each primitive as a submesh --------------
        newMesh->submeshes.reserve(mesh.primitives.size());
        for (auto &&p : mesh.primitives)
        {
            auto indexAccessor = p.indicesAccessor.value();

            // -------------- add a submesh --------------
            scene::Submesh submesh;
            submesh.baseVertexLocation = static_cast<uint32_t>(vertices.size());
            submesh.startIndexLocation = static_cast<uint32_t>(indices.size());
            submesh.indexCount         = static_cast<uint32_t>(asset->accessors[indexAccessor].count);

            // -------------- add material --------------
            if (p.materialIndex.has_value())
            {
                submesh.material = materials[p.materialIndex.value()].get();
            }

            newMesh->submeshes.push_back(submesh);

            // -------------- get indices --------------
            indices.reserve(indices.size() + submesh.indexCount);
            fastgltf::iterateAccessor<uint32_t>(asset.get(), asset->accessors[indexAccessor], [&](uint32_t index) {
                indices.push_back(index + submesh.baseVertexLocation);
            });

            // -------------- get vertices --------------
            {
                auto &accessor = asset->accessors[p.findAttribute("POSITION")->accessorIndex];
                vertices.resize(vertices.size() + accessor.count);
                fastgltf::iterateAccessorWithIndex<math::XMFLOAT3>(
                    asset.get(), accessor,
                    [&](math::XMFLOAT3 position, size_t index) {
                        scene::Vertex v;
                        v.position = position;

                        vertices[submesh.baseVertexLocation + index] = v;
                    }
                );
            }

            // -------------- get normals --------------
            {
                auto normals = p.findAttribute("NORMAL");
                if (normals != p.attributes.end())
                {
                    auto &accessor = asset->accessors[normals->accessorIndex];
                    fastgltf::iterateAccessorWithIndex<math::XMFLOAT3>(
                        asset.get(), accessor,
                        [&](math::XMFLOAT3 normal, size_t index) {
                            vertices[submesh.baseVertexLocation + index].normal = normal;
                        }
                    );
                }
            }

            // -------------- get texcoords --------------
            {
                auto uvs = p.findAttribute("TEXCOORD_0");
                if (uvs != p.attributes.end())
                {
                    auto &accessor = asset->accessors[uvs->accessorIndex];
                    fastgltf::iterateAccessorWithIndex<math::XMFLOAT2>(
                        asset.get(), accessor,
                        [&](math::XMFLOAT2 texCoord, size_t index) {
                            vertices[submesh.baseVertexLocation + index].texCoord = texCoord;
                        }
                    );
                }
            }

            // -------------- get tangents --------------
            // TODO: calculate tangents if not included
            {
                auto tangents = p.findAttribute("TANGENT");
                if (tangents != p.attributes.end())
                {
                    auto &accessor = asset->accessors[tangents->accessorIndex];
                    fastgltf::iterateAccessorWithIndex<math::XMFLOAT4>(
                        asset.get(), accessor,
                        [&](math::XMFLOAT4 tangent, size_t index) {
                            vertices[submesh.baseVertexLocation + index].tangent = tangent;
                        }
                    );
                }
            }
        }

        // -------------- begin resource upload block --------------
        gfx::ResourceUpload upload(device);
        upload.Begin();

        // -------------- create upload buffers --------------
        newMesh->vertexBufferByteSize = static_cast<uint32_t>(vertices.size()) * sizeof(scene::Vertex);
        newMesh->indexBufferByteSize  = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);
        newMesh->vertexBuffer         = device->createUploadBuffer(newMesh->vertexBufferByteSize, vertices.data());
        newMesh->indexBuffer          = device->createUploadBuffer(newMesh->indexBufferByteSize, indices.data());

        // -------------- create shader resource view --------------
        newMesh->vertexBuffer->srvDescriptor = device->getCbvSrvUavHeap()->allocate();
        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {
            .Format                  = DXGI_FORMAT_UNKNOWN,
            .ViewDimension           = D3D12_SRV_DIMENSION_BUFFER,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Buffer =
                {
                    .FirstElement        = 0,
                    .NumElements         = static_cast<UINT>(vertices.size()),
                    .StructureByteStride = sizeof(scene::Vertex),
                    .Flags               = D3D12_BUFFER_SRV_FLAG_NONE,
                },
        };
        device->createShaderResourceView(newMesh->vertexBuffer.get(), &desc);

        // -------------- end resource upload block --------------
        auto finish = upload.Finish();
        finish.wait();

        // -------------- mesh loaded successfully --------------
        LOG_INFO("Loaded mesh: " + newMesh->name);
        m_meshes[newMesh->name] = std::move(newMesh);
    }

    LOG_INFO("Loaded " + path.string());
    return true;
}

const std::string ResourceManager::addMesh(std::unique_ptr<scene::Mesh> mesh)
{
    auto it = m_meshes.find(mesh->name);
    if (it != m_meshes.end())
    {
        LOG_WARNING("Mesh " + std::string(mesh->name) + " is already loaded.");
        return mesh->name;
    }

    // TODO: This is convoluted, maybe theres a better way of doing this
    LOG_INFO("Mesh added: " + mesh->name);
    std::string name     = mesh->name;
    m_meshes[mesh->name] = std::move(mesh);
    return name;
}

scene::Mesh *const ResourceManager::getMesh(std::string_view name)
{
    auto it = m_meshes.find(name);
    if (it == m_meshes.end())
    {
        LOG_WARNING("Mesh " + std::string(name) + " not found");
        return nullptr;
    }

    LOG_VERBOSE("Found mesh " + std::string(name));
    return it->second.get();
}

gfx::Texture *const ResourceManager::getTexture(std::string_view name)
{
    auto it = m_textures.find(std::string(name));
    if (it == m_textures.end())
    {
        LOG_WARNING("Texture " + std::string(name) + " not found");
        return nullptr;
    }

    LOG_VERBOSE("Found texture " + std::string(name));
    return it->second.get();
}

const std::filesystem::path &ResourceManager::getWorkingDirectory() const
{
    return m_currentWorkingDirectory;
}

const std::filesystem::path &ResourceManager::getShaderDirectory() const
{
    return m_shaderDirectory;
}

const std::filesystem::path &ResourceManager::getTextureDirectory() const
{
    return m_textureDirectory;
}

} // namespace bisky::core