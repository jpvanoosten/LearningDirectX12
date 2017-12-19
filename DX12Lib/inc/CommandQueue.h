/**
 * Wrapper class for a ID3D12CommandQueue.
 */

#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <cstdint>

#include <vector>
#include <queue>

class CommandList;

class CommandQueue
{
public:
    CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandQueue();

    uint64_t Signal();
    bool IsFenceComplete(uint64_t fenceValue);
    void WaitForFenceValue(uint64_t fenceValue);
    void Flush();

    // Get an available command list from the command queue.
    std::shared_ptr<CommandList> GetCommandList();
    // Execute a command list.
    // Returns the fence value to wait for for this command list.
    uint64_t ExecuteCommandList( std::shared_ptr<CommandList> commandList );

protected:

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
    std::shared_ptr<CommandList> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator>);

private:
    // Keep track of command allocators that are "in-flight"
    struct CommandAllocatorEntry
    {
        uint64_t fenceValue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    };

    using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
    using CommandListQueue = std::queue< std::shared_ptr<CommandList> >;

    D3D12_COMMAND_LIST_TYPE                     m_CommandListType;
    Microsoft::WRL::ComPtr<ID3D12Device2>       m_d3d12Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>  m_d3d12CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence>         m_d3d12Fence;
    HANDLE                                      m_FenceEvent;
    uint64_t                                    m_FenceValue;

    CommandAllocatorQueue                       m_CommandAllocatorQueue;
    CommandListQueue                            m_CommandListQueue;
};