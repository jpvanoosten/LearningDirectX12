#include <DX12LibPCH.h>

#include <ResourceStateTracker.h>

#include <CommandList.h>
#include <Resource.h>

// Static definitions.
std::mutex ResourceStateTracker::ms_GlobalMutex;
bool ResourceStateTracker::ms_IsLocked = false;
ResourceStateTracker::ResourceStateMap ResourceStateTracker::ms_GlobalResourceState;

ResourceStateTracker::ResourceStateTracker()
{}

ResourceStateTracker::~ResourceStateTracker()
{}

void ResourceStateTracker::ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
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
            auto& resourceState = iter->second;
            // If the known final state of the resource is different...
            if ( transitionBarrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
                 !resourceState.SubresourceState.empty() )
            {
                // First transition all of the subresources if they are different than the StateAfter.
                for ( auto subresourceState : resourceState.SubresourceState )
                {
                    if ( transitionBarrier.StateAfter != subresourceState.second )
                    {
                        D3D12_RESOURCE_BARRIER newBarrier = barrier;
                        newBarrier.Transition.Subresource = subresourceState.first;
                        newBarrier.Transition.StateBefore = subresourceState.second;
                        m_ResourceBarriers.push_back( newBarrier );
                    }
                }
            }
            else
            {
                auto finalState = resourceState.GetSubresourceState( transitionBarrier.Subresource );
                if ( transitionBarrier.StateAfter != finalState )
                {
                    // Push a new transition barrier with the correct before state.
                    D3D12_RESOURCE_BARRIER newBarrier = barrier;
                    newBarrier.Transition.StateBefore = finalState;
                    m_ResourceBarriers.push_back( newBarrier );
                }
            }
        }
        else // In this case, the resource is being used on the command list for the first time. 
        {
            // Add a pending barrier. The pending barriers will be resolved
            // before the command list is executed on the command queue.
            m_PendingResourceBarriers.push_back(barrier);
        }

        // Push the final known state (possibly replacing the previously known state for the subresource).
        m_FinalResourceState[transitionBarrier.pResource].SetSubresourceState(transitionBarrier.Subresource, transitionBarrier.StateAfter);
    }
    else
    {
        // Just push non-transition barriers to the resource barriers array.
        m_ResourceBarriers.push_back(barrier);
    }
}

void ResourceStateTracker::TransitionResource( ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource )
{
    if ( resource )
    {
        ResourceBarrier( CD3DX12_RESOURCE_BARRIER::Transition( resource, D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource ) );
    }
}

void ResourceStateTracker::TransitionResource( const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource )
{
    TransitionResource( resource.GetD3D12Resource().Get(), stateAfter, subResource );
}

void ResourceStateTracker::UAVBarrier(const Resource* resource )
{
    ID3D12Resource* pResource = resource != nullptr ? resource->GetD3D12Resource().Get() : nullptr;

    ResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pResource));
}

void ResourceStateTracker::AliasBarrier(const Resource* resourceBefore, const Resource* resourceAfter)
{
    ID3D12Resource* pResourceBefore = resourceBefore != nullptr ? resourceBefore->GetD3D12Resource().Get() : nullptr;
    ID3D12Resource* pResourceAfter = resourceAfter != nullptr ? resourceAfter->GetD3D12Resource().Get() : nullptr;

    ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Aliasing(pResourceBefore, pResourceAfter));
}

void ResourceStateTracker::FlushResourceBarriers(CommandList& commandList)
{
    UINT numBarriers = static_cast<UINT>(m_ResourceBarriers.size());
    if (numBarriers > 0 )
    {
        auto d3d12CommandList = commandList.GetGraphicsCommandList();
        d3d12CommandList->ResourceBarrier(numBarriers, m_ResourceBarriers.data());
        m_ResourceBarriers.clear();
    }
}

uint32_t ResourceStateTracker::FlushPendingResourceBarriers(CommandList& commandList)
{
    assert(ms_IsLocked);

    // Resolve the pending resource barriers by checking the global state of the 
    // (sub)resources. Add barriers if the pending state and the global state do
    //  not match.
    ResourceBarriers resourceBarriers;
    // Reserve enough space (worst-case, all pending barriers).
    resourceBarriers.reserve(m_PendingResourceBarriers.size());

    for (auto pendingBarrier : m_PendingResourceBarriers)
    {
        if (pendingBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)  // Only transition barriers should be pending...
        {
            auto pendingTransition = pendingBarrier.Transition;
            
            const auto& iter = ms_GlobalResourceState.find(pendingTransition.pResource);
            if (iter != ms_GlobalResourceState.end())
            {
                // If all subresources are being transitioned, and there are multiple
                // subresources of the resource that are in a different state...
                auto& resourceState = iter->second;
                if ( pendingTransition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
                     !resourceState.SubresourceState.empty() )
                {
                    // Transition all subresources
                    for ( auto subresourceState : resourceState.SubresourceState )
                    {
                        if ( pendingTransition.StateAfter != subresourceState.second )
                        {
                            D3D12_RESOURCE_BARRIER newBarrier = pendingBarrier;
                            newBarrier.Transition.Subresource = subresourceState.first;
                            newBarrier.Transition.StateBefore = subresourceState.second;
                            resourceBarriers.push_back( newBarrier );
                        }
                    }
                }
                else
                {
                    // No (sub)resources need to be transitioned. Just add a single transition barrier (if needed).
                    auto globalState = ( iter->second ).GetSubresourceState( pendingTransition.Subresource );
                    if ( pendingTransition.StateAfter != globalState )
                    {
                        // Fix-up the before state based on current global state of the resource.
                        pendingBarrier.Transition.StateBefore = globalState;
                        resourceBarriers.push_back( pendingBarrier );
                    }
                }
            }
        }
    }

    UINT numBarriers = static_cast<UINT>(resourceBarriers.size());
    if (numBarriers > 0 )
    {
        auto d3d12CommandList = commandList.GetGraphicsCommandList();
        d3d12CommandList->ResourceBarrier(numBarriers, resourceBarriers.data());
    }

    m_PendingResourceBarriers.clear();

    return numBarriers;
}

void ResourceStateTracker::CommitFinalResourceStates()
{
    assert(ms_IsLocked);

    // Commit final resource states to the global resource state array (map).
    for (const auto& resourceState : m_FinalResourceState)
    {
        ms_GlobalResourceState[resourceState.first] = resourceState.second;
    }

    m_FinalResourceState.clear();
}

void ResourceStateTracker::Reset()
{
    // Reset the pending, current, and final resource states.
    m_PendingResourceBarriers.clear();
    m_ResourceBarriers.clear();
    m_FinalResourceState.clear();
}

void ResourceStateTracker::Lock()
{
    ms_GlobalMutex.lock();
    ms_IsLocked = true;
}

void ResourceStateTracker::Unlock()
{
    ms_GlobalMutex.unlock();
    ms_IsLocked = false;
}

void ResourceStateTracker::AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
{
    if ( resource != nullptr )
    {
        std::lock_guard<std::mutex> lock(ms_GlobalMutex);
        ms_GlobalResourceState[resource].SetSubresourceState(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
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
