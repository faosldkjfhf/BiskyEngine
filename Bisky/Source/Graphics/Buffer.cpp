#include "Common.hpp"

#include "Graphics/Buffer.hpp"

namespace bisky::gfx
{

int Buffer::GetSrvIndex(const Buffer *const buffer)
{
    if (buffer)
        return buffer->srvDescriptor.index;

    return -1;
}

int Buffer::GetUavIndex(const Buffer *const buffer)
{
    if (buffer)
        return buffer->uavDescriptor.index;

    return -1;
}

int Buffer::GetCbvIndex(const Buffer *const buffer)
{
    if (buffer)
        return buffer->cbvDescriptor.index;

    return -1;
}

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
    resource.Reset();
}

} // namespace bisky::gfx