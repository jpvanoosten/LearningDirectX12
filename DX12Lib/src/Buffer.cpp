#include <DX12LibPCH.h>

#include <Buffer.h>

Buffer::Buffer(const std::wstring& name)
    : Resource(name)
{}

Buffer::Buffer(Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource, D3D12_RESOURCE_STATES initialResourceState)
    : Resource(d3d12Resource, initialResourceState)
{}
