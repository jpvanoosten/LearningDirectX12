/**
 * This is an allocator for CPU visible descriptors.
 * CPU visible descriptors must be copied to a GPU visible descriptor heap before
 * being used in a shader. The DynamicDescriptorHeap class is used to upload
 * CPU visible descriptors to a GPU visible descriptor heap.
 *
 * Variable sized memory allocation strategy based on:
 * http://diligentgraphics.com/diligent-engine/architecture/d3d12/variable-size-memory-allocations-manager/
 * Date Accessed: May 9, 2018
 */
#pragma once

#include "DescriptorAllocation.h"

#include "d3dx12.h"

#include <cstdint>
#include <mutex>
#include <memory>
#include <set>
#include <vector>

class DescriptorAllocatorPage;

class DescriptorAllocator
{
public:
    DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 256);
    virtual ~DescriptorAllocator();

    /**
     * Allocate a number of contiguous descriptors from a CPU visible descriptor heap.
     * 
     * @param numDescriptors The number of contiguous descriptors to allocate. 
     * Cannot be more than the number of descriptors per descriptor heap.
     */
    DescriptorAllocation Allocate(uint32_t numDescriptors = 1);

    /**
     * When the frame has completed, the stale descriptors can be released.
     */
    void ReleaseStaleDescriptors( uint64_t frameNumber );

protected:

private:
    using DescriptorHeapPool = std::vector< std::shared_ptr<DescriptorAllocatorPage> >;

    // Create a new heap with a specific number of descriptors.
    std::shared_ptr<DescriptorAllocatorPage> CreateAllocatorPage();

    D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
    uint32_t m_NumDescriptorsPerHeap;

    DescriptorHeapPool m_HeapPool;
    // Indices of available heaps in the heap pool.
    std::set<size_t> m_AvailableHeaps;

    std::mutex m_AllocationMutex;
};