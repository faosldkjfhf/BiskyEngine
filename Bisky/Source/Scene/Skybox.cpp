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
            m_device->createShaderResourceView(m_skybox.get(), D3D12_SRV_DIMENSION_TEXTURECUBE);
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