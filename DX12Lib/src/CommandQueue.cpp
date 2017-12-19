#include <DX12LibPCH.h>

#include <CommandQueue.h>
#include <CommandList.h>

CommandQueue::CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
    : m_FenceValue(0)
    , m_CommandListType(type)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
    ThrowIfFailed(device->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));

    m_FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(m_FenceEvent && "Failed to create fence event handle.");

}

CommandQueue::~CommandQueue()
{}

uint64_t CommandQueue::Signal()
{
    m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), ++m_FenceValue);
    return m_FenceValue;
}

bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
    return m_d3d12Fence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::WaitForFenceValue(uint64_t fenceValue)
{
    if (!IsFenceComplete(fenceValue))
    {
        m_d3d12Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
        ::WaitForSingleObject(m_FenceEvent, DWORD_MAX);
    }
}

void CommandQueue::Flush()
{
    WaitForFenceValue(Signal());
}

std::shared_ptr<CommandList> CommandQueue::GetCommandList()
{
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    std::shared_ptr<CommandList> commandList;

    if ( !m_CommandAllocatorQueue.empty() && IsFenceComplete(m_CommandAllocatorQueue.front().fenceValue))
    {
        commandAllocator = m_CommandAllocatorQueue.front().commandAllocator;
        m_CommandAllocatorQueue.pop();

        commandAllocator->Reset();
    }
    else
    {
        commandAllocator = CreateCommandAllocator();
    }

    if (!m_CommandListQueue.empty())
    {
        commandList = m_CommandListQueue.front();
        m_CommandListQueue.pop();

        commandList->Reset(commandAllocator);
    }
    else
    {
        commandList = CreateCommandList(commandAllocator);
    }

    return commandList;
}

// Execute a command list.
// Returns the fence value to wait for for this command list.
uint64_t CommandQueue::ExecuteCommandList(std::shared_ptr<CommandList> commandList)
{
    commandList->Close();

    ID3D12CommandList* const ppCommandLists[] = {
        commandList->GetCommandList().Get()
    };

    m_d3d12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
    uint64_t fenceValue = Signal();

    m_CommandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandList->GetCommandAllocator() });
    m_CommandListQueue.push(commandList);

    return fenceValue;
}
