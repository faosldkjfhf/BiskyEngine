#include "Common.hpp"

#include "Graphics/CommandQueue.hpp"
#include "Graphics/Device.hpp"
#include "Graphics/GraphicsCommandList.hpp"
#include "Graphics/ResourceUpload.hpp"

namespace bisky::gfx
{

ResourceUpload::ResourceUpload(Device *const device) : m_device(*device)
{
    m_graphicsCommandList = std::make_unique<GraphicsCommandList>(device);
}

ResourceUpload::~ResourceUpload()
{
}

void ResourceUpload::Begin(D3D12_COMMAND_LIST_TYPE cmdListType)
{
    m_graphicsCommandList->reset();
    // LOG_VERBOSE("------ Beginning resource upload block ------");
}

std::future<void> ResourceUpload::Finish()
{
    std::promise<void> p;
    std::future<void>  future = p.get_future();

    std::thread([&p, this]() {
        std::array<const CommandList *const, 1> cmdLists = {m_graphicsCommandList.get()};
        m_device.getDirectCommandQueue()->executeCommandLists(cmdLists);
        m_device.getDirectCommandQueue()->flush();
        // LOG_VERBOSE("------ Resource upload complete ------");
        p.set_value_at_thread_exit();
    }).join();

    return future;
}

GraphicsCommandList *const ResourceUpload::getCommandList() const
{
    return m_graphicsCommandList.get();
}

} // namespace bisky::gfx