#include "Common.hpp"

#include "Graphics/Device.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/DeferredRenderer.hpp"

namespace bisky::renderer
{

DeferredRenderer::DeferredRenderer(gfx::Device *const device, gfx::Window *const window) : m_device(device)
{
    // ----- position texture -----
    m_gBuffer.position = device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );

    // ----- normal texture -----
    m_gBuffer.normals = device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );

    // ----- albedo texture -----
    m_gBuffer.albedo = device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );

    // ----- specular texture -----
    m_gBuffer.specular = device->createTexture2D(
        window->getWidth(), window->getHeight(), DXGI_FORMAT_R16G16B16A16_FLOAT, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    );
}

DeferredRenderer::~DeferredRenderer()
{
}

} // namespace bisky::renderer