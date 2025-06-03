#pragma once

#include "Graphics/CommandQueue.hpp"
#include "Graphics/DescriptorHeap.hpp"
#include "Graphics/FrameResource.hpp"
#include "Graphics/PipelineState.hpp"
#include "Graphics/Resources.hpp"
#include "Graphics/RootSignature.hpp"
#include "Graphics/Texture.hpp"

namespace bisky::gfx
{

class Window;

/*
 * A wrapper class for D3D12.
 * Handles setting up most of the nitty gritty for you.
 */
class Device
{
  public:
    /*
     * Initializes the core D3D12 objects.
     *
     * @param window The window to use for the swap chain.
     * @param backBufferFormat The format to use for the back buffers.
     */
    explicit Device(Window *window, DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM);

    /*
     * Resets the core D3D12 objects.
     */
    ~Device();

    Device(const Device &)                    = delete;
    const Device &operator=(const Device &)   = delete;
    Device(const Device &&)                   = delete;
    const Device &&operator=(const Device &&) = delete;

  public: // Static variables
    constexpr static uint32_t FramesInFlight = 3u;

  public: // Public methods
    /*
     * Fetches the current back buffer index from the swap chain.
     */
    void update();

    /*
     * Resizes the swap chain and the buffers.
     * This is automatically called by the window.
     *
     * @param width The width to resize to.
     * @param height The height to resize to.
     */
    void resize(uint32_t width, uint32_t height);

    /*
     * Releases the back buffers.
     * This is automatically called by the window.
     */
    void releaseBuffers();

    /*
     * Retrieves the back buffers from the swap chain.
     *
     * @param width The width of the buffers.
     */
    void getBuffers(uint32_t width, uint32_t height);

    /*
     * Adds a graphics pipeline state with the given description.
     *
     * @param name The name to set.
     * @param gfxDesc The pipeline state description.
     */
    void addGraphicsPipelineState(std::string_view name, const GraphicsPipelineStateDesc &gfxDesc);

    /*
     * Adds a new root signature from the given root parameters.
     *
     * @param name The name to set.
     * @param gfxDesc The root parameters to build the root signature.
     */
    void addRootSignature(std::string_view name, const gfx::RootParameters &parameters);

    /*
     * Creates an upload buffer and returns it.
     * If data is passed in, also initializes it with the given data.
     * If dataSize is not passed in, the data is assumed to be the same
     * size as the buffer.
     *
     * @param size Size of the buffer to allocate.
     * @param data Optional pointer to start of initial data.
     * @param dataSize Optional size of the initial data.
     * @return An allocated upload buffer.
     */
    std::unique_ptr<Buffer> createUploadBuffer(uint32_t size, void *data = nullptr, uint32_t dataSize = 0u);

    /*
     * Creates an allocated texture2D with the given parameters.
     *
     * @param width The width of the texture.
     * @param height The height of the texture.
     * @param format The format of the texture.
     * @param flags The flags for the texture.
     * @return An allocated texture.
     */
    std::unique_ptr<Texture> createTexture2D(
        uint32_t width, uint32_t height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE
    );

    void copyToTexture(unsigned char *data, const ImageData &imageData, Texture *const texture);

    std::shared_ptr<Texture> createImageFromMemory(unsigned char *data, size_t dataSize);

    /*
     * Creates a shader resource view with the given buffer and description.
     *
     * @param buffer The buffer to bind.
     * @param desc The description of the data.
     */
    void createShaderResourceView(Buffer *const buffer, const D3D12_SHADER_RESOURCE_VIEW_DESC *desc);

    /*
     * Increments the frame resource index.
     * If this is called, it assumes that the previous frame resource has already been completed.
     */
    void incrementFrameResourceIndex();

  public: // Getter methods
    /*
     * Gets a pointer to the inner device.
     *
     * @return An umodifiable pointer to the inner device.
     */
    ID3D12Device10 *const getDevice() const;

    /*
     * Gets a pointer to the inner factory.
     *
     * @return An unmodifiable pointer to the inner factory.
     */
    IDXGIFactory7 *const getFactory() const;

    /*
     * Gets the direct command queue.
     *
     * @return An unmodifiable pointer to the direct command queue.
     */
    CommandQueue *const getDirectCommandQueue() const;

    /*
     * Gets the swap chain.
     *
     * @return An unmodifiable pointer to the swap chain.
     */
    IDXGISwapChain4 *const getSwapChain() const;

    std::array<std::unique_ptr<FrameResource>, Device::FramesInFlight> &getFrameResources();

    /*
     * Gets the frame resource for this frame.
     *
     * @return An unmodifiable pointer to the frame resource.
     */
    FrameResource *const getFrameResource() const;

    /*
     * Gets the default viewport.
     * It uses (0, 0) in the top left, and is of size (width, height).
     * Min depth of 0 and max depth of 1.
     *
     * @return A const reference to the default viewport.
     */
    const D3D12_VIEWPORT &getViewport() const;

    /*
     * Gets the default scissor rect.
     * It has left and top set to 0.
     * Right is set to width, and bottom is set to height.
     *
     * @return A const reference to the default scissor rect
     */
    const D3D12_RECT &getScissor() const;

    /*
     * Gets the current render target buffer.
     *
     * @return An unmodifiable pointer to the current render target buffer.
     */
    Texture *const getRenderTargetBuffer() const;

    /*
     * Gets the current render target view handle.
     *
     * @return A const reference to the current render target view handle.
     */
    const D3D12_CPU_DESCRIPTOR_HANDLE &getRenderTargetView() const;

    /*
     * Returns the back buffer format.
     *
     * @return The back buffer format.
     */
    DXGI_FORMAT getBackBufferFormat() const;

    /*
     * Gets the root signature with the given name.
     *
     * @param name The name of the root signature to fetch.
     * @return The root signature, or nullptr if not found.
     */
    RootSignature *const getRootSignature(std::string_view name) const;

    /*
     * Gets the pipeline state with the given name.
     *
     * @param name The name of the pipeline state to fetch.
     * @return The pipeline state, or nullptr if not found.
     */
    PipelineState *const getPipelineState(std::string_view name) const;

    /*
     * Gets a pointer to the cbv srv and uav heap.
     *
     * @return An unmodifiable pointer to the cbv srv and uav heap.
     */
    DescriptorHeap *const getCbvSrvUavHeap() const;

    /*
     * Gets the depth stencil buffer.
     *
     * @return An unmodifiable pointer to the depth stencil buffer.
     */
    Texture *const getDepthStencilBuffer() const;

    /*
     * Gets the depth stencil format.
     *
     * @return The depth stencil format.
     */
    DXGI_FORMAT getDepthStencilFormat() const;

    /*
     * Gets a CPU handle to the depth stencil.
     *
     * @return A const reference to the depth stencil handle.
     */
    const D3D12_CPU_DESCRIPTOR_HANDLE &getDepthStencilView() const;

  private: // Private methods
    void initDevice();
    void initDescriptorHeaps();
    void initFactory();
    void initCommandQueue();
    void initSwapChain(Window *window);
    void initFrameResources();

  private: // Private variables
    wrl::ComPtr<ID3D12Device10>  m_device;
    wrl::ComPtr<IDXGIFactory7>   m_factory;
    wrl::ComPtr<IDXGISwapChain4> m_swapChain;

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT     m_scissor;

    // command queues
    std::unique_ptr<CommandQueue> m_directCommandQueue;

    // heaps
    std::unique_ptr<DescriptorHeap> m_rtvHeap;
    std::unique_ptr<DescriptorHeap> m_dsvHeap;
    std::unique_ptr<DescriptorHeap> m_cbvSrvUavHeap;

    // back buffer stuff
    std::array<std::unique_ptr<Texture>, FramesInFlight> m_renderTargetBuffers;
    std::array<Descriptor, FramesInFlight>               m_renderTargetHandles;
    DXGI_FORMAT                                          m_backBufferFormat;
    uint32_t                                             m_currentBackBufferIndex    = 0;
    uint32_t                                             m_currentFrameResourceIndex = 0;

    // depth stencil stuff
    std::unique_ptr<Texture> m_depthStencilBuffer;
    Descriptor               m_depthStencilHandle = {};
    DXGI_FORMAT              m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // per-frame stuff
    std::array<std::unique_ptr<FrameResource>, FramesInFlight> m_frameResources;

    std::unordered_map<std::string_view, std::unique_ptr<RootSignature>> m_rootSignatures;
    std::unordered_map<std::string_view, std::unique_ptr<PipelineState>> m_pipelineStates;
};

} // namespace bisky::gfx