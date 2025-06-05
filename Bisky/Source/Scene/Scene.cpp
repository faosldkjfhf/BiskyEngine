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
    m_skybox.reset();
    m_renderObjects.clear();
    m_camera.reset();
}

void Scene::update(const core::GameTimer *const timer)
{
    m_camera->input(timer);
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

Skybox *const Scene::getSkybox() const
{
    return m_skybox.get();
}

void Scene::initDefaultScene()
{
    m_camera->setPosition(0.0f, 0.0f, -5.0f);

    if (core::ResourceManager::get().loadMesh(m_device, "DamagedHelmet.glb"))
    {
        auto ro  = m_renderObjects.emplace_back(std::make_shared<RenderObject>());
        ro->mesh = core::ResourceManager::get().getMesh("mesh_helmet_LP_13930damagedHelmet");
        ro->transform->setScale(2.0f, 2.0f, 2.0f);
        ro->transform->setRotation(90.0f, 0.0f, 180.0f);
        LOG_INFO("Added new render object");
    }

    auto &light = m_lights.emplace_back();
    XMStoreFloat4(&light.position, dx::FXMVECTOR{0.0f, 3.0f, -3.0f, 1.0f});
    XMStoreFloat4(&light.strength, dx::FXMVECTOR{1.0f, 1.0f, 1.0f, 1.0f});

    m_skybox = std::make_unique<Skybox>(m_device, "Skybox\\cubemap.dds");
}

} // namespace bisky::scene