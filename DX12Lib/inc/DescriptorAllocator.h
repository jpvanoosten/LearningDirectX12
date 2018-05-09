/**
 * This is a linear allocator for CPU visible descriptors.
 * CPU visible descriptors must be copied to a GPU visible descriptor heap before
 * being used in a shader. The DynamicDescriptorHeap class is used to upload
 * CPU visible descriptors to a GPU visible descriptor heap.
 */
#pragma once

#include "d3dx12.h"

#include <wrl.h>

#include <cstdint>
#include <map>
#include <queue>
#include <vector>

/**
 * Variable sized memory allocation strategy based on:
 * http://diligentgraphics.com/diligent-engine/architecture/d3d12/variable-size-memory-allocations-manager/
 * Date Accessed: May 9, 2018
 */
class DescriptorAllocatorPage
{
public:
    DescriptorAllocatorPage( D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors );

    /**
     * Allocate a number of descriptors from this descriptor heap.
     * If the allocation cannot be satisfied, then a NULL descriptor
     * is returned.
     */
    D3D12_CPU_DESCRIPTOR_HANDLE Allocate( uint32_t numDescriptors );

    /**
     * Return a descriptor back to the heap.
     * @param frameNumber Stale descriptors are not freed directly, but put 
     * on a stale allocations queue. Stale allocations are returned to the heap
     * using the DescriptorAllocatorPage::ReleaseStaleAllocations method.
     */
    void Free( D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, uint64_t frameNumber );

    /**
     * Returned the stale descriptors back to the descriptor heap.
     */
    void ReleaseStaleDescriptors( uint64_t frameNumber );

protected:
    // Adds a new block to the free list.
    void AddNewBlock( uint32_t offset, uint32_t numDescriptors );
private:
    // The offset (in descriptors) within the descriptor heap.
    using OffsetType = uint32_t;
    // The number of descriptors that are available.
    using SizeType = uint32_t;

    struct FreeBlock;
    // A map that lists the free blocks by the offset within the descriptor heap.
    using FreeListByOffset = std::map<OffsetType, FreeBlock>;

    // A map that lists the free blocks by size.
    // Needs to be a multimap since multiple blocks can have the same size.
    using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;

    // Map the allocated descriptor to the number of descriptors that were
    // allocated. This is required for returning the descriptors back to the heap.
    using AllocatedDesciptorMap = std::map<D3D12_CPU_DESCRIPTOR_HANDLE, SizeType>;

    struct StaleDescriptor;
    // Stale descriptors are queued for release until the frame that they were freed
    // has completed.
    using StaleDescriptorQueue = std::queue< StaleDescriptor>;

    struct FreeBlock
    {
        FreeBlock( SizeType size )
            : Size(size)
        {}

        SizeType Size;
        FreeListBySize::iterator FreeListBySizeIt;
    };

    struct StaleDescriptor
    {
        // The handle to the stale descriptor.
        D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
        // The frame number that the descriptor was freed.
        uint64_t FrameNumber;
    };

    FreeListByOffset m_FreeListByOffset;
    FreeListBySize m_FreeListBySize;
    AllocatedDesciptorMap m_AllocatedDescriptors;
    StaleDescriptorQueue m_StaleDescriptors;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_d3d12DescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_BaseDescriptor;
    uint32_t m_DescriptorHandleIncrementSize;
    uint32_t m_NumDescriptorsInHeap;
    uint32_t m_NumFreeHandles;
};

class DescriptorAllocator
{
public:
    DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 256);
    virtual ~DescriptorAllocator();

    /**
     * Allocate a number of contiguous descriptors from a CPU visible descriptor heap.
     * Must be less than the number of descriptors per descriptor heap (specified
     * in the constructor for the DescriptorAllocator. Default is 256 descriptors 
     * per heap.
     * 
     * @param numDescriptors The number of contiguous descriptors to allocate. 
     * Cannot be more than the number of descriptors per descriptor heap.
     */
    D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t numDescriptors = 1);

protected:

private:
    using DescriptorHeapPool = std::vector< Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> >;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateHeap();

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CurrentHeap;
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_CurrentHandle;
    D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;

    DescriptorHeapPool m_DescriptorHeapPool;
    uint32_t m_NumDescriptorsPerHeap;
    uint32_t m_DescriptorSize;
    uint32_t m_NumFreeHandles;
};