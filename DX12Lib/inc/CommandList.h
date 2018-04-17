/**
 * CommandList class encapsulates a ID3D12GraphicsCommandList2 interface.
 * The CommandList class provides additional functionality that makes working with 
 * DirectX 12 applications easier.
 */
#pragma once

#include <d3d12.h>
#include <wrl.h>

#include <memory> // for std::unique_ptr
#include <vector> // for std::vector

class Buffer;
class ConstantBuffer;
class DynamicDescriptorHeap;
class IndexBuffer;
class Resource;
class ResourceStateTracker;
class UploadBuffer;
class VertexBuffer;

class CommandList
{
public:
    CommandList(D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandList();

    /**
     * Get the type of command list. 
     */
    D3D12_COMMAND_LIST_TYPE GetCommandListType() const
    {
        return m_d3d12CommandListType;
    }

    /**
     * Get direct access to the ID3D12GraphicsCommandList2 interface.
     */
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetGraphicsCommandList() const
    {
        return m_d3d12CommandList;
    }

    /**
     * Transition a resource to a particular state.
     * 
     * @param resource The resource to transition.
     * @param stateAfter The state to transition the resource to. The before state is resolved by the resource state tracker.
     * @param subresource The subresource to transition. By default, this is D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES which indicates that all subresources are transitioned to the same state.
     * @param flushBarriers Force flush any barriers. Resource barriers need to be flushed before a command (draw, dispatch, or copy) that expects the resource to be in a particular state can run.
     */
    void TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);

    /**
     * Add a UAV barrier to ensure that any writes to a resource have completed
     * before reading from the resource.
     * 
     * @param resource The resource to add a UAV barrier for.
     * @param flushBarriers Force flush any barriers. Resource barriers need to be
     * flushed before a command (draw, dispatch, or copy) that expects the resource
     * to be in a particular state can run.
     */
    void UAVBarrier(const Resource& resource, bool flushBarriers = false);

    /**
     * Flush any barriers that have been pushed to the command list.
     */
    void FlushResourceBarriers();

    /**
     * Set the contents of a vertex buffer.
     */
    void SetVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData);
    template<typename T>
    void SetVertexBuffer(VertexBuffer& vertexBuffer, const std::vector<T>& vertexBufferData)
    {
        SetVertexBuffer(vertexBuffer, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
    }

    /**
     * Set the contents of an index buffer.
     */
    void SetIndexBuffer(IndexBuffer& indexBuffer, size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData);
    template<typename T>
    void SetIndexBuffer(IndexBuffer& indexBuffer, const std::vector<T>& indexBufferData)
    {
        static_assert(sizeof(T) == 2 || sizeof(T) == 4);

        DXGI_FORMAT indexFormat = ( sizeof(T) == 2 ) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        SetIndexBuffer(indexBuffer, indexBufferData.size(), indexFormat, indexBufferData.data());
    }

    /**
     * Bind a dynamic constant buffer data to an inline descriptor in the root
     * signature.
     */
    void BindGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData);
    template<typename T>
    void BindGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, const T& data )
    {
        BindGraphicsDynamicConstantBuffer(rootParameterIndex, sizeof(T), &data);
    }

    /**
     * Bind a set of 32-bit constants to the graphics pipeline.
     */
    void BindGraphics32BitConstants(uint32_t rootParameterIndex, uint32_t numConstants, const void* constants);
    template<typename T>
    void BindGraphics32BitConstants(uint32_t rootParameterIndex, const T& constants)
    {
        static_assert(sizeof(T) % sizeof(uint32_t) == 0, "Size of type must be a multiple of 4 bytes");
        BindGraphics32BitConstants(rootParameterIndex, sizeof(T) / sizeof(uint32_t), &constants);
    }

    /**
     * Bind the vertex buffer to the rendering pipeline.
     */
    void BindVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer);

    /**
     * Bind dynamic vertex buffer data to the rendering pipeline.
     */
    void BindDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData);
    template<typename T>
    void BindDynamicVertexBuffer(uint32_t slot, const std::vector<T>& vertexBufferData)
    {
        BindDynamicVertexBuffer(slot, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
    }


    /**
     * Bind the index buffer to the rendering pipeline.
     */
    void BindIndexBuffer(const IndexBuffer& indexBuffer);

    /**
     * Bind dynamic index buffer data to the rendering pipeline.
     */
    void BindDynamicIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData);
    template<typename T>
    void BindDynamicIndexBuffer(const std::vector<T>& indexBufferData)
    {
        static_assert(sizeof(T) == 2 || sizeof(T) == 4);

        DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        BindDynamicIndexBuffer(indexBufferData.size(), indexFormat, indexBufferData.data());
    }

    /**
     * Close the command list.
     * Used by the command queue.
     * 
     * @param pendingCommandList The command list that is used to execute pending
     * resource barriers (if any) for this command list.
     */
    void Close( CommandList& pendingCommandList );
    // Just close the command list. This is useful for pending command lists.
    void Close();

    /**
     * Reset the command list. This should only be called by the CommandQueue
     * before the command list is returned from CommandQueue::GetCommandList.
     */
    void Reset();

    /**
    * Set the currently bound descriptor heap.
    * Should only be called by the DynamicDescriptorHeap class.
    */
    void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap);

protected:

private:
    // Set the contents of a buffer (possibly replacing the previous buffer contents.
    void SetBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    // Binds the current descriptor heaps to the command list.
    void BindDescriptorHeaps();

    using TrackedObjects = std::vector < Microsoft::WRL::ComPtr<ID3D12Object> >;

    D3D12_COMMAND_LIST_TYPE m_d3d12CommandListType;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_d3d12CommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_d3d12CommandAllocator;

    // Keep track of the currently bound root signatures to minimize root
    // signature changes.
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_CurrentGraphicsRootSignature;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_CurrentComputeRootSignature;

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
    
    // Objects that are being tracked by a command list that is "in-flight" on 
    // the command-queue and cannot be deleted. To ensure objects are not deleted 
    // until the command list is finished executing, a reference to the object
    // is stored. The referenced objects are released when the command list is 
    // reset.
    TrackedObjects m_TrackedObjects;
};