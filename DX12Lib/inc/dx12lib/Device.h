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

#include "d3dx12.h"
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>

namespace dx12lib
{

class Adapter;
class ByteAddressBuffer;
class CommandQueue;
class CommandList;
class ConstantBuffer;
class ConstantBufferView;
class DescriptorAllocator;
class IndexBuffer;
class IndexBufferView;
class PipelineStateObject;
class Resource;
class RootSignature;
class ShaderResourceView;
class StructuredBuffer;
class SwapChain;
class Texture;
class UnorderedAccessView;
class VertexBuffer;
class VertexBufferView;

class Device
{
public:
    /**
     * Create a new DX12 device using the provided adapter.
     * If no adapter is specified, then the highest performance adapter will be  chosen.
     */
    static Device& Create( std::shared_ptr<Adapter> adapter = nullptr );

    /**
     * Get a reference to the previously created device.
     */
    static Device& Get();

    /**
     * Destroy the device instance.
     */
    static void Destroy();

    /**
     * Get a description of the adapter that was used to create the device.
     */
    std::wstring GetDescription() const;

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
                                            TextureUsage               textureUsage = TextureUsage::Albedo,
                                            const D3D12_CLEAR_VALUE*   clearValue   = nullptr );
    std::shared_ptr<Texture> CreateTexture( Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                                            TextureUsage                           textureUsage = TextureUsage::Albedo,
                                            const D3D12_CLEAR_VALUE*               clearValue   = nullptr );

    std::shared_ptr<IndexBuffer> CreateIndexBuffer( size_t numIndicies, DXGI_FORMAT indexFormat );
    std::shared_ptr<IndexBuffer> CreateIndexBuffer( Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices,
                                                    DXGI_FORMAT indexFormat );

    std::shared_ptr<VertexBuffer> CreateVertexBuffer( size_t numVertices, size_t vertexStride );
    std::shared_ptr<VertexBuffer> CreateVertexBuffer( Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                                                      size_t numVertices, size_t vertexStride );

    std::shared_ptr<RootSignature> CreateRootSignature( const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc );

    template<class PipelineStateStream>
    std::shared_ptr<PipelineStateObject> CreatePipelineStateObject( PipelineStateStream& pipelineStateStream )
    {
        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = { sizeof( PipelineStateStream ),
                                                                     &pipelineStateStream };

        return DoCreatePipelineStateObject( pipelineStateStreamDesc );
    }

    std::shared_ptr<VertexBufferView> CreateVertexBufferView( std::shared_ptr<VertexBuffer> vertexBuffer );

    std::shared_ptr<IndexBufferView> CreateIndexBufferView( std::shared_ptr<IndexBuffer> indexBuffer );

    std::shared_ptr<ConstantBufferView>
        CreateConstantBufferView( std::shared_ptr<ConstantBuffer>        constantBuffer,
                                  const D3D12_CONSTANT_BUFFER_VIEW_DESC* cbv = nullptr );

    std::shared_ptr<ShaderResourceView>
        CreateShaderResourceView( std::shared_ptr<Resource>              resource,
                                  const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr );

    std::shared_ptr<UnorderedAccessView>
        CreateUnorderedAccessView( std::shared_ptr<Resource>               resource,
                                   std::shared_ptr<Resource>               counterResource = nullptr,
                                   const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav             = nullptr );

    /**
     * Flush all command queues.
     */
    void Flush();

    /**
     * Release stale descriptors. This should only be called with a completed frame counter.
     */
    void ReleaseStaleDescriptors();

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
    CommandQueue& GetCommandQueue( D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT );

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap( UINT                       numDescriptors,
                                                                       D3D12_DESCRIPTOR_HEAP_TYPE type );

    Microsoft::WRL::ComPtr<ID3D12Device8> GetD3D12Device() const
    {
        return m_d3d12Device;
    }

    D3D_ROOT_SIGNATURE_VERSION GetHighestRootSignatureVersion() const
    {
        return m_HighestRootSignatureVersion;
    }

protected:
    explicit Device( std::shared_ptr<Adapter> adapter );
    virtual ~Device();

    std::shared_ptr<PipelineStateObject>
        DoCreatePipelineStateObject( const D3D12_PIPELINE_STATE_STREAM_DESC& pipelineStateStreamDesc );

private:
    Microsoft::WRL::ComPtr<ID3D12Device8> m_d3d12Device;

    // The adapter that was used to create the device:
    std::shared_ptr<Adapter> m_Adapter;

    // Default command queues.
    std::unique_ptr<CommandQueue> m_DirectCommandQueue;
    std::unique_ptr<CommandQueue> m_ComputeCommandQueue;
    std::unique_ptr<CommandQueue> m_CopyCommandQueue;

    // Descriptor allocators.
    std::unique_ptr<DescriptorAllocator> m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    D3D_ROOT_SIGNATURE_VERSION m_HighestRootSignatureVersion;
};
}  // namespace dx12lib