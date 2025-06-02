#include "Common.hpp"

#include "Graphics/Texture.hpp"

namespace bisky::gfx
{

int32_t Texture::GetSrvIndex(const Texture *const texture)
{
    return texture->srvDescriptor.index;
}

} // namespace bisky::gfx