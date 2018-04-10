#include <DX12LibPCH.h>

#include <ResourceStateTracker.h>

#include <CommandList.h>

// Static definitions.
std::mutex ResourceStateTracker::ms_GlobalMutex;
ResourceStateTracker::ResourceStateMap ResourceStateTracker::ms_GlobalResourceState;

ResourceStateTracker::ResourceStateTracker()
{}

ResourceStateTracker::~ResourceStateTracker()
{}

void ResourceStateTracker::PushResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
{
    if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
    {
        const D3D12_RESOURCE_TRANSITION_BARRIER& transitionBarrier = barrier.Transition;

        // First check if there is already a known "final" state for the given resource.
        // If there is, the resource has been used on the command list before and
        // already has a known state within the command list execution.
        const auto iter = m_FinalResourceState.find(transitionBarrier.pResource);
        if (iter != m_FinalResourceState.end())
        {
            // If the known final state of the resource is different...
            auto& finalState = iter->second;
            if (finalState[transitionBarrier.Subresource] != transitionBarrier.StateAfter )
            {
                // Push a new transition barrier with the correct before state.
                D3D12_RESOURCE_BARRIER newBarrier = barrier;
                newBarrier.Transition.StateBefore = finalState[transitionBarrier.Subresource];
                m_ResourceBarriers.push_back(newBarrier);
            }
        }
        else // In this case, the resource is being used on the command list for the first time...
        {
            // Check to see if there is a known global state for the resource.
            std::lock_guard<std::mutex> lock(ms_GlobalMutex);
            const auto iter = ms_GlobalResourceState.find(transitionBarrier.pResource);
            if (iter != ms_GlobalResourceState.end())
            {
                // If the global state of the (sub) resource is different...
                auto& globalState = iter->second;
                if (globalState[transitionBarrier.Subresource] != transitionBarrier.StateAfter)
                {
                    // Create a pending barrier to begin the transition.
                    D3D12_RESOURCE_BARRIER pendingBarrier = barrier;
                    pendingBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
                    pendingBarrier.Transition.StateBefore = globalState[transitionBarrier.Subresource];
                    m_PendingResourceBarriers.push_back(pendingBarrier);
                    pendingBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
                    m_ResourceBarriers.push_back(pendingBarrier);
                }
            }
            else
            {
                // This shouldn't happen. Resource must be added to the global resource state
                // on resource creation. Resources should only be removed from the global resource state
                // array on resource destruction.
                throw std::invalid_argument("Resources must be registered using the ResourceStateTracker::AddGlobalResourceState method. Are you using the CommandList class to create and bind resources?");
            }
        }

        // Push the final known state (possibly replacing the previously known state).
        m_FinalResourceState[transitionBarrier.pResource][transitionBarrier.Subresource] = transitionBarrier.StateAfter;
    }
    else
    {
        // Just push the barrier to the resource barriers array.
        m_ResourceBarriers.push_back(barrier);
    }
}

void ResourceStateTracker::FlushResourceBarriers(CommandList& commandList)
{
    // TODO: Call ID3D12GraphicsCommandList::ResourceBarrier method with m_ResourceBarriers
}

void ResourceStateTracker::FlushPendingResourceBarriers(CommandList& commandList)
{
    // TODO: Call ID3D12GraphicsCommandList::ResourceBarrier method with m_PendingResourceBarriers
}

void ResourceStateTracker::CommitFinalResourceStates()
{
    // Commit final resource states to the global resource state array (map).
    std::lock_guard<std::mutex> lock(ms_GlobalMutex);
    for (const auto& resourceState : m_FinalResourceState)
    {
        ID3D12Resource* resource = resourceState.first;
        for (const auto& subresourceState : resourceState.second.SubresourceState)
        {
            UINT subresource = subresourceState.first;
            D3D12_RESOURCE_STATES state = subresourceState.second;
            ms_GlobalResourceState[resource][subresource] = state;
        }
    }
}

void ResourceStateTracker::Reset()
{
    // Reset the pending, current, and final resource states.
    m_PendingResourceBarriers.clear();
    m_ResourceBarriers.clear();
    m_FinalResourceState.clear();
}

void ResourceStateTracker::AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
{
    if ( resource != nullptr )
    {
        std::lock_guard<std::mutex> lock(ms_GlobalMutex);
        ms_GlobalResourceState[resource][D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES] = state;
    }
}

void ResourceStateTracker::RemoveGlobalResourceState(ID3D12Resource* resource)
{
    if ( resource != nullptr )
    {
        std::lock_guard<std::mutex> lock(ms_GlobalMutex);
        ms_GlobalResourceState.erase(resource);
    }
}
