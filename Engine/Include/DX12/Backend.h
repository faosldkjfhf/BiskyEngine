#pragma once

#include "Common.h"
#include "DX12/Context.h"

namespace DX12::Backend
{

// TODO: Have a separate command queue for immediate submit
/*
 * A helper function to immediately submit the function to the Context's direct command list
 *
 * \param function The lambda function to call
 */
inline void ImmediateSubmit(std::function<void(ID3D12GraphicsCommandList10 *cmdList)> &&function)
{
  auto *cmdList = Context::Get().ResetCommandList();
  function(cmdList);
  Context::Get().ExecuteCommandList(cmdList);
  Context::Get().FlushCommandQueue();
}

} // namespace DX12::Backend
