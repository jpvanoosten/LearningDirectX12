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
}

void DescriptorAllocatorPage::AddNewBlock(uint32_t offset, uint32_t numDescriptors)
{
    auto offsetIt = m_FreeListByOffset.emplace(offset, numDescriptors);
    auto sizeIt = m_FreeListBySize.emplace(numDescriptors, offsetIt.first);
    offsetIt.first->second.FreeListBySizeIt = sizeIt;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocatorPage::Allocate(uint32_t numDescriptors)
{

}

void DescriptorAllocatorPage::Free(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, uint64_t frameNumber)
{

}

void DescriptorAllocatorPage::ReleaseStaleDescriptors(uint64_t frameNumber)
{

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