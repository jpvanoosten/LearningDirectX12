#include <DX12LibPCH.h>

#include <CommandList.h>

#include <Application.h>

CommandList::CommandList(D3D12_COMMAND_LIST_TYPE type, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator)
    : m_CommandListType(type)
    , m_CommandAllocator(allocator)
{
    auto device = Application::Get().GetDevice();

    ThrowIfFailed(device->CreateCommandList(0, m_CommandListType, m_CommandAllocator.Get(),
        nullptr, IID_PPV_ARGS(&m_GraphicsCommandList)));
}