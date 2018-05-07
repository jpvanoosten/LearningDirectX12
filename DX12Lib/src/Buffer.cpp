#include <DX12LibPCH.h>

#include <Buffer.h>

Buffer::Buffer(const std::wstring& name)
    : Resource(name)
{}

Buffer::Buffer( const D3D12_RESOURCE_DESC& resDesc,
    size_t numElements, size_t elementSize,
    D3D12_RESOURCE_STATES initialState,
    const std::wstring& name )
    : Resource(resDesc, nullptr, initialState, name)
{
    CreateViews(numElements, elementSize);
}

void Buffer::CreateViews(size_t numElements, size_t elementSize)
{
    throw std::exception("Unimplemented function.");
}
