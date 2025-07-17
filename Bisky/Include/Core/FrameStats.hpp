#pragma once

#include "Editor/IRenderable.hpp"
#include <cstdint>
#include <imgui.h>

namespace bisky::core
{

struct FrameStats : editor::IRenderable
{
    float    frameTime;
    uint32_t triangleCount;
    uint32_t drawCount;
    float    sceneUpdateTime;
    float    meshDrawTime;
    float    finalRenderDrawTime;

    inline virtual void draw() override
    {
        ImGui::Begin("Debug");
        ImGui::Text("Triangle Count: %i", triangleCount);
        ImGui::Text("Draw Count: %i", drawCount);
        ImGui::Text("Frame Time: %f", frameTime);
        ImGui::Text("Scene Update Time: %f", sceneUpdateTime);
        ImGui::Text("Mesh Draw Time: %f", meshDrawTime);
        ImGui::Text("Final Render Draw Time: %f", finalRenderDrawTime);
        ImGui::End();
    }
};

} // namespace bisky::core