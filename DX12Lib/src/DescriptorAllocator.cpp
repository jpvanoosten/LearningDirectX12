#include "DX12LibPCH.h"

#include <dx12lib/DescriptorAllocator.h>

#include <dx12lib/DescriptorAllocatorPage.h>

using namespace dx12lib;

// Adapter for make_shared
struct MakeAllocatorPage : public DescriptorAllocatorPage
{
public:
    MakeAllocatorPage( Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors )
    : DescriptorAllocatorPage(device, type, numDescriptors )
    {}

    virtual ~MakeAllocatorPage() {}
};

DescriptorAllocator::DescriptorAllocator( Device& device, D3D12_DESCRIPTOR_HEAP_TYPE type,
                                          uint32_t numDescriptorsPerHeap )
: m_Device( device )
, m_HeapType( type )
, m_NumDescriptorsPerHeap( numDescriptorsPerHeap )
{}

DescriptorAllocator::~DescriptorAllocator() {}

std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::CreateAllocatorPage()
{
    std::shared_ptr<DescriptorAllocatorPage> newPage = std::make_shared<MakeAllocatorPage>( m_Device, m_HeapType, m_NumDescriptorsPerHeap );

    m_HeapPool.emplace_back( newPage );
    m_AvailableHeaps.insert( m_HeapPool.size() - 1 );

    return newPage;
}

DescriptorAllocation DescriptorAllocator::Allocate( uint32_t numDescriptors )
{
    std::lock_guard<std::mutex> lock( m_AllocationMutex );

    DescriptorAllocation allocation;

    auto iter = m_AvailableHeaps.begin();
    while ( iter != m_AvailableHeaps.end() )
    {
        auto allocatorPage = m_HeapPool[*iter];

        allocation = allocatorPage->Allocate( numDescriptors );

        if ( allocatorPage->NumFreeHandles() == 0 )
        {
            iter = m_AvailableHeaps.erase( iter );
        }
        else
        {
            ++iter;
        }

        // A valid allocation has been found.
        if ( !allocation.IsNull() )
        {
            break;
        }
    }

    // No available heap could satisfy the requested number of descriptors.
    if ( allocation.IsNull() )
    {
        m_NumDescriptorsPerHeap = std::max( m_NumDescriptorsPerHeap, numDescriptors );
        auto newPage            = CreateAllocatorPage();

        allocation = newPage->Allocate( numDescriptors );
    }

    return allocation;
}

void DescriptorAllocator::ReleaseStaleDescriptors()
{
    std::lock_guard<std::mutex> lock( m_AllocationMutex );

    for ( size_t i = 0; i < m_HeapPool.size(); ++i )
    {
        auto page = m_HeapPool[i];

        page->ReleaseStaleDescriptors();

        if ( page->NumFreeHandles() > 0 )
        {
            m_AvailableHeaps.insert( i );
        }
    }
}
