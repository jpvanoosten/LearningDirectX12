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

#include "d3dx12.h"

#include <unordered_set>
#include <vector>


class ResourceStateTracker
{
public:
    ResourceStateTracker();
    virtual ~ResourceStateTracker();

    
protected:

private:
    // The current state of a (sub)resource.
    struct SubresourceState
    {
        SubresourceState( ID3D12Resource* resource, UINT subresource, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON )
            : Resource(resource)
            , Subresource(subresource)
            , State(state)
        {}

        bool operator==( const SubresourceState& other )
        {
            return Resource == other.Resource &&
                (Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES || other.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES || Subresource == other.Subresource);
        }

        ID3D12Resource* Resource;
        UINT Subresource;
        D3D12_RESOURCE_STATES State;
    };

    struct SubresourceStateHasher
    {
        // Provide a hashing function for SubresourceState.
        // A SubresourceState object is uniquely identified by its resource pointer 
        // and its subresource. Using only the resource pointer as a hash value may
        // cause collisions in a std::set or std::unordered_set but these will 
        // be resolved by the SubresourceState::operator== function.
        size_t operator()(const SubresourceState& subresourceState)
        {
            return reinterpret_cast<size_t>(subresourceState.Resource);
        }
    };

    // Pending resource transitions.
    std::vector<D3D12_RESOURCE_BARRIER> m_PendingResourceBarriers;

    // Global (sub)resource state.
    using GlobalResourceState = std::unordered_set<SubresourceState, SubresourceStateHasher>;
    static GlobalResourceState ms_GlobalResourceState;

};