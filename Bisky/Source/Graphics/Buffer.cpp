#include "Common.hpp"

#include "Graphics/Buffer.hpp"

namespace bisky::gfx
{

int Buffer::GetSrvIndex(const Buffer *const buffer)
{
    return buffer->srvDescriptor.index;
}

int Buffer::GetUavIndex(const Buffer *const buffer)
{
    return buffer->uavDescriptor.index;
}

int Buffer::GetCbvIndex(const Buffer *const buffer)
{
    return buffer->cbvDescriptor.index;
}

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
    resource.Reset();
}

} // namespace bisky::gfx