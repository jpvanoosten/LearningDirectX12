#pragma once

#pragma once

/*
 *  Copyright(c) 2020 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file Device.h
 *  @date October 9, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief A wrapper for the D3D12Device.
 */

#include "DescriptorAllocation.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>

namespace dx12lib
{

class Adapter;
class ByteAddressBuffer;
class CommandQueue;
class DescriptorAllocator;
class SwapChain;

class Device : public std::enable_shared_from_this<Device>
{
public:
    /**
     * Create a new DX12 device using the provided adapter.
     * If no adapter is specified, then the highest performance adapter will be
     * chosen.
     */
    static std::shared_ptr<Device> Create( std::shared_ptr<Adapter> adapter = nullptr );

    /**
     * Allocate a number of CPU visible descriptors.
     */
    DescriptorAllocation AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors = 1 );

    /**
     * Gets the size of the handle increment for the given type of descriptor heap.
     */
    UINT GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE type ) const
    {
        return m_d3d12Device->GetDescriptorHandleIncrementSize( type );
    }

    /**
     * Release stale descriptors. This should only be called with a completed frame counter.
     */
    void ReleaseStaleDescriptors( uint64_t finishedFrame );

    /**
     * Create a swapchain using the provided OS window handle.
     */
    std::shared_ptr<SwapChain> CreateSwapChain( HWND hWnd );

    /**
     * Create a ByteAddressBuffer
     * 
     * @param resDesc A description of the resource.
     */
    std::shared_ptr<ByteAddressBuffer> CreateByteAddressBuffer( const D3D12_RESOURCE_DESC& resDesc );

    /**
     * Flush all command queues.
     */
    void Flush();

    /**
     * Get the adapter that was used to create this device.
     */
    std::shared_ptr<Adapter> GetAdapter() const
    {
        return m_Adapter;
    }

    /**
     * Get a command queue. Valid types are:
     * - D3D12_COMMAND_LIST_TYPE_DIRECT : Can be used for draw, dispatch, or copy commands.
     * - D3D12_COMMAND_LIST_TYPE_COMPUTE: Can be used for dispatch or copy commands.
     * - D3D12_COMMAND_LIST_TYPE_COPY   : Can be used for copy commands.
     * By default, a D3D12_COMMAND_LIST_TYPE_DIRECT queue is returned.
     */
    std::shared_ptr<CommandQueue>
        GetCommandQueue( D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT ) const;

    /**
     * Get a description of the adapter that was used to create the device.
     */
    std::wstring GetDescription() const;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap( UINT                       numDescriptors,
                                                                       D3D12_DESCRIPTOR_HEAP_TYPE type );

    Microsoft::WRL::ComPtr<ID3D12Device8> GetD3D12Device() const
    {
        return m_d3d12Device;
    }

protected:
    explicit Device( std::shared_ptr<Adapter> adapter );
    virtual ~Device();

    // Init after creation.
    void Init();

private:
    Microsoft::WRL::ComPtr<ID3D12Device8> m_d3d12Device;

    // The adapter that was used to create the device:
    std::shared_ptr<Adapter> m_Adapter;

    // Default command queues.
    std::shared_ptr<CommandQueue> m_DirectCommandQueue;
    std::shared_ptr<CommandQueue> m_ComputeCommandQueue;
    std::shared_ptr<CommandQueue> m_CopyCommandQueue;

    // Descriptor allocators.
    std::unique_ptr<DescriptorAllocator> m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};
}  // namespace dx12lib