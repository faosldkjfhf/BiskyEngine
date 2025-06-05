#include "Common.hpp"

#include "Core/ResourceManager.hpp"
#include "Graphics/Device.hpp"
#include "Scene/Skybox.hpp"

namespace bisky::scene
{

Skybox::Skybox(gfx::Device *const device, const std::string_view name) : m_device(device)
{
    bool isCubemap = false;
    if (core::ResourceManager::get().loadDDS(device, name, &isCubemap))
    {
        if (isCubemap)
        {
            m_skybox = core::ResourceManager::get().getTexture(name);

            // create shader resource view
            m_skybox->srvDescriptor = device->getCbvSrvUavHeap()->allocate();

            D3D12_SHADER_RESOURCE_VIEW_DESC srv = {
                .Format                  = m_skybox->resource->GetDesc().Format,
                .ViewDimension           = D3D12_SRV_DIMENSION_TEXTURECUBE,
                .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
                .TextureCube =
                    {
                        .MostDetailedMip     = 0,
                        .MipLevels           = m_skybox->resource->GetDesc().MipLevels,
                        .ResourceMinLODClamp = 0.0f,
                    },
            };
            device->getDevice()->CreateShaderResourceView(m_skybox->resource.Get(), &srv, m_skybox->srvDescriptor.cpu);
        }
    }
}

Skybox::~Skybox()
{
    m_skybox.reset();
}

gfx::Texture *const Skybox::getTexture() const
{
    return m_skybox.get();
}

} // namespace bisky::scene