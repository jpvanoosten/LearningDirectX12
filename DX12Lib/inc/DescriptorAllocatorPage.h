#pragma  once;

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file DescriptorAllocatorPage.h
 *  @date October 22, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief A descriptor heap (page for the DescriptorAllocator class).
 *
 *  Variable sized memory allocation strategy based on:
 *  http://diligentgraphics.com/diligent-engine/architecture/d3d12/variable-size-memory-allocations-manager/
 *  Date Accessed: May 9, 2018
 */

#include "DescriptorAllocation.h"

#include <d3d12.h>

#include <wrl.h>

#include <map>
#include <memory>
#include <mutex>
#include <queue>

class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage>
{
public:
    DescriptorAllocatorPage( D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors );

    D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const;

    /**
    * Check to see if this descriptor page has a contiguous block of descriptors
    * large enough to satisfy the request.
    */
    bool HasSpace( uint32_t numDescriptors ) const;

    /**
    * Get the number of available handles in the heap.
    */
    uint32_t NumFreeHandles() const;

    /**
    * Allocate a number of descriptors from this descriptor heap.
    * If the allocation cannot be satisfied, then a NULL descriptor
    * is returned.
    */
    DescriptorAllocation Allocate( uint32_t numDescriptors );

    /**
    * Return a descriptor back to the heap.
    * @param frameNumber Stale descriptors are not freed directly, but put
    * on a stale allocations queue. Stale allocations are returned to the heap
    * using the DescriptorAllocatorPage::ReleaseStaleAllocations method.
    */
    void Free( DescriptorAllocation&& descriptorHandle, uint64_t frameNumber );

    /**
    * Returned the stale descriptors back to the descriptor heap.
    */
    void ReleaseStaleDescriptors( uint64_t frameNumber );

protected:

    // Compute the offset of the descriptor handle from the start of the heap.
    uint32_t ComputeOffset( D3D12_CPU_DESCRIPTOR_HANDLE handle );

    // Adds a new block to the free list.
    void AddNewBlock( uint32_t offset, uint32_t numDescriptors );

    // Free a block of descriptors.
    // This will also merge free blocks in the free list to form larger blocks
    // that can be reused.
    void FreeBlock( uint32_t offset, uint32_t numDescriptors );

private:
    // The offset (in descriptors) within the descriptor heap.
    using OffsetType = uint32_t;
    // The number of descriptors that are available.
    using SizeType = uint32_t;

    struct FreeBlockInfo;
    // A map that lists the free blocks by the offset within the descriptor heap.
    using FreeListByOffset = std::map<OffsetType, FreeBlockInfo>;

    // A map that lists the free blocks by size.
    // Needs to be a multimap since multiple blocks can have the same size.
    using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;

    struct FreeBlockInfo
    {
        FreeBlockInfo( SizeType size )
            : Size( size )
        {}

        SizeType Size;
        FreeListBySize::iterator FreeListBySizeIt;
    };

    struct StaleDescriptorInfo
    {
        StaleDescriptorInfo( OffsetType offset, SizeType size, uint64_t frame )
            : Offset( offset )
            , Size( size )
            , FrameNumber( frame )
        {}

        // The offset within the descriptor heap.
        OffsetType Offset;
        // The number of descriptors
        SizeType Size;
        // The frame number that the descriptor was freed.
        uint64_t FrameNumber;
    };

    // Stale descriptors are queued for release until the frame that they were freed
    // has completed.
    using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;

    FreeListByOffset m_FreeListByOffset;
    FreeListBySize m_FreeListBySize;
    StaleDescriptorQueue m_StaleDescriptors;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_d3d12DescriptorHeap;
    D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_BaseDescriptor;
    uint32_t m_DescriptorHandleIncrementSize;
    uint32_t m_NumDescriptorsInHeap;
    uint32_t m_NumFreeHandles;

    std::mutex m_AllocationMutex;
};