#include <DX12LibPCH.h>

#include <DescriptorAllocator.h>

#include <Application.h>

DescriptorAllocatorPage::DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors )
    : m_HeapType( type )
    , m_NumDescriptorsInHeap( numDescriptors )
{
    auto device = Application::Get().GetDevice();

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = m_HeapType;
    heapDesc.NumDescriptors = m_NumDescriptorsInHeap;

    ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_d3d12DescriptorHeap)));

    m_BaseDescriptor = m_d3d12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_DescriptorHandleIncrementSize = Application::Get().GetDescriptorHandleIncrementSize(m_HeapType);
    m_NumFreeHandles = m_NumDescriptorsInHeap;

    // Initialize the freelists(s)
    AddNewBlock(0, m_NumFreeHandles);
}

void DescriptorAllocatorPage::AddNewBlock(uint32_t offset, uint32_t numDescriptors)
{
    auto offsetIt = m_FreeListByOffset.emplace(offset, numDescriptors);
    auto sizeIt = m_FreeListBySize.emplace(numDescriptors, offsetIt.first);
    offsetIt.first->second.FreeListBySizeIt = sizeIt;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocatorPage::Allocate(uint32_t numDescriptors)
{
    // There are less than the requested number of descriptors left in the heap.
    // Return a NULL descriptor and try another heap.
    if (numDescriptors > m_NumFreeHandles)
    {
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
    }

    // Get the first block that is large enough to satisfy the request.
    auto smallestBlockIt = m_FreeListBySize.lower_bound(numDescriptors);
    if (smallestBlockIt == m_FreeListBySize.end())
    {
        // There was no free block that could satisfy the request.
        return CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
    }

    // The size of the smallest block that satisfies the request.
    auto blockSize = smallestBlockIt->first;

    // The pointer to the same entry in the FreeListByIndex map.
    auto offsetIt = smallestBlockIt->second;

    // The offset in the descriptor heap.
    auto offset = offsetIt->first;
    
    // Compute the new freeblock that results from splitting this block.
    auto newOffset = offset + numDescriptors;
    auto newSize = blockSize - numDescriptors;

    // Remove the existing freeblock from the free list.
    m_FreeListBySize.erase(smallestBlockIt);
    m_FreeListByOffset.erase(offsetIt);

    if (newSize > 0)
    {
        // If the allocation didn't exactly match the requested size,
        // return the left-over to the freelist.
        AddNewBlock(newOffset, newSize);
    }

    // Add the allocated descriptor to the allocation table.
    m_AllocatedDescriptors.emplace(offset, numDescriptors);

    // Decrement free handles.
    m_NumFreeHandles -= numDescriptors;

    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_BaseDescriptor, offset, m_DescriptorHandleIncrementSize);
}

uint32_t DescriptorAllocatorPage::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
    return ( descriptorHandle.ptr - m_BaseDescriptor.ptr ) / m_DescriptorHandleIncrementSize;
}

void DescriptorAllocatorPage::Free(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, uint64_t frameNumber)
{
    // Compute the offset of the descriptor within the descriptor heap.
    auto offset = ComputeOffset(descriptorHandle);

    if (offset >= m_NumDescriptorsInHeap)
    {
        throw std::exception("Computed offset of the descriptor is out of bounds for this descriptor heap.");
    }

    // Don't add the block directly to the freelist until the frame has completed.
    m_StaleDescriptors.emplace(offset, frameNumber);
}

void DescriptorAllocatorPage::FreeBlock(uint32_t offset, uint32_t numDescriptors)
{
    // Find the first element whose offset is greater than the specified offset.
    // This is the block that should appear after the block that is being freed.
    auto nextBlockIt = m_FreeListByOffset.upper_bound(offset);

    // Find the block that appears before the block being freed.
    auto prevBlockIt = nextBlockIt;
    // If it's not the first block in the list.
    if (prevBlockIt != m_FreeListByOffset.begin())
    {
        // Go to the previous block in the list.
        --prevBlockIt;
    }
    else
    {
        // Otherwise, just set it to the end of the list to indicate that no
        // block comes before the one being freed.
        prevBlockIt = m_FreeListByOffset.end();
    }

    if (prevBlockIt != m_FreeListByOffset.end() &&
        offset == prevBlockIt->first + prevBlockIt->second.Size)
    {
        // The previous block is exactly behind the block that is to be freed.
        //
        // PrevBlock.Offset           Offset
        // |                          |
        // |<-----PrevBlock.Size----->|<------Size-------->|
        //

        // Increase the block size by the size of merging with the previous block.
        offset = prevBlockIt->first;
        numDescriptors += prevBlockIt->second.Size;

        // Remove the previous block from the freelist.
        m_FreeListBySize.erase(prevBlockIt->second.FreeListBySizeIt);
        m_FreeListByOffset.erase(prevBlockIt);
    }

    if (nextBlockIt != m_FreeListByOffset.end() &&
        offset + numDescriptors == nextBlockIt->first)
    {
        // The next block is exactly in front of the block that is to be freed.
        //
        // Offset               NextBlock.Offset 
        // |                    |
        // |<------Size-------->|<-----NextBlock.Size----->|

        // Increase the block size by the size of merging with the next block.
        numDescriptors += nextBlockIt->second.Size;

        m_FreeListBySize.erase(nextBlockIt->second.FreeListBySizeIt);
        m_FreeListByOffset.erase(nextBlockIt);
    }

    // Add the freed block to the freelist.
    AddNewBlock(offset, numDescriptors);

    // Add the number of free handles back to the heap.
    m_NumFreeHandles += numDescriptors;
}

void DescriptorAllocatorPage::ReleaseStaleDescriptors(uint64_t frameNumber)
{
    while ( !m_StaleDescriptors.empty() && m_StaleDescriptors.front().FrameNumber <= frameNumber )
    {
        auto& staleDescriptor = m_StaleDescriptors.front();
        auto allocatedDescriptorIt = m_AllocatedDescriptors.find(staleDescriptor.Offset);

        // This shouldn't happen (unless the wrong descriptor was freed from the wrong heap?)
        if (allocatedDescriptorIt == m_AllocatedDescriptors.end())
        {
            throw std::exception("Allocated descriptor heap is out of sync with stale descriptor queue.");
        }

        // The offset of the descriptor in the heap.
        auto offset = staleDescriptor.Offset;
        // The number of descriptors that were allocated.
        auto numDescriptors = allocatedDescriptorIt->second;

        FreeBlock(offset, numDescriptors);

        m_StaleDescriptors.pop();
        m_AllocatedDescriptors.erase(allocatedDescriptorIt);
    }
}

DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap)
    : m_HeapType(type)
    , m_NumDescriptorsPerHeap(numDescriptorsPerHeap)
    , m_NumFreeHandles(0)
{
    m_DescriptorSize = Application::Get().GetDescriptorHandleIncrementSize(m_HeapType);
}

DescriptorAllocator::~DescriptorAllocator()
{}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocator::Allocate(uint32_t numDescriptors)
{
    if (!m_CurrentHeap || m_NumFreeHandles < numDescriptors)
    {
        m_CurrentHeap = CreateHeap();
        m_CurrentHandle = m_CurrentHeap->GetCPUDescriptorHandleForHeapStart();
        m_NumFreeHandles = m_NumDescriptorsPerHeap;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE ret = m_CurrentHandle;
    m_CurrentHandle.Offset(numDescriptors, m_DescriptorSize);
    m_NumFreeHandles -= numDescriptors;

    return ret;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DescriptorAllocator::CreateHeap()
{
    auto device = Application::Get().GetDevice();

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = m_HeapType;
    heapDesc.NumDescriptors = m_NumDescriptorsPerHeap;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap)));
    m_DescriptorHeapPool.emplace_back(descriptorHeap);

    return descriptorHeap;
}