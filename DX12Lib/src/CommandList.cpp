#include <DX12LibPCH.h>
#include <CommandList.h>

CommandList::CommandList(Microsoft::WRL::ComPtr<ID3D12Device2> device, 
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator,
    D3D12_COMMAND_LIST_TYPE type)
    : m_d3d12Device( device )
    , m_d3d12CommandAllocator(allocator)
    , m_Type(type)
    , m_IsClosed(false)
{
    ThrowIfFailed(device->CreateCommandList(0, m_Type, m_d3d12CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_d3d12CommandList)));
}

CommandList::~CommandList()
{

}

void CommandList::Close()
{
    if ( !m_IsClosed )
    {
        ThrowIfFailed(m_d3d12CommandList->Close());
        m_IsClosed = true;
    }
}

void CommandList::Reset(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator)
{
    m_d3d12CommandAllocator = allocator;
    ThrowIfFailed(m_d3d12CommandList->Reset(m_d3d12CommandAllocator.Get(), nullptr));
    m_IsClosed = false;
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandList::GetCommandAllocator() const
{
    return m_d3d12CommandAllocator;
}

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CommandList::GetCommandList() const
{
    return m_d3d12CommandList;
}
