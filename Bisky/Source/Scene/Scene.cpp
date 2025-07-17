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
    m_camera        = std::make_unique<Camera>(window->getAspectRatio(), 0.1f, 100.0f);
    m_arcballCamera = std::make_unique<ArcballCamera>(window->getWidth(), window->getHeight());

    LOG_INFO(core::float4(m_arcballCamera->getRight()));
    LOG_INFO(core::float4(m_arcballCamera->getUp()));
    LOG_INFO(core::float4(m_arcballCamera->getForward()));
    LOG_INFO(core::float4(m_arcballCamera->getPosition()));

    initDefaultScene();
    LOG_INFO("Scene " + std::string(name) + " created");
}

Scene::~Scene()
{
    m_skybox.reset();
    m_renderObjects.clear();
    m_arcballCamera.reset();
    m_camera.reset();
}

void Scene::update(const core::GameTimer *const timer)
{
    // m_camera->input(timer);
    // m_camera->updateViewMatrix();
}

void Scene::draw()
{
    dx::XMFLOAT3 position;
    XMStoreFloat3(&position, m_arcballCamera->getPosition());
    auto &renderObjects = getRenderObjects();
    auto &lights        = getLights();

    ImGui::Begin("Scene");
    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SeparatorText("Position");
        {
            if (ImGui::SliderFloat("X", (float *)&position.x, -10.0f, 10.0f))
            {
                // camera->setPosition(position.x, position.y, position.z);
            }

            if (ImGui::SliderFloat("Y", (float *)&position.y, -10.0f, 10.0f))
            {
                // camera->setPosition(position.x, position.y, position.z);
            }

            if (ImGui::SliderFloat("Z", (float *)&position.z, -10.0f, 10.0f))
            {
                // camera->setPosition(position.x, position.y, position.z);
            }
        }

        ImGui::SeparatorText("View Matrix");
        {
            /*
             * Display as row-major.
             */
            dx::XMFLOAT4X4 view;
            XMStoreFloat4x4(&view, m_arcballCamera->getView());

            ImGui::Text("%f %f %f %f", view._11, view._12, view._13, view._14);
            ImGui::Text("%f %f %f %f", view._21, view._22, view._23, view._24);
            ImGui::Text("%f %f %f %f", view._31, view._32, view._33, view._34);
            ImGui::Text("%f %f %f %f", view._41, view._42, view._43, view._44);
        }
    }
    if (ImGui::CollapsingHeader("Objects", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (auto &object : renderObjects)
        {
            auto scale       = object->transform->getScale3f();
            auto rotation    = object->transform->getRotation3f();
            auto translation = object->transform->getTranslation3f();

            if (ImGui::TreeNode(object->name.c_str()))
            {
                ImGui::Unindent();
                if (ImGui::SliderFloat3("Position", (float *)&translation, -10.0f, 10.0f))
                    object->transform->setTranslation(translation.x, translation.y, translation.z);
                if (ImGui::SliderFloat3("Rotation", (float *)&rotation, -180.0f, 180.0f))
                    object->transform->setRotation(rotation.x, rotation.y, rotation.z);
                if (ImGui::SliderFloat3("Scale", (float *)&scale, 0.0f, 100.0f))
                    object->transform->setScale(scale.x, scale.y, scale.z);
                ImGui::Indent();
                ImGui::TreePop();
            }
        }
    }
    if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Text("Number of lights: %i", lights.size());
        for (auto &light : lights)
        {
            if (ImGui::TreeNode("Light"))
            {
                ImGui::Unindent();
                ImGui::SliderFloat3("Position", (float *)&light.position, -10.0f, 10.0f);
                ImGui::ColorEdit3("Strength", (float *)&light.strength);
                ImGui::Indent();
                ImGui::TreePop();
            }
        }
    }
    ImGui::End();
}

const std::vector<std::shared_ptr<RenderObject>> &Scene::getRenderObjects() const
{
    return m_renderObjects;
}

Camera *const Scene::getCamera() const
{
    return m_camera.get();
}

ArcballCamera *const Scene::getArcballCamera() const
{
    return m_arcballCamera.get();
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
        ro->transform->setScale(1.0f, 1.0f, 1.0f);
        ro->transform->setRotation(90.0f, 0.0f, 180.0f);
        LOG_INFO("Added new render object");
    }

    auto &light = m_lights.emplace_back();
    XMStoreFloat4(&light.position, dx::FXMVECTOR{0.0f, 3.0f, -3.0f, 1.0f});
    XMStoreFloat4(&light.strength, dx::FXMVECTOR{1.0f, 1.0f, 1.0f, 1.0f});

    m_skybox = std::make_unique<Skybox>(m_device, "Skybox\\cubemap.dds");
}

} // namespace bisky::scene