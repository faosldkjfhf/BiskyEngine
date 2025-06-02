#include "Common.hpp"

#include "Core/GameTimer.hpp"
#include "Core/ResourceManager.hpp"
#include "Graphics/Constants.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/ResourceUpload.hpp"
#include "Graphics/Utilities.hpp"
#include "Graphics/Window.hpp"
#include "Scene/Scene.hpp"
#include "Scene/ScreenQuad.hpp"

namespace bisky::scene
{

Scene::Scene(gfx::Window *const window, gfx::Device *const device, std::string_view name)
    : m_name(name), m_device(device), m_window(window)
{
    m_camera = std::make_unique<Camera>(window->getAspectRatio(), 0.1f, 100.0f);
    initDefaultScene();
    LOG_INFO("Scene " + std::string(name) + " created");
}

Scene::~Scene()
{
    m_renderObjects.clear();
    m_camera.reset();
}

void Scene::update(const core::GameTimer *const timer)
{
    m_camera->input(timer);
    auto *frameResource = m_device->getFrameResource();

    if (m_camera->getNumFramesDirty() > 0)
    {
        gfx::SceneBuffer sceneBuffer = {};
        XMStoreFloat4x4(&sceneBuffer.view, m_camera->getView());
        XMStoreFloat4x4(&sceneBuffer.projection, m_camera->getProjection());
        XMStoreFloat4x4(&sceneBuffer.viewProjection, m_camera->getView() * m_camera->getProjection());

        void *upload;
        frameResource->sceneBuffer->resource->Map(0, nullptr, &upload);
        memcpy(upload, &sceneBuffer, gfx::constantBufferByteSize(sizeof(gfx::SceneBuffer)));
        frameResource->sceneBuffer->resource->Unmap(0, nullptr);

        m_camera->cleanFrame();
    }
}

const std::vector<std::shared_ptr<RenderObject>> &Scene::getRenderObjects() const
{
    return m_renderObjects;
}

Camera *const Scene::getCamera() const
{
    return m_camera.get();
}

void Scene::initDefaultScene()
{
    if (core::ResourceManager::get().loadMesh(m_device, "DamagedHelmet.glb"))
    {
        auto ro  = m_renderObjects.emplace_back(std::make_shared<RenderObject>());
        ro->mesh = core::ResourceManager::get().getMesh("mesh_helmet_LP_13930damagedHelmet");
        ro->transform->setScale(2.0f, 2.0f, 2.0f);
        LOG_INFO("Added new render object");
    }
}

} // namespace bisky::scene