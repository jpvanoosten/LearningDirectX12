#include <DX12LibPCH.h>

#include <DynamicDescriptorHeap.h>

#include <Application.h>

#include <new> // For std::bad_alloc

DynamicDescriptorHeap::DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_COMMAND_LIST_TYPE commandListType, uint32_t numDescriptorsPerHeap)
    : m_CommandListType(commandListType)
    , m_DescriptorHeapType(heapType)
    , m_NumDescriptorsPerHeap(numDescriptorsPerHeap)
    , m_DescriptorTableBitMask(0)
    , m_StaleDescriptorTableBitMask(0)
    , m_CurrentCPUDescriptorHandle(D3D12_DEFAULT)
    , m_CurrentGPUDescriptorHandle(D3D12_DEFAULT)
    , m_NumFreeHandles(0)
{
    m_DescriptorSize = Application::Get().GetDescriptorHandleIncrementSize(heapType);

    // Allocate space for staging CPU visible descriptors.
    m_DescriptorHandleCache = std::make_unique<D3D12_CPU_DESCRIPTOR_HANDLE[]>(m_NumDescriptorsPerHeap);
}

DynamicDescriptorHeap::~DynamicDescriptorHeap()
{}

void DynamicDescriptorHeap::StageDescriptors(uint32_t rootParameterIndex, uint32_t offset, uint32_t numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors[])
{
    // Cannot stage more than the maximum number of descriptors per heap.
    // Cannot stage more than MaxDescriptorTables root parameters.
    if (numDescriptors > m_NumDescriptorsPerHeap || rootParameterIndex >= MaxDescriptorTables )
    {
        throw std::bad_alloc();
    }

    DescriptorTableCache& descriptorTableCache = m_DescriptorTableCache[rootParameterIndex];

    // Check that the number of descriptors to copy does not exceed the number
    // of descriptors expected in the descriptor table.
    if ( (offset + numDescriptors) > descriptorTableCache.NumDescriptors)
    {
        throw std::length_error("Number of descriptors to stage exceeds the size of the descriptor table.");
    }

    D3D12_CPU_DESCRIPTOR_HANDLE* dstDescriptor = (descriptorTableCache.BaseDescriptor + offset);
    for (uint32_t i = 0; i < numDescriptors; ++i)
    {
        dstDescriptor[i] = srcDescriptors[i];
    }

    // Set the root parameter index bit to make sure the descriptor table 
    // at that index is bound to the command list.
    m_StaleDescriptorTableBitMask |= (1 << rootParameterIndex);
}



Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::RequestDescriptorHeap()
{
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    if (!m_AvailableDescriptorHeaps.empty())
    {
        descriptorHeap = m_AvailableDescriptorHeaps.front();
        m_AvailableDescriptorHeaps.pop();
    }
    else
    {
        descriptorHeap = CreateDescriptorHeap();
        m_DescriptorHeapPool.push(descriptorHeap);
    }

    return descriptorHeap;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DynamicDescriptorHeap::CreateDescriptorHeap()
{
    auto device = Application::Get().GetDevice();

    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    descriptorHeapDesc.Type = m_DescriptorHeapType;
    descriptorHeapDesc.NumDescriptors = m_NumDescriptorsPerHeap;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    ThrowIfFailed(device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

void DynamicDescriptorHeap::Reset()
{
    m_AvailableDescriptorHeaps = m_DescriptorHeapPool;
    m_CurrentDescriptorHeap.Reset();
    m_CurrentCPUDescriptorHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
    m_CurrentGPUDescriptorHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D3D12_DEFAULT);
    m_NumFreeHandles = 0;
    m_DescriptorTableBitMask = 0;
    m_StaleDescriptorTableBitMask = 0;
}
