/**
 * CommandList class encapsulates a ID3D12GraphicsCommandList2 interface.
 * The CommandList class provides additional functionality that makes working with 
 * DirectX 12 applications easier.
 */

#include <d3d12.h>
#include <wrl.h>

#include <memory> // for std::unique_ptr
#include <vector> // for std::vector

class DynamicDescriptorHeap;
class Resource;
class ResourceStateTracker;
class UploadBuffer;

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
        return m_CommandListType;
    }

    /**
     * Get direct access to the ID3D12GraphicsCommandList2 interface.
     */
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetGraphicsCommandList() const
    {
        return m_GraphicsCommandList;
    }

    /**
     * Transition a resource to a particular state.
     * 
     * @param resource The resource to transition.
     * @param state The state to transition the resource to.
     * @param flushBarriers Force flush any barriers. Resource barriers need to be 
     * flushed before a command (draw, dispatch, or copy) that expects the resource 
     * to be in a particular state can run.
     */
    void TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES state, bool flushBarriers = false);

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
     * Bind a dynamic constant buffer data to an inline descriptor in the root
     * signature.
     */
    void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData);

    /**
     * Reset the command list. This should only be called by the CommandQueue
     * before the command list is returned from CommandQueue::GetCommandList.
     */
    void Reset();

protected:

private:
    using D3D12ObjectList = std::vector < Microsoft::WRL::ComPtr<ID3D12Object> >;

    D3D12_COMMAND_LIST_TYPE m_CommandListType;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_GraphicsCommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;

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


    // Objects that are being referenced by a command list that is "in-flight" on 
    // the command-queue cannot be deleted. To ensure objects are not deleted 
    // until the command list is finished executing, a reference to the object
    // is stored. The referenced objects are released when the command list is 
    // reset.
    D3D12ObjectList m_ReferencedObjects;
};