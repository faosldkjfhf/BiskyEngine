#include "Common.hpp"

#include "Graphics/Constants.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/Utilities.hpp"
#include "Graphics/Window.hpp"

namespace bisky::gfx
{

Device::Device(Window *window, DXGI_FORMAT backBufferFormat) : m_backBufferFormat(backBufferFormat)
{
    initDevice();
    initDescriptorHeaps();
    initFactory();
    initCommandQueue();
    initSwapChain(window);
    getBuffers(window->getWidth(), window->getHeight());
    initFrameResources();

    LOG_INFO("D3D12 initialized");
}

Device::~Device()
{
    m_pipelineStates.clear();
    m_rootSignatures.clear();

    for (uint32_t i = 0; i < FramesInFlight; i++)
    {
        m_renderTargetBuffers[i].reset();
        m_frameResources[i].reset();
    }

    m_swapChain.Reset();
    m_directCommandQueue.reset();
    m_factory.Reset();
    m_cbvSrvUavHeap.reset();
    m_dsvHeap.reset();
    m_rtvHeap.reset();
    m_device.Reset();
}

void Device::update()
{
    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void Device::resize(uint32_t width, uint32_t height)
{
    m_swapChain->ResizeBuffers(
        FramesInFlight, width, height, DXGI_FORMAT_UNKNOWN,
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
    );

    m_viewport.Width  = static_cast<float>(width);
    m_viewport.Height = static_cast<float>(height);
    m_scissor.right   = width;
    m_scissor.bottom  = height;
}

void Device::releaseBuffers()
{
    for (uint32_t i = 0; i < FramesInFlight; i++)
    {
        m_renderTargetBuffers[i].reset();
    }

    m_depthStencilBuffer.reset();
}

void Device::getBuffers(uint32_t width, uint32_t height)
{
    for (uint32_t i = 0; i < FramesInFlight; i++)
    {
        m_renderTargetBuffers[i] = std::make_unique<Texture>();
        // m_renderTargetBuffers[i]->width         = width;
        // m_renderTargetBuffers[i]->height        = height;
        m_renderTargetBuffers[i]->rtvDescriptor = m_renderTargetHandles[i];

        m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargetBuffers[i]->resource));

        D3D12_RENDER_TARGET_VIEW_DESC rtv{};
        rtv.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtv.Format               = m_backBufferFormat;
        rtv.Texture2D.MipSlice   = 0;
        rtv.Texture2D.PlaneSlice = 0;
        m_device->CreateRenderTargetView(m_renderTargetBuffers[i]->resource.Get(), &rtv, m_renderTargetHandles[i].cpu);
    }

    m_depthStencilBuffer =
        createTexture2D(width, height, m_depthStencilFormat, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    m_depthStencilBuffer->dsvDescriptor = m_depthStencilHandle;

    D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {
        .Format        = m_depthStencilFormat,
        .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
        .Flags         = D3D12_DSV_FLAG_NONE,
        .Texture2D     = {.MipSlice = 0},
    };
    m_device->CreateDepthStencilView(m_depthStencilBuffer->resource.Get(), &dsv, m_depthStencilHandle.cpu);
}

void Device::addGraphicsPipelineState(std::string_view name, const GraphicsPipelineStateDesc &gfxDesc)
{
    m_pipelineStates[name] = std::make_unique<PipelineState>(m_device.Get(), gfxDesc);
    LOG_INFO("Pipeline state " + std::string(name) + " created");
}

void Device::addRootSignature(std::string_view name, const gfx::RootParameters &parameters)
{
    m_rootSignatures[name] = std::make_unique<RootSignature>(m_device.Get(), parameters);
    LOG_INFO("Root signature " + std::string(name) + " created");
}

std::unique_ptr<Buffer> Device::createUploadBuffer(uint32_t size, void *data, uint32_t dataSize)
{
    std::unique_ptr<Buffer> buffer = std::make_unique<Buffer>();

    D3D12_HEAP_PROPERTIES heap = {
        .Type = D3D12_HEAP_TYPE_UPLOAD,
    };

    D3D12_RESOURCE_DESC resource = {
        .Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER,
        .Alignment        = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
        .Width            = size,
        .Height           = 1,
        .DepthOrArraySize = 1,
        .MipLevels        = 1,
        .Format           = DXGI_FORMAT_UNKNOWN,
        .SampleDesc       = {.Count = 1, .Quality = 0},
        .Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        .Flags            = D3D12_RESOURCE_FLAG_NONE,
    };

    m_device->CreateCommittedResource(
        &heap, D3D12_HEAP_FLAG_NONE, &resource, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&buffer->resource)
    );

    if (data)
    {
        uint32_t allocSize = dataSize;
        if (dataSize == 0u)
        {
            allocSize = size;
        }

        void *mapped;
        buffer->resource->Map(0, nullptr, &mapped);
        memcpy(mapped, data, static_cast<size_t>(allocSize));
        buffer->resource->Unmap(0, nullptr);
    }

    return std::move(buffer);
}

std::unique_ptr<Texture> Device::createTexture2D(
    uint32_t width, uint32_t height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags
)
{
    std::unique_ptr<Texture> texture = std::make_unique<Texture>();
    // texture->width                   = width;
    // texture->height                  = height;

    D3D12_HEAP_PROPERTIES heap = {
        .Type = D3D12_HEAP_TYPE_DEFAULT,
    };

    D3D12_RESOURCE_DESC resource = {
        .Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        .Alignment        = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
        .Width            = width,
        .Height           = height,
        .DepthOrArraySize = 1,
        .MipLevels        = 1,
        .Format           = format,
        .SampleDesc       = {.Count = 1, .Quality = 0},
        .Layout           = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        .Flags            = flags,
    };

    if (format == DXGI_FORMAT_D24_UNORM_S8_UINT || format == DXGI_FORMAT_D32_FLOAT)
    {
        D3D12_CLEAR_VALUE optClear = {.Format = format, .DepthStencil = {.Depth = 1.0f, .Stencil = 0}};
        m_device->CreateCommittedResource(
            &heap, D3D12_HEAP_FLAG_NONE, &resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, &optClear,
            IID_PPV_ARGS(&texture->resource)
        );
    }
    else
    {
        m_device->CreateCommittedResource(
            &heap, D3D12_HEAP_FLAG_NONE, &resource, D3D12_RESOURCE_STATE_COMMON, nullptr,
            IID_PPV_ARGS(&texture->resource)
        );
    }

    return std::move(texture);
}

void Device::copyToTexture(unsigned char *data, const ImageData &imageData, Texture *const texture)
{
    // -------------- begin upload block --------------
    ResourceUpload upload(this);
    upload.Begin();

    // -------------- create upload buffer --------------
    std::unique_ptr<Buffer> uploadBuffer =
        createUploadBuffer(static_cast<uint32_t>(imageData.width * imageData.height * 4), data);

    // -------------- copy upload buffer to texture --------------
    upload.getCommandList()->copyTextureRegion(uploadBuffer.get(), texture, imageData);

    // -------------- end upload block --------------
    auto finish = upload.Finish();
    finish.wait();
}

std::shared_ptr<Texture> Device::createImageFromMemory(unsigned char *data, size_t dataSize)
{
    // -------------- attempt to load the image --------------
    ImageData      imageData;
    unsigned char *loadedImage = stbi_load_from_memory(
        data, static_cast<int>(dataSize), &imageData.width, &imageData.height, &imageData.channelCount, 4
    );
    if (!loadedImage)
    {
        LOG_WARNING("Failed to load image from memory");
        return nullptr;
    }

    // -------------- create a texture and copy the data to it --------------
    std::unique_ptr<Texture> texture = createTexture2D(imageData.width, imageData.height, imageData.format);
    copyToTexture(loadedImage, imageData, texture.get());
    stbi_image_free(loadedImage);

    // -------------- create a shader resource view for the texture --------------
    texture->srvDescriptor              = m_cbvSrvUavHeap->allocate();
    D3D12_SHADER_RESOURCE_VIEW_DESC srv = {
        .Format                  = imageData.format,
        .ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D,
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        .Texture2D =
            {
                .MostDetailedMip     = 0u,
                .MipLevels           = texture->resource->GetDesc().MipLevels,
                .PlaneSlice          = 0u,
                .ResourceMinLODClamp = 0.0f,
            },
    };
    m_device->CreateShaderResourceView(texture->resource.Get(), &srv, texture->srvDescriptor.cpu);

    return std::move(texture);
}

void Device::createShaderResourceView(Buffer *const buffer, const D3D12_SHADER_RESOURCE_VIEW_DESC *desc)
{
    m_device->CreateShaderResourceView(buffer->resource.Get(), desc, buffer->srvDescriptor.cpu);
}

void Device::incrementFrameResourceIndex()
{
    m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1) % FramesInFlight;
}

ID3D12Device10 *const Device::getDevice() const
{
    return m_device.Get();
}

IDXGIFactory7 *const Device::getFactory() const
{
    return m_factory.Get();
}

CommandQueue *const Device::getDirectCommandQueue() const
{
    return m_directCommandQueue.get();
}

IDXGISwapChain4 *const Device::getSwapChain() const
{
    return m_swapChain.Get();
}

std::array<std::unique_ptr<FrameResource>, Device::FramesInFlight> &Device::getFrameResources()
{
    return m_frameResources;
}

FrameResource *const Device::getFrameResource() const
{
    return m_frameResources[m_currentFrameResourceIndex].get();
}

const D3D12_VIEWPORT &Device::getViewport() const
{
    return m_viewport;
}

const D3D12_RECT &Device::getScissor() const
{
    return m_scissor;
}

Texture *const Device::getRenderTargetBuffer() const
{
    return m_renderTargetBuffers[m_currentBackBufferIndex].get();
}

const D3D12_CPU_DESCRIPTOR_HANDLE &Device::getRenderTargetView() const
{
    return m_renderTargetHandles[m_currentBackBufferIndex].cpu;
}

DXGI_FORMAT Device::getBackBufferFormat() const
{
    return m_backBufferFormat;
}

RootSignature *const Device::getRootSignature(std::string_view name) const
{
    auto it = m_rootSignatures.find(name);
    if (it == m_rootSignatures.end())
    {
        LOG_WARNING("Failed to find root signature " + std::string(name));
        return nullptr;
    }

    return it->second.get();
}

PipelineState *const Device::getPipelineState(std::string_view name) const
{
    auto it = m_pipelineStates.find(name);
    if (it == m_pipelineStates.end())
    {
        LOG_WARNING("Failed to find pipeline state " + std::string(name));
        return nullptr;
    }

    return it->second.get();
}

DescriptorHeap *const Device::getCbvSrvUavHeap() const
{
    return m_cbvSrvUavHeap.get();
}

Texture *const Device::getDepthStencilBuffer() const
{
    return m_depthStencilBuffer.get();
}

DXGI_FORMAT Device::getDepthStencilFormat() const
{
    return m_depthStencilFormat;
}

const D3D12_CPU_DESCRIPTOR_HANDLE &Device::getDepthStencilView() const
{
    return m_depthStencilHandle.cpu;
}

void Device::initDevice()
{
    D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
    LOG_VERBOSE("Device created");
}

void Device::initDescriptorHeaps()
{
    m_rtvHeap       = std::make_unique<DescriptorHeap>(m_device.Get(), DescriptorType::Rtv, 64);
    m_dsvHeap       = std::make_unique<DescriptorHeap>(m_device.Get(), DescriptorType::Dsv, 1);
    m_cbvSrvUavHeap = std::make_unique<DescriptorHeap>(
        m_device.Get(), DescriptorType::CbvSrvUav, 4096, DescriptorFlags::ShaderVisible
    );

    for (uint32_t i = 0; i < FramesInFlight; i++)
    {
        m_renderTargetHandles[i] = m_rtvHeap->allocate();
    }

    m_depthStencilHandle = m_dsvHeap->allocate();
    LOG_VERBOSE("Descriptor Heaps created");
}

void Device::initFactory()
{
    CreateDXGIFactory2(0, IID_PPV_ARGS(&m_factory));
    LOG_VERBOSE("Factory created");
}

void Device::initCommandQueue()
{
    m_directCommandQueue = std::make_unique<CommandQueue>(m_device.Get());
}

void Device::initSwapChain(Window *window)
{
    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.Format             = m_backBufferFormat;
    sd.Width              = window->getWidth();
    sd.Height             = window->getHeight();
    sd.Stereo             = false;
    sd.BufferUsage        = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount        = FramesInFlight;
    sd.SampleDesc.Count   = 1;
    sd.SampleDesc.Quality = 0;
    sd.Scaling            = DXGI_SCALING_STRETCH;
    sd.AlphaMode          = DXGI_ALPHA_MODE_IGNORE;
    sd.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd{};
    fd.Windowed = TRUE;

    wrl::ComPtr<IDXGISwapChain1> sc1;
    m_factory->CreateSwapChainForHwnd(
        m_directCommandQueue->getCommandQueue(), window->getHandle(), &sd, &fd, nullptr, &sc1
    );
    sc1->QueryInterface(IID_PPV_ARGS(&m_swapChain));

    LOG_VERBOSE("Swap Chain created");

    RECT cr;
    if (GetClientRect(window->getHandle(), &cr))
    {
        auto width  = cr.right - cr.left;
        auto height = cr.bottom - cr.top;

        m_viewport          = {};
        m_viewport.TopLeftX = 0.0f;
        m_viewport.TopLeftY = 0.0f;
        m_viewport.Width    = static_cast<float>(width);
        m_viewport.Height   = static_cast<float>(height);
        m_viewport.MinDepth = 0.0f;
        m_viewport.MaxDepth = 1.0f;

        m_scissor        = {};
        m_scissor.left   = 0;
        m_scissor.top    = 0;
        m_scissor.right  = width;
        m_scissor.bottom = height;
    }
}

void Device::initFrameResources()
{
    auto sceneBufferSize = constantBufferByteSize(sizeof(SceneBuffer));
    for (uint32_t i = 0; i < FramesInFlight; i++)
    {
        m_frameResources[i]                      = std::make_unique<FrameResource>();
        m_frameResources[i]->graphicsCommandList = std::make_unique<GraphicsCommandList>(this);
        m_frameResources[i]->resourceAllocator   = std::make_unique<Allocator>(this, 64u * 1024u);
        m_frameResources[i]->sceneBuffer         = createUploadBuffer(sceneBufferSize);
        m_frameResources[i]->fenceValue          = 0;

        // -------------- create constant buffer view --------------
        {
            m_frameResources[i]->sceneBuffer->cbvDescriptor = m_cbvSrvUavHeap->allocate();
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv{
                .BufferLocation = m_frameResources[i]->sceneBuffer->resource->GetGPUVirtualAddress(),
                .SizeInBytes    = sceneBufferSize
            };
            m_device->CreateConstantBufferView(&cbv, m_frameResources[i]->sceneBuffer->cbvDescriptor.cpu);
        }
    }

    LOG_VERBOSE("Frame resources created");
}

} // namespace bisky::gfx