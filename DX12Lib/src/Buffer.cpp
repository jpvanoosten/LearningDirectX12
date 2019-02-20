#include <DX12LibPCH.h>

#include <Buffer.h>

Buffer::Buffer(const std::wstring& name)
    : Resource(name)
{}

Buffer::Buffer( const D3D12_RESOURCE_DESC& resDesc,
    size_t numElements, size_t elementSize,
    const std::wstring& name )
    : Resource(resDesc, nullptr, name)
{}
