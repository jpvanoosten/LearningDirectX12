#include <DX12LibPCH.h>

#include <CommandQueue.h>

#include <Application.h>
#include <CommandList.h>
#include <ResourceStateTracker.h>

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type)
    : m_FenceValue(0)
    , m_CommandListType(type)
{
    auto device = Application::Get().GetDevice();

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
{
}

uint64_t CommandQueue::Signal()
{
    uint64_t fenceValue = ++m_FenceValue;
    m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), fenceValue);
    return fenceValue;
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
    std::shared_ptr<CommandList> commandList;

    // If there is a command list on the queue.
    if ( !m_CommandListQueue.empty() && IsFenceComplete(m_CommandListQueue.front().fenceValue))
    {
        commandList = m_CommandListQueue.front().commandList;
        m_CommandListQueue.pop();

        commandList->Reset();
    }
    else
    {
        // Otherwise create a new command list.
        commandList = std::make_shared<CommandList>(m_CommandListType);
    }

    return commandList;
}

// Execute a command list.
// Returns the fence value to wait for for this command list.
uint64_t CommandQueue::ExecuteCommandList(std::shared_ptr<CommandList> commandList)
{
    ResourceStateTracker::Lock();

    auto pendingCommandList = GetCommandList();

    // Close the command list, flushing any pending resource barriers.
    commandList->Close(*pendingCommandList);
    pendingCommandList->Close();

    ID3D12CommandList* const ppCommandLists[] = {
        pendingCommandList->GetGraphicsCommandList().Get(),
        commandList->GetGraphicsCommandList().Get()
    };

    m_d3d12CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    uint64_t fenceValue = Signal();

    ResourceStateTracker::Unlock();

    m_CommandListQueue.emplace(CommandListEntry{ fenceValue, pendingCommandList });
    m_CommandListQueue.emplace(CommandListEntry{ fenceValue, commandList });

    return fenceValue;
}

uint64_t CommandQueue::ExecuteCommandLists(const std::vector<std::shared_ptr<CommandList> >& commandLists)
{
    ResourceStateTracker::Lock();

    // Command lists that need to put back on the command list queue.
    std::vector<std::shared_ptr<CommandList> > toBeQueued;
    toBeQueued.reserve(commandLists.size() * 2);        // 2x since each command list will have a pending command list.

    // Command lists that need to be executed.
    std::vector<ID3D12CommandList*> d3d12CommandLists;
    d3d12CommandLists.reserve(commandLists.size() * 2); // 2x since each command list will have a pending command list.

    for (auto commandList : commandLists)
    {
        auto pendingCommandList = GetCommandList();
        commandList->Close(*pendingCommandList);
        pendingCommandList->Close();

        d3d12CommandLists.push_back(pendingCommandList->GetGraphicsCommandList().Get());
        d3d12CommandLists.push_back(commandList->GetGraphicsCommandList().Get());

        toBeQueued.push_back(pendingCommandList);
        toBeQueued.push_back(commandList);
    }

    UINT numCommandLists = static_cast<UINT>(d3d12CommandLists.size());
    m_d3d12CommandQueue->ExecuteCommandLists(numCommandLists, d3d12CommandLists.data());
    uint64_t fenceValue = Signal();
    
    ResourceStateTracker::Unlock();

    // Queue command lists.
    for (auto commandList : toBeQueued)
    {
        m_CommandListQueue.emplace(CommandListEntry{ fenceValue, commandList });
    }

    return fenceValue;
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue::GetD3D12CommandQueue() const
{
    return m_d3d12CommandQueue;
}
