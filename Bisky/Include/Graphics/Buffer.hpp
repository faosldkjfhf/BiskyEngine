#pragma once

#include "Graphics/Descriptor.hpp"

namespace bisky::gfx
{

/*
 * This is a wrapper class around a D3D12 Buffer Resource
 */
struct Buffer
{
    /*
     * Gets the index of the descriptor heap that the given buffer lives in.
     * -1 indicates that this type of resource doesn't exist in that heap.
     *
     * @param buffer The buffer to parse.
     * @return The index in the descriptor heap.
     */
    static int GetSrvIndex(const Buffer *const buffer);

    /*
     * Gets the index of the descriptor heap that the given buffer lives in.
     * -1 indicates that this type of resource doesn't exist in that heap.
     *
     * @param buffer The buffer to parse.
     * @return The index in the descriptor heap.
     */
    static int GetUavIndex(const Buffer *const buffer);

    /*
     * Gets the index of the descriptor heap that the given buffer lives in.
     * -1 indicates that this type of resource doesn't exist in that heap.
     *
     * @param buffer The buffer to parse.
     * @return The index in the descriptor heap.
     */
    static int GetCbvIndex(const Buffer *const buffer);

    explicit Buffer();
    ~Buffer();

    wrl::ComPtr<ID3D12Resource> resource      = nullptr;
    gfx::Descriptor             srvDescriptor = {};
    gfx::Descriptor             uavDescriptor = {};
    gfx::Descriptor             cbvDescriptor = {};

    /*
     * Rule of 5 deletors.
     */
    Buffer(const Buffer &)                    = delete;
    const Buffer &operator=(const Buffer &)   = delete;
    Buffer(const Buffer &&)                   = delete;
    const Buffer &&operator=(const Buffer &&) = delete;
};

} // namespace bisky::gfx