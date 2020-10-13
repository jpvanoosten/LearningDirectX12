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
#include "TextureUsage.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>

namespace dx12lib
{

class Adapter;
class ByteAddressBuffer;
class CommandQueue;
class CommandList;
class DescriptorAllocator;
class IndexBuffer;
class RootSignature;
class StructuredBuffer;
class SwapChain;
class Texture;
class VertexBuffer;

class Device : public std::enable_shared_from_this<Device>
{
public:
    /**
     * Create a new DX12 device using the provided adapter.
     * If no adapter is specified, then the highest performance adapter will be
     * chosen.
     */
    static std::shared_ptr<Device> CreateDevice( std::shared_ptr<Adapter> adapter = nullptr );

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
     * Create a swapchain using the provided OS window handle.
     */
    std::shared_ptr<SwapChain> CreateSwapChain( HWND hWnd );

    /**
     * Create a ByteAddressBuffer resource.
     *
     * @param resDesc A description of the resource.
     */
    std::shared_ptr<ByteAddressBuffer> CreateByteAddressBuffer( size_t bufferSize );
    std::shared_ptr<ByteAddressBuffer> CreateByteAddressBuffer( Microsoft::WRL::ComPtr<ID3D12Resource> resource );

    /**
     * Create a structured buffer resource.
     */
    std::shared_ptr<StructuredBuffer> CreateStructuredBuffer( size_t numElements, size_t elementSize );
    std::shared_ptr<StructuredBuffer> CreateStructuredBuffer( Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                                                              size_t numElements, size_t elementSize );

    /**
     * Create a Texture resource.
     *
     * @param resourceDesc A description of the texture to create.
     * @param [clearVlue] Optional optimized clear value for the texture.
     * @param [textureUsage] Optional texture usage flag provides a hint about how the texture will be used.
     *
     * @returns A pointer to the created texture.
     */
    std::shared_ptr<Texture> CreateTexture( const D3D12_RESOURCE_DESC& resourceDesc,
                                            const D3D12_CLEAR_VALUE*   clearValue   = nullptr,
                                            TextureUsage               textureUsage = TextureUsage::Albedo );
    std::shared_ptr<Texture> CreateTexture( Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                                            const D3D12_CLEAR_VALUE*               clearValue = nullptr,
                                            TextureUsage textureUsage                         = TextureUsage::Albedo );

    std::shared_ptr<IndexBuffer> CreateIndexBuffer( size_t numIndicies, DXGI_FORMAT indexFormat );
    std::shared_ptr<IndexBuffer> CreateIndexBuffer( Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices,
                                                    DXGI_FORMAT indexFormat );

    std::shared_ptr<VertexBuffer> CreateVertexBuffer( size_t numVertices, size_t vertexStride );
    std::shared_ptr<VertexBuffer> CreateVertexBuffer( Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                                                      size_t numVertices, size_t vertexStride );

    std::shared_ptr<RootSignature> CreateRootSignature( const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc,
                                                        D3D_ROOT_SIGNATURE_VERSION        rootSignatureVersion );
    /**
     * Flush all command queues.
     */
    void Flush();

    /**
     * Release stale descriptors. This should only be called with a completed frame counter.
     */
    void ReleaseStaleDescriptors( uint64_t finishedFrame );

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
    friend class CommandQueue;

    explicit Device( std::shared_ptr<Adapter> adapter );
    virtual ~Device();

    // Init after creation.
    void Init();

    /**
     * Create a command list for use by a CommandQueue.
     */
    std::shared_ptr<CommandList> CreateCommandList( D3D12_COMMAND_LIST_TYPE type );

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