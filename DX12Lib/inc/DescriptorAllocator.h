/**
 * This is a linear allocator for CPU visible descriptors.
 * CPU visible descriptors must be copied to a GPU visible descriptor heap before
 * being used in a shader. The DynamicDescriptorHeap class is used to upload
 * CPU visible descriptors to a GPU visible descriptor heap.
 */

#include "d3dx12.h"

#include <wrl.h>

#include <cstdint>
#include <vector>

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