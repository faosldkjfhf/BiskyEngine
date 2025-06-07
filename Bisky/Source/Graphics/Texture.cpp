#include "Common.hpp"

#include "Graphics/Texture.hpp"

namespace bisky::gfx
{

int32_t Texture::GetSrvIndex(const Texture *const texture)
{
    if (texture)
    {
        return texture->srvDescriptor.index;
    }

    return -1;
}

} // namespace bisky::gfx