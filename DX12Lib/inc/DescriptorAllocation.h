/**
 * A single page for the descriptor allocator.
 *
 * Variable sized memory allocation strategy based on:
 * http://diligentgraphics.com/diligent-engine/architecture/d3d12/variable-size-memory-allocations-manager/
 * Date Accessed: May 9, 2018
 */
#pragma once

#include <d3d12.h>

class DescriptorAllocatorPage;

class DescriptorAllocation
{
public:
    // Creates a NULL descriptor.
    DescriptorAllocation();

    DescriptorAllocation( D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page );

    // Copies are not allowed.
    DescriptorAllocation( const DescriptorAllocation& ) = delete;

    // Moves allowed.
    DescriptorAllocation( DescriptorAllocation&& allocation ) = default;

    // The destructor will automatically free the allocation.
    ~DescriptorAllocation();

    // Assignment is not allowed.
    DescriptorAllocation& operator=( const DescriptorAllocation& ) = delete;

    // Move assignment is allowed.
    DescriptorAllocation& operator=( DescriptorAllocation&& other ) = default;

    // Check if this a valid descriptor.
    bool IsNull() const;

    // Get a descriptor at a particular offset in the allocation.
    D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle( uint32_t offset = 0 ) const;

    // Get the number of (consecutive) handles for this allocation.
    uint32_t GetNumHandles() const;

    // Get the heap that this allocation came from.
    // (For internal use only).
    std::shared_ptr<DescriptorAllocatorPage> GetDescriptorAllocatorPage() const;

private:
    // The base descriptor.
    D3D12_CPU_DESCRIPTOR_HANDLE m_Descriptor;
    // The number of descriptors in this allocation.
    uint32_t m_NumHandles;
    // The offset to the next descriptor.
    uint32_t m_DescriptorSize;

    // A pointer back to the original page where this allocation came from.
    std::shared_ptr<DescriptorAllocatorPage> m_Page;
};