#include <DX12LibPCH.h>

#include <CommandList.h>

#include <Application.h>
#include <DynamicDescriptorHeap.h>
#include <Resource.h>
#include <ResourceStateTracker.h>
#include <UploadBuffer.h>

CommandList::CommandList(D3D12_COMMAND_LIST_TYPE type)
    : m_CommandListType(type)
{
    auto device = Application::Get().GetDevice();

    ThrowIfFailed(device->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&m_CommandAllocator)));

    ThrowIfFailed(device->CreateCommandList(0, m_CommandListType, m_CommandAllocator.Get(),
        nullptr, IID_PPV_ARGS(&m_GraphicsCommandList)));

    m_UploadBuffer = std::make_unique<UploadBuffer>();

    m_ResourceStateTracker = std::make_unique<ResourceStateTracker>();

    for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i)
    {
        m_DynamicDescriptorHeap[i] = std::make_unique<DynamicDescriptorHeap>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
    }
}

CommandList::~CommandList()
{}