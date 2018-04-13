/**
 * The ResourceStateTracker tracks the known state of a (sub)resource within a command list.
 * It is often difficult (or impossible) to know the current state of a (sub)resource if
 * it is being used in multiple command lists. For example when doing shadow mapping,
 * a depth buffer resource is being used as a depth-stencil view in a command list
 * that is generating the shadow map for a light source, but needs to be used as
 * a shader-resource view in a command list that is performing shadow mapping. If
 * the command lists are being generated in separate threads, the exact state of the 
 * resource can't be guaranteed at the moment it is used in the command list.
 * The ResourceStateTracker class is intended to be used within a command list
 * to track the state of the resource as it is known within that command list.
 * 
 * See: https://youtu.be/nmB2XMasz2o
 * See: https://msdn.microsoft.com/en-us/library/dn899226(v=vs.85).aspx#implicit_state_transitions
 */
#pragma once

#include "d3dx12.h"

#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>

class CommandList;
class Resource;

class ResourceStateTracker
{
public:
    ResourceStateTracker();
    virtual ~ResourceStateTracker();

    /**
     * Push a resource barrier to the resource state tracker.
     * 
     * @param barrier The resource barrier to push to the state tracker.
     * @param flushImmediately Force flush any resource barriers that have been pushed
     * to the command list.
     */
    void ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);

    /**
     * Push a transition resource barrier to the resource state tracker.
     * 
     * @param resource The resource to transition.
     * @param stateAfter The state to transition the resource to.
     * @param subResource The subresource to transition. By default, this is D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
     * which indicates that all subresources should be transitioned to the same state.
     */
    void TransitionResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    /**
     * Push a UAV resource barrier for the given resource.
     * 
     * @param resource The resource to add a UAV barrier for. Can be NULL which 
     * indicates that UAV access could require the barrier.
     */
    void UAVBarrier(const Resource* resource = nullptr);

    /**
     * Push an aliasing barrier for the given resource.
     * 
     * @param beforeResource The resource currently occupying the space in the heap.
     * @param afterResource The resource that will be occupying the space in the heap.
     * 
     * Either the beforeResource or the afterResource parameters can be NULL which 
     * indicates that any placed or reserved resource could cause aliasing.
     */
    void AliasBarrier(const Resource* resourceBefore = nullptr, const Resource* resourceAfter = nullptr);


    /**
     * Check to see if there are any pending resource barriers.
     */
    const bool HasPendingResourceBarriers() const
    {
        return !m_PendingResourceBarriers.empty();
    }

    /**
     * Flush any pending split resource barriers to the command list.
     */
    void FlushPendingResourceBarriers(CommandList& commandList);

    /**
     * Flush any resource barriers that have been pushed to the resource state
     * tracker.
     */
    void FlushResourceBarriers(CommandList& commandList);

    /**
     * Commit final resource states to the global resource state array (map)
     * This must be called when the command list is closed.
     */
    void CommitFinalResourceStates();

    /**
     * Reset state tracking. This must be done when the command list is reset.
     */
    void Reset();

    /**
     * Add a resource with a given state to the global resource state array (map).
     * This should be done when the resource is created for the first time.
     */
    static void AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);

    /**
     * Remove a resource from the global resource state array (map).
     * This should be done when the resource is destroyed.
     */
    static void RemoveGlobalResourceState(ID3D12Resource* resource);

protected:

private:
    // An array (vector) of resource barriers.
    using ResourceBarriers = std::vector<D3D12_RESOURCE_BARRIER>;

    // Tracks the state of a particular resource and all of its subresources.
    struct ResourceState
    {
        // Initialize all of the subresources within a resource to the given state.
        ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON)
            : State(state)
        {}

        // Returns true if any subresource defines a state.
        // If false, then all (sub)resources have the same state.
        bool HasSubresourceState() const
        {
            return !SubresourceState.empty();
        }

        // Get the state of a (sub)resource within the resource.
        // If the specified subresource is not found in the SubresourceState array (map)
        // then the state of the resource (D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) is
        // returned.
        D3D12_RESOURCE_STATES GetSubresourceState(UINT subresource) const
        {
            D3D12_RESOURCE_STATES state = State;
            const auto iter = SubresourceState.find(subresource);
            if (iter != SubresourceState.end())
            {
                state = iter->second;
            }
            return state;
        }

        // Set a subresource to a particular state.
        void SetSubresourceState(UINT subresource, D3D12_RESOURCE_STATES state)
        {
            if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                State = state;
                SubresourceState.clear();
            }
            else
            {
                SubresourceState[subresource] = state;
            }
        }

        // If the SubresourceState array (map) is empty, then the State variable defines 
        // the state of all of the subresources.
        D3D12_RESOURCE_STATES State;
        std::map<UINT, D3D12_RESOURCE_STATES> SubresourceState;
    };

    // Pending resource transitions are committed before a command list
    // is executed on the command queue. This guarantees that resources will
    // be in the expected state at the beginning of a command list.
    ResourceBarriers m_PendingResourceBarriers;

    // Resource barriers that need to be committed to the command list.
    ResourceBarriers m_ResourceBarriers;

    using ResourceStateMap = std::unordered_map<ID3D12Resource*, ResourceState>;

    // The final (last known state) of the resources within a command list.
    // The final resource state is committed to the global resource state when the 
    // command list is closed but before it is executed on the command queue.
    ResourceStateMap m_FinalResourceState;

    // The mutex protects shared access to the GlobalResourceState map.
    static std::mutex ms_GlobalMutex;
    // The global resource state array (map) stores the state of a resource
    // between command list execution.
    static ResourceStateTracker::ResourceStateMap ms_GlobalResourceState;
};