#pragma once

#include "Graphics/GraphicsPipeline.hpp"
#include <memory>

namespace bisky::core
{
struct FrameStats;
}

namespace bisky::gfx
{
class Device;
class Window;
struct Texture;
} // namespace bisky::gfx

namespace bisky::scene
{
class Scene;
}

namespace bisky::renderer
{

struct GBuffer
{
    std::unique_ptr<gfx::Texture> positionAmbientOcclusion;
    std::unique_ptr<gfx::Texture> normalRoughness;
    std::unique_ptr<gfx::Texture> albedoMetallic;
    std::unique_ptr<gfx::Texture> emissive;
};

class DeferredRenderer
{
  public:
    struct RenderResource
    {
        int vertexBufferIndex;
        int diffuseMapIndex;
        int metallicRoughnessMapIndex;
        int ambientOcclusionMapIndex;
        int emissiveMapIndex;
    };

    struct LightPassRenderResource
    {
        int vertexBufferIndex;
        int positionMapIndex;
        int normalMapIndex;
        int albedoMapIndex;
        int emissiveMapIndex;
    };

    explicit DeferredRenderer(gfx::Device *const device, gfx::Window *const window);
    ~DeferredRenderer();

  public:
    auto geometryPass(
        scene::Scene *const scene, gfx::FrameResource *const frameResource, core::FrameStats *const frameStats
    ) const -> void;
    auto lightPass(
        scene::Scene *const scene, gfx::FrameResource *const frameResource, core::FrameStats *const frameStats
    ) const -> void;
    auto resize(gfx::Window *const window) -> void;

  public:
    auto getGBuffer() -> GBuffer *;

  private:
    auto initGBuffer(gfx::Window *const window) -> void;

  private:
    std::unique_ptr<gfx::GraphicsPipeline> m_graphicsPipeline;
    std::unique_ptr<gfx::GraphicsPipeline> m_lightPassPipeline;
    std::unique_ptr<scene::RenderObject>   m_quad;
    gfx::Device *const                     m_device;
    GBuffer                                m_gBuffer;
};

} // namespace bisky::renderer