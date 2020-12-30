#include "DX12LibPCH.h"

#include <dx12lib/CommandQueue.h>

#include <dx12lib/CommandList.h>
#include <dx12lib/Device.h>
#include <dx12lib/ResourceStateTracker.h>

using namespace dx12lib;

// Adapter for std::make_shared
class MakeCommandList : public CommandList
{
public:
    MakeCommandList( Device& device, D3D12_COMMAND_LIST_TYPE type )
    : CommandList( device, type )
    {}

    virtual ~MakeCommandList() {}
};

CommandQueue::CommandQueue( Device& device, D3D12_COMMAND_LIST_TYPE type )
: m_Device( device )
, m_CommandListType( type )
, m_FenceValue( 0 )
{
    auto d3d12Device = m_Device.GetD3D12Device();

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type                     = type;
    desc.Priority                 = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask                 = 0;

    ThrowIfFailed( d3d12Device->CreateCommandQueue( &desc, IID_PPV_ARGS( &m_d3d12CommandQueue ) ) );
    ThrowIfFailed( d3d12Device->CreateFence( m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &m_d3d12Fence ) ) );

    switch ( type )
    {
    case D3D12_COMMAND_LIST_TYPE_COPY:
        m_d3d12CommandQueue->SetName( L"Copy Command Queue" );
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        m_d3d12CommandQueue->SetName( L"Compute Command Queue" );
        break;
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        m_d3d12CommandQueue->SetName( L"Direct Command Queue" );
        break;
    }
}

CommandQueue::~CommandQueue() {}

uint64_t CommandQueue::Signal()
{
    uint64_t fenceValue = ++m_FenceValue;
    m_d3d12CommandQueue->Signal( m_d3d12Fence.Get(), fenceValue );
    return fenceValue;
}

bool CommandQueue::IsFenceComplete( uint64_t fenceValue )
{
    return m_d3d12Fence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::WaitForFenceValue( uint64_t fenceValue )
{
    if ( !IsFenceComplete( fenceValue ) )
    {
        auto event = ::CreateEvent( NULL, FALSE, FALSE, NULL );
        if ( event )
        {
            m_d3d12Fence->SetEventOnCompletion( fenceValue, event );
            ::WaitForSingleObject( event, DWORD_MAX );
            ::CloseHandle( event );
        }
    }
}

void CommandQueue::Flush()
{
    std::lock_guard<std::mutex> lock( m_CommandListsMutex );

    // In case the command queue was signaled directly
    // using the CommandQueue::Signal method then the
    // fence value of the command queue might be higher than the fence
    // value of any of the executed command lists.
    WaitForFenceValue( m_FenceValue );

    // Reset command lists to release any allocations and references.
    CommandListQueue tmpQueue = m_CommandLists;
    while ( !tmpQueue.empty() )
    {
        CommandListEntry commandListEntry = tmpQueue.front();
        commandListEntry.commandList->Reset();
        commandListEntry.commandList->Close();  // The command list is expected to be in a closed state.
        tmpQueue.pop();
    }
}

std::shared_ptr<CommandList> CommandQueue::GetCommandList()
{
    std::shared_ptr<CommandList> commandList;

    // If there is a finished command list on the queue.
    std::lock_guard<std::mutex> lock( m_CommandListsMutex );
    if ( !m_CommandLists.empty() && IsFenceComplete( m_CommandLists.front().fenceValue ) )
    {
        commandList = m_CommandLists.front().commandList;
        m_CommandLists.pop();

        commandList->Reset();
    }
    else
    {
        // Otherwise create a new command list.
        commandList = std::make_shared<MakeCommandList>( m_Device, m_CommandListType );
    }

    return commandList;
}

// Execute a command list.
// Returns the fence value to wait for for this command list.
uint64_t CommandQueue::ExecuteCommandList( std::shared_ptr<CommandList> commandList )
{
    return ExecuteCommandLists( std::vector<std::shared_ptr<CommandList>>( { commandList } ) );
}

uint64_t CommandQueue::ExecuteCommandLists( const std::vector<std::shared_ptr<CommandList>>& commandLists )
{
    ResourceStateTracker::Lock();

    // Command lists that need to put back on the command list queue.
    std::vector<std::shared_ptr<CommandList>> toBeQueued;
    toBeQueued.reserve( commandLists.size() * 2 );  // 2x since each command list will have a pending command list.

    // Generate mips command lists.
    std::vector<std::shared_ptr<CommandList>> generateMipsCommandLists;
    generateMipsCommandLists.reserve( commandLists.size() );

    // Command lists that need to be executed.
    std::vector<ID3D12CommandList*> d3d12CommandLists;
    d3d12CommandLists.reserve( commandLists.size() *
                               2 );  // 2x since each command list will have a pending command list.

    for ( auto commandList: commandLists )
    {
        auto pendingCommandList = GetCommandList();
        bool hasPendingBarriers = commandList->Close( pendingCommandList );
        pendingCommandList->Close();
        // If there are no pending barriers on the pending command list, there is no reason to
        // execute an empty command list on the command queue.
        if ( hasPendingBarriers )
        {
            d3d12CommandLists.push_back( pendingCommandList->GetD3D12CommandList().Get() );
        }
        d3d12CommandLists.push_back( commandList->GetD3D12CommandList().Get() );

        toBeQueued.push_back( pendingCommandList );
        toBeQueued.push_back( commandList );

        auto generateMipsCommandList = commandList->GetGenerateMipsCommandList();
        if ( generateMipsCommandList )
        {
            generateMipsCommandLists.push_back( generateMipsCommandList );
        }
    }

    UINT numCommandLists = static_cast<UINT>( d3d12CommandLists.size() );
    m_d3d12CommandQueue->ExecuteCommandLists( numCommandLists, d3d12CommandLists.data() );
    uint64_t fenceValue = Signal();

    ResourceStateTracker::Unlock();

    // Queue command lists for reuse.
    {
        std::lock_guard<std::mutex> lock( m_CommandListsMutex );
        for ( auto commandList: toBeQueued )
        {
            m_CommandLists.emplace( fenceValue, commandList );
        }
    }

    // If there are any command lists that generate mips then execute those
    // after the initial resource command lists have finished.
    if ( generateMipsCommandLists.size() > 0 )
    {
        auto& computeQueue = m_Device.GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COMPUTE );
        computeQueue.Wait( *this );
        computeQueue.ExecuteCommandLists( generateMipsCommandLists );
    }

    return fenceValue;
}

void CommandQueue::Wait( const CommandQueue& other )
{
    m_d3d12CommandQueue->Wait( other.m_d3d12Fence.Get(), other.m_FenceValue );
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> CommandQueue::GetD3D12CommandQueue() const
{
    return m_d3d12CommandQueue;
}
