#pragma once

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
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
 *  @file CommandList.h
 *  @date October 22, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief CommandList class encapsulates a ID3D12GraphicsCommandList2 interface.
 *  The CommandList class provides additional functionality that makes working with
 *  DirectX 12 applications easier.
 */
#include "TextureUsage.h"

#include <d3d12.h>
#include <wrl.h>

#include <map>     // for std::map
#include <memory>  // for std::unique_ptr
#include <mutex>   // for std::mutex
#include <vector>  // for std::vector

namespace dx12lib
{

class Buffer;
class ByteAddressBuffer;
class ConstantBuffer;
class ConstantBufferView;
class Device;
class DynamicDescriptorHeap;
class GenerateMipsPSO;
class IndexBuffer;
class PanoToCubemapPSO;
class PipelineStateObject;
class RenderTarget;
class Resource;
class ResourceStateTracker;
class RootSignature;
class Scene;
class ShaderResourceView;
class StructuredBuffer;
class Texture;
class UnorderedAccessView;
class UploadBuffer;
class VertexBuffer;

class CommandList : public std::enable_shared_from_this<CommandList>
{
public:
    /**
     * Get the type of command list.
     */
    D3D12_COMMAND_LIST_TYPE GetCommandListType() const
    {
        return m_d3d12CommandListType;
    }

    /**
     * Get the device that was used to create this command list.
     */
    Device& GetDevice() const
    {
        return m_Device;
    }

    /**
     * Get direct access to the ID3D12GraphicsCommandList2 interface.
     */
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetD3D12CommandList() const
    {
        return m_d3d12CommandList;
    }

    /**
     * Transition a resource to a particular state.
     *
     * @param resource The resource to transition.
     * @param stateAfter The state to transition the resource to. The before state is resolved by the resource state
     * tracker.
     * @param subresource The subresource to transition. By default, this is D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
     * which indicates that all subresources are transitioned to the same state.
     * @param flushBarriers Force flush any barriers. Resource barriers need to be flushed before a command (draw,
     * dispatch, or copy) that expects the resource to be in a particular state can run.
     */
    void TransitionBarrier( const std::shared_ptr<Resource>& resource, D3D12_RESOURCE_STATES stateAfter,
                            UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false );
    void TransitionBarrier( Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter,
                            UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false );

    /**
     * Add a UAV barrier to ensure that any writes to a resource have completed
     * before reading from the resource.
     *
     * @param [resource} The resource to add a UAV barrier for (can be null).
     * @param flushBarriers Force flush any barriers. Resource barriers need to be
     * flushed before a command (draw, dispatch, or copy) that expects the resource
     * to be in a particular state can run.
     */
    void UAVBarrier( const std::shared_ptr<Resource>& resource = nullptr, bool flushBarriers = false );
    void UAVBarrier( Microsoft::WRL::ComPtr<ID3D12Resource> resource, bool flushBarriers = false );

    /**
     * Add an aliasing barrier to indicate a transition between usages of two
     * different resources that occupy the same space in a heap.
     *
     * @param [beforeResource] The resource that currently occupies the heap (can be null).
     * @param [afterResource] The resource that will occupy the space in the heap (can be null).
     */
    void AliasingBarrier( const std::shared_ptr<Resource>&               = nullptr,
                          const std::shared_ptr<Resource>& afterResource = nullptr, bool flushBarriers = false );
    void AliasingBarrier( Microsoft::WRL::ComPtr<ID3D12Resource> beforeResource,
                          Microsoft::WRL::ComPtr<ID3D12Resource> afterResource, bool flushBarriers = false );

    /**
     * Flush any barriers that have been pushed to the command list.
     */
    void FlushResourceBarriers();

    /**
     * Copy resources.
     */
    void CopyResource( const std::shared_ptr<Resource>& dstRes, const std::shared_ptr<Resource>& srcRes );
    void CopyResource( Microsoft::WRL::ComPtr<ID3D12Resource> dstRes, Microsoft::WRL::ComPtr<ID3D12Resource> srcRes );

    /**
     * Resolve a multisampled resource into a non-multisampled resource.
     */
    void ResolveSubresource( const std::shared_ptr<Resource>&, const std::shared_ptr<Resource>&,
                             uint32_t dstSubresource = 0, uint32_t srcSubresource = 0 );

    /**
     * Copy the contents to a vertex buffer in GPU memory.
     */
    std::shared_ptr<VertexBuffer> CopyVertexBuffer( size_t numVertices, size_t vertexStride,
                                                    const void* vertexBufferData );
    template<typename T>
    std::shared_ptr<VertexBuffer> CopyVertexBuffer( const std::vector<T>& vertexBufferData )
    {
        return CopyVertexBuffer( vertexBufferData.size(), sizeof( T ), vertexBufferData.data() );
    }

    /**
     * Copy the contents to a index buffer in GPU memory.
     */
    std::shared_ptr<IndexBuffer> CopyIndexBuffer( size_t numIndicies, DXGI_FORMAT indexFormat,
                                                  const void* indexBufferData );
    template<typename T>
    std::shared_ptr<IndexBuffer> CopyIndexBuffer( const std::vector<T>& indexBufferData )
    {
        assert( sizeof( T ) == 2 || sizeof( T ) == 4 );

        DXGI_FORMAT indexFormat = ( sizeof( T ) == 2 ) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        return CopyIndexBuffer( indexBufferData.size(), indexFormat, indexBufferData.data() );
    }

    /**
     * Copy the contents to a constant buffer in GPU memory.
     */
    std::shared_ptr<ConstantBuffer> CopyConstantBuffer( size_t bufferSize, const void* bufferData );

    template<typename T>
    std::shared_ptr<ConstantBuffer> CopyConstantBuffer(const T& data)
    {
        return CopyConstantBuffer( sizeof( T ), &data );
    }

    /**
     * Copy the contents to a byte address buffer in GPU memory.
     */
    std::shared_ptr<ByteAddressBuffer> CopyByteAddressBuffer( size_t bufferSize, const void* bufferData );
    template<typename T>
    std::shared_ptr<ByteAddressBuffer> CopyByteAddressBuffer( const T& data )
    {
        return CopyByteAddressBuffer( sizeof( T ), &data );
    }

    /**
     * Copy the contents to a structured buffer in GPU memory.
     */
    std::shared_ptr<StructuredBuffer> CopyStructuredBuffer( size_t numElements, size_t elementSize,
                                                            const void* bufferData );
    template<typename T>
    std::shared_ptr<StructuredBuffer> CopyStructuredBuffer( const std::vector<T>& bufferData )
    {
        return CopyStructuredBuffer( bufferData.size(), sizeof( T ), bufferData.data() );
    }

    /**
     * Set the current primitive topology for the rendering pipeline.
     */
    void SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY primitiveTopology );

    /**
     * Load a texture by a filename.
     */
    std::shared_ptr<Texture> LoadTextureFromFile( const std::wstring& fileName,
                                                  TextureUsage        textureUsage = TextureUsage::Albedo );

    /**
     * Load a scene file.
     */
    std::shared_ptr<Scene> LoadSceneFromFile( const std::wstring& filname );

    /**
     * Clear a texture.
     */
    void ClearTexture( const std::shared_ptr<Texture>& texture, const float clearColor[4] );

    /**
     * Clear depth/stencil texture.
     */
    void ClearDepthStencilTexture( const std::shared_ptr<Texture>& texture, D3D12_CLEAR_FLAGS clearFlags,
                                   float depth = 1.0f, uint8_t stencil = 0 );

    /**
     * Generate mips for the texture.
     * The first subresource is used to generate the mip chain.
     * Mips are automatically generated for textures loaded from files.
     */
    void GenerateMips( const std::shared_ptr<Texture>& texture );

    /**
     * Generate a cubemap texture from a panoramic (equirectangular) texture.
     */
    void PanoToCubemap( const std::shared_ptr<Texture>& cubemapTexture, const std::shared_ptr<Texture>& panoTexture );

    /**
     * Copy subresource data to a texture.
     */
    void CopyTextureSubresource( const std::shared_ptr<Texture>& texture, uint32_t firstSubresource,
                                 uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData );

    /**
     * Set a dynamic constant buffer data to an inline descriptor in the root
     * signature.
     */
    void SetGraphicsDynamicConstantBuffer( uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData );
    template<typename T>
    void SetGraphicsDynamicConstantBuffer( uint32_t rootParameterIndex, const T& data )
    {
        SetGraphicsDynamicConstantBuffer( rootParameterIndex, sizeof( T ), &data );
    }

    /**
     * Set a set of 32-bit constants on the graphics pipeline.
     */
    void SetGraphics32BitConstants( uint32_t rootParameterIndex, uint32_t numConstants, const void* constants );
    template<typename T>
    void SetGraphics32BitConstants( uint32_t rootParameterIndex, const T& constants )
    {
        static_assert( sizeof( T ) % sizeof( uint32_t ) == 0, "Size of type must be a multiple of 4 bytes" );
        SetGraphics32BitConstants( rootParameterIndex, sizeof( T ) / sizeof( uint32_t ), &constants );
    }

    /**
     * Set a set of 32-bit constants on the compute pipeline.
     */
    void SetCompute32BitConstants( uint32_t rootParameterIndex, uint32_t numConstants, const void* constants );
    template<typename T>
    void SetCompute32BitConstants( uint32_t rootParameterIndex, const T& constants )
    {
        static_assert( sizeof( T ) % sizeof( uint32_t ) == 0, "Size of type must be a multiple of 4 bytes" );
        SetCompute32BitConstants( rootParameterIndex, sizeof( T ) / sizeof( uint32_t ), &constants );
    }

    /**
     * Set the vertex buffer to the rendering pipeline.
     *
     * @param slot The slot to bind the vertex buffer to.
     * @vertexBuffer The vertex buffer to bind (can be null to remove the vertex buffer from the slot).
     */
    void SetVertexBuffers( uint32_t startSlot, const std::vector<std::shared_ptr<VertexBuffer>>& vertexBufferViews );
    void SetVertexBuffer( uint32_t slot, const std::shared_ptr<VertexBuffer>& vertexBufferView );

    /**
     * Set dynamic vertex buffer data to the rendering pipeline.
     */
    void SetDynamicVertexBuffer( uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData );
    template<typename T>
    void SetDynamicVertexBuffer( uint32_t slot, const std::vector<T>& vertexBufferData )
    {
        SetDynamicVertexBuffer( slot, vertexBufferData.size(), sizeof( T ), vertexBufferData.data() );
    }

    /**
     * Bind the index buffer to the rendering pipeline.
     *
     * @param indexBuffer The index buffer to bind to the rendering pipeline.
     */
    void SetIndexBuffer( const std::shared_ptr<IndexBuffer>& indexBuffer );

    /**
     * Bind dynamic index buffer data to the rendering pipeline.
     */
    void SetDynamicIndexBuffer( size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData );
    template<typename T>
    void SetDynamicIndexBuffer( const std::vector<T>& indexBufferData )
    {
        static_assert( sizeof( T ) == 2 || sizeof( T ) == 4 );

        DXGI_FORMAT indexFormat = ( sizeof( T ) == 2 ) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        SetDynamicIndexBuffer( indexBufferData.size(), indexFormat, indexBufferData.data() );
    }

    /**
     * Set dynamic structured buffer contents.
     */
    void SetGraphicsDynamicStructuredBuffer( uint32_t slot, size_t numElements, size_t elementSize,
                                             const void* bufferData );
    template<typename T>
    void SetGraphicsDynamicStructuredBuffer( uint32_t slot, const std::vector<T>& bufferData )
    {
        SetGraphicsDynamicStructuredBuffer( slot, bufferData.size(), sizeof( T ), bufferData.data() );
    }

    /**
     * Set viewports.
     */
    void SetViewport( const D3D12_VIEWPORT& viewport );
    void SetViewports( const std::vector<D3D12_VIEWPORT>& viewports );

    /**
     * Set scissor rects.
     */
    void SetScissorRect( const D3D12_RECT& scissorRect );
    void SetScissorRects( const std::vector<D3D12_RECT>& scissorRects );

    /**
     * Set the pipeline state object on the command list.
     */
    void SetPipelineState( const std::shared_ptr<PipelineStateObject>& pipelineState );

    /**
     * Set the current root signature on the command list.
     */
    void SetGraphicsRootSignature( const std::shared_ptr<RootSignature>& rootSignature );
    void SetComputeRootSignature( const std::shared_ptr<RootSignature>& rootSignature );

    /**
     * Set an inline CBV.
     *
     * Note: Only ConstantBuffer's can be used with inline CBV's.
     */
    void SetConstantBufferView( uint32_t rootParameterIndex, const std::shared_ptr<ConstantBuffer>& buffer,
                                D3D12_RESOURCE_STATES stateAfter   = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                                size_t                bufferOffset = 0 );

    /**
     * Set an inline SRV.
     *
     * Note: Only Buffer resources can be used with inline SRV's
     */
    void SetShaderResourceView( uint32_t rootParameterIndex, const std::shared_ptr<Buffer>& buffer,
                                D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                                                                   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                                size_t bufferOffset = 0 );
    /**
     * Set an inline UAV.
     *
     * Note: Only Buffer resoruces can be used with inline UAV's.
     */
    void SetUnorderedAccessView( uint32_t rootParameterIndex, const std::shared_ptr<Buffer>& buffer,
                                 D3D12_RESOURCE_STATES stateAfter   = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                 size_t                bufferOffset = 0 );

    /**
     * Set the CBV on the rendering pipeline.
     */
    void SetConstantBufferView( uint32_t rootParameterIndex, uint32_t descriptorOffset,
                                const std::shared_ptr<ConstantBufferView>& cbv,
                                D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER );

    /**
     * Set the SRV on the graphics pipeline.
     */
    void SetShaderResourceView( uint32_t rootParameterIndex, uint32_t descriptorOffset,
                                const std::shared_ptr<ShaderResourceView>& srv,
                                D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                                                                   D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                                UINT firstSubresource = 0,
                                UINT numSubresources  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES );

    /**
     * Set the UAV on the graphics pipeline.
     */
    void SetUnorderedAccessView( uint32_t rootParameterIndex, uint32_t descrptorOffset,
                                 const std::shared_ptr<UnorderedAccessView>& uav,
                                 D3D12_RESOURCE_STATES stateAfter       = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                 UINT                  firstSubresource = 0,
                                 UINT                  numSubresources  = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES );

    /**
     * Set the render targets for the graphics rendering pipeline.
     */
    void SetRenderTarget( const RenderTarget& renderTarget );

    /**
     * Draw geometry.
     */
    void Draw( uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0 );
    void DrawIndexed( uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0,
                      uint32_t startInstance = 0 );

    /**
     * Dispatch a compute shader.
     */
    void Dispatch( uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1 );

protected:
    friend class CommandQueue;
    friend class DynamicDescriptorHeap;
    friend class std::default_delete<CommandList>;

    CommandList( Device& device, D3D12_COMMAND_LIST_TYPE type );
    virtual ~CommandList();

    /**
     * Close the command list.
     * Used by the command queue.
     *
     * @param pendingCommandList The command list that is used to execute pending
     * resource barriers (if any) for this command list.
     *
     * @return true if there are any pending resource barriers that need to be
     * processed.
     */
    bool Close( const std::shared_ptr<CommandList>& pendingCommandList );
    // Just close the command list. This is useful for pending command lists.
    void Close();

    /**
     * Reset the command list. This should only be called by the CommandQueue
     * before the command list is returned from CommandQueue::GetCommandList.
     */
    void Reset();

    /**
     * Release tracked objects. Useful if the swap chain needs to be resized.
     */
    void ReleaseTrackedObjects();

    /**
     * Set the currently bound descriptor heap.
     * Should only be called by the DynamicDescriptorHeap class.
     */
    void SetDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap );

    std::shared_ptr<CommandList> GetGenerateMipsCommandList() const
    {
        return m_ComputeCommandList;
    }

private:
    void TrackResource( Microsoft::WRL::ComPtr<ID3D12Object> object );
    void TrackResource( const std::shared_ptr<Resource>& res );

    // Generate mips for UAV compatible textures.
    void GenerateMips_UAV( const std::shared_ptr<Texture>& texture, bool isSRGB );

    // Copy the contents of a CPU buffer to a GPU buffer (possibly replacing the previous buffer contents).
    Microsoft::WRL::ComPtr<ID3D12Resource> CopyBuffer( size_t bufferSize, const void* bufferData,
                                                       D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE );

    // Binds the current descriptor heaps to the command list.
    void BindDescriptorHeaps();

    // The device that is used to create this command list.
    Device&                                            m_Device;
    D3D12_COMMAND_LIST_TYPE                            m_d3d12CommandListType;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_d3d12CommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>     m_d3d12CommandAllocator;

    // For copy queues, it may be necessary to generate mips while loading textures.
    // Mips can't be generated on copy queues but must be generated on compute or
    // direct queues. In this case, a Compute command list is generated and executed
    // after the copy queue is finished uploading the first sub resource.
    std::shared_ptr<CommandList> m_ComputeCommandList;

    // Keep track of the currently bound root signatures to minimize root
    // signature changes.
    ID3D12RootSignature* m_RootSignature;

    // Resource created in an upload heap. Useful for drawing of dynamic geometry
    // or for uploading constant buffer data that changes every draw call.
    std::unique_ptr<UploadBuffer> m_UploadBuffer;

    // Resource state tracker is used by the command list to track (per command list)
    // the current state of a resource. The resource state tracker also tracks the
    // global state of a resource in order to minimize resource state transitions.
    std::unique_ptr<ResourceStateTracker> m_ResourceStateTracker;

    // The dynamic descriptor heap allows for descriptors to be staged before
    // being committed to the command list. Dynamic descriptors need to be
    // committed before a Draw or Dispatch.
    std::unique_ptr<DynamicDescriptorHeap> m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    // Keep track of the currently bound descriptor heaps. Only change descriptor
    // heaps if they are different than the currently bound descriptor heaps.
    ID3D12DescriptorHeap* m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    // Pipeline state object for Mip map generation.
    std::unique_ptr<GenerateMipsPSO> m_GenerateMipsPSO;
    // Pipeline state object for converting panorama (equirectangular) to cubemaps
    std::unique_ptr<PanoToCubemapPSO> m_PanoToCubemapPSO;

    using TrackedObjects = std::vector<Microsoft::WRL::ComPtr<ID3D12Object>>;

    // Objects that are being tracked by a command list that is "in-flight" on
    // the command-queue and cannot be deleted. To ensure objects are not deleted
    // until the command list is finished executing, a reference to the object
    // is stored. The referenced objects are released when the command list is
    // reset.
    TrackedObjects m_TrackedObjects;

    // Keep track of loaded textures to avoid loading the same texture multiple times.
    static std::map<std::wstring, ID3D12Resource*> ms_TextureCache;
    static std::mutex                              ms_TextureCacheMutex;
};
}  // namespace dx12lib