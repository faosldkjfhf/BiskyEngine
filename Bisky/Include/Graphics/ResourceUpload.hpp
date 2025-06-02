#pragma once

#include <d3d12.h>
#include <future>

namespace bisky::gfx
{

class Device;
class GraphicsCommandList;

/*
 * A wrapper over a command list that is immediately submitted.
 *
 * User should create and call Begin to start an upload pass.
 * Once they are done with all the commands they want to submit,
 * call Finish, which returns an std::future<void> that the user
 * can wait on.
 */
class ResourceUpload
{
  public:
    ResourceUpload(Device *const device);
    ~ResourceUpload();

    ResourceUpload(const ResourceUpload &)                    = delete;
    const ResourceUpload &operator=(const ResourceUpload &)   = delete;
    ResourceUpload(const ResourceUpload &&)                   = delete;
    const ResourceUpload &&operator=(const ResourceUpload &&) = delete;

  public:
    void              Begin(D3D12_COMMAND_LIST_TYPE cmdListType = D3D12_COMMAND_LIST_TYPE_DIRECT);
    std::future<void> Finish();

  public:
    GraphicsCommandList *const getCommandList() const;

  private:
    Device                              &m_device;
    std::unique_ptr<GraphicsCommandList> m_graphicsCommandList;
};

} // namespace bisky::gfx