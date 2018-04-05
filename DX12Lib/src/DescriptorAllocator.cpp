#include <DX12LibPCH.h>

#include <DescriptorAllocator.h>

#include <Application.h>

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