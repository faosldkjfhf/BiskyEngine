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
    m_sceneBuffer = std::make_unique<gfx::SceneBuffer>();
    m_camera      = std::make_unique<Camera>(window->getAspectRatio(), 0.1f, 100.0f);
    initDefaultScene();
    updateSceneBuffer();
    LOG_INFO("Scene " + std::string(name) + " created");
}

Scene::~Scene()
{
    m_sceneBuffer.reset();
    m_renderObjects.clear();
    m_camera.reset();
}

void Scene::update(const core::GameTimer *const timer)
{
    m_camera->input(timer);
    // updateSceneBuffer();
}

void Scene::updateSceneBuffer()
{
    // if (m_camera->getDirty())
    //{
    //     m_camera->updateViewMatrix();
    //     XMStoreFloat4x4(&m_sceneBuffer->view, m_camera->getView());
    //     XMStoreFloat4x4(&m_sceneBuffer->projection, m_camera->getProjection());
    //     XMStoreFloat4x4(&m_sceneBuffer->viewProjection, m_camera->getView() * m_camera->getProjection());
    //     XMStoreFloat4(&m_sceneBuffer->viewPosition, m_camera->getPosition());

    //    for (auto &frameResource : m_device->getFrameResources())
    //    {
    //        void *upload;
    //        frameResource->sceneBuffer->resource->Map(0, nullptr, &upload);
    //        memcpy(upload, m_sceneBuffer.get(), gfx::constantBufferByteSize(sizeof(gfx::SceneBuffer)));
    //        frameResource->sceneBuffer->resource->Unmap(0, nullptr);
    //    }
    //}
}

const std::vector<std::shared_ptr<RenderObject>> &Scene::getRenderObjects() const
{
    return m_renderObjects;
}

Camera *const Scene::getCamera() const
{
    return m_camera.get();
}

const std::vector<PointLight> &Scene::getLights() const
{
    return m_lights;
}

void Scene::initDefaultScene()
{
    m_camera->setPosition(0.0f, 0.0f, -5.0f);

    if (core::ResourceManager::get().loadMesh(m_device, "DamagedHelmet.glb"))
    {
        auto ro  = m_renderObjects.emplace_back(std::make_shared<RenderObject>());
        ro->mesh = core::ResourceManager::get().getMesh("mesh_helmet_LP_13930damagedHelmet");
        ro->transform->setScale(2.0f, 2.0f, 2.0f);
        LOG_INFO("Added new render object");
    }

    auto &light = m_lights.emplace_back();
    XMStoreFloat4(&light.position, math::FXMVECTOR{0.0f, 3.0f, -3.0f, 1.0f});
    XMStoreFloat4(&light.strength, math::FXMVECTOR{1.0f, 1.0f, 1.0f, 1.0f});
}

} // namespace bisky::scene