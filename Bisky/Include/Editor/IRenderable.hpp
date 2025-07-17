#pragma once

namespace bisky::editor
{

struct IRenderable
{
    virtual void draw() = 0;
};

} // namespace bisky::editor