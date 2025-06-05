#pragma once

#include "Graphics/CommandList.hpp"
#include "Graphics/Resources.hpp"
#include "Graphics/Texture.hpp"
#include "Scene/Mesh.hpp"

namespace bisky::gfx
{

class Device;
class PipelineState;
class RootSignature;
class DescriptorHeap;

/*
 * A wrapper class for a graphics command list.
 *
 * The methods here are meant to work with the other wrappers that are included in this library.
 *
 * This should be used over the normal d3d12 graphics command list.
 */
class GraphicsCommandList : public CommandList
{
  public:
    /*
     * Initializes the command list and command allocator.
     *
     * @param device The device wrapper to store.
     */
    explicit GraphicsCommandList(Device *device);

    /*
     * Resets the command list and command allocator.
     */
    virtual void reset() override;

    /*
     * Clears the given render target with an array of color values.
     *
     * @param renderTargetView The handle to the render target.
     * @param color An array of 4 float color values.
     */
    void clearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, float color[4]);

    void clearDepthStencilView(D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView, float depth, uint32_t stencil);

    /*
     * Sets the viewport to the given viewport.
     *
     * @param viewport The viewport to use.
     */
    void setViewport(const D3D12_VIEWPORT &viewport);

    /*
     * Sets the scissor rect to the given scissor.
     *
     * @param scissor The scissor rect to use.
     */
    void setScissorRect(const D3D12_RECT &scissor);

    /*
     * Sets the render target for the output merger.
     * This method assumes there is no depth stencil and only one render target.
     *
     * @param renderTarget The render target to use.
     */
    void setRenderTargets(const D3D12_CPU_DESCRIPTOR_HANDLE &renderTarget);

    /*
     * Sets the render target and depth stencil for the output merger.
     * This method assumes there is only one render target.
     *
     * @param renderTarget The render target to use.
     * @param depthStencilView The depth stencil to use.
     */
    void setRenderTargets(
        const D3D12_CPU_DESCRIPTOR_HANDLE &renderTarget, const D3D12_CPU_DESCRIPTOR_HANDLE &depthStencilView
    );

    /*
     * Copies the buffer region at offset 0 from src to dst.
     * It's up to the user to make sure the structs have enough memory allocated.
     *
     * @param src The source buffer to copy
     * @param dst The buffer to copy into
     * @param bufferSize The size of the data to copy
     */
    void copyBufferRegion(Buffer *const src, Buffer *const dst, size_t bufferSize);

    void copyTextureRegion(Buffer *const src, Texture *const dst, const ImageData &imageData);

    /*
     * Sets the descriptor heaps.
     * In a bindless model, this should be called before setting the root signature.
     * It's up to the user to do this.
     *
     * @param descriptorHeaps A span of descriptor heaps to include
     */
    void setDescriptorHeaps(const std::span<const DescriptorHeap *const> &descriptorHeaps);

    /*
     * Sets the pipeline state to the given state.
     *
     * @param pipelineState The state to set.
     */
    void setPipelineState(gfx::PipelineState *const pipelineState);

    /*
     * Sets the root signature.
     *
     * @param rootSignature The root signature to set.
     */
    void setRootSignature(gfx::RootSignature *const rootSignature);

    /*
     * Sets the vertex buffers.
     * This doesn't need to be used if using a bindless model.
     *
     * @param vertexBufferView The vertex buffer view details.
     */
    void setVertexBuffers(const VertexBufferView &vertexBufferView);

    /*
     * Sets the index buffer.
     *
     * @param indexBufferView The index buffer view details.
     */
    void setIndexBuffer(const IndexBufferView &indexBufferView);

    /*
     * Sets the primitive topology.
     * This should match the primitive topology in the pipeline state.
     *
     * @param topology The topology to use.
     */
    void setPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology);

    /*
     * Issues a draw call using the submesh arguments.
     *
     * @param submesh The submesh to draw.
     */
    void drawIndexedInstanced(const scene::Submesh &submesh);

    /*
     * Passes a constant buffer view to the shader.
     *
     * @param index The index of the root parameter.
     * @param handle The GPU address to set it at.
     */
    void setConstantBufferView(uint32_t index, D3D12_GPU_VIRTUAL_ADDRESS handle);

    /*
     * Passes 32-bit constants to the shader.
     *
     * @param index The index of the root parameter.
     * @param numValues The number of values to set.
     * @param data A pointer to the start of the data.
     */
    void set32BitConstants(uint32_t index, uint32_t numValues, void *data);

  private:
    Device &m_device;
};

} // namespace bisky::gfx