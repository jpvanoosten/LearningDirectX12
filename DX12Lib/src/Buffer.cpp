#include "DX12LibPCH.h"

#include <dx12lib/Buffer.h>

using namespace dx12lib;

Buffer::Buffer( std::shared_ptr<Device> device, const D3D12_RESOURCE_DESC& resDesc )
: Resource( device, resDesc )
{}

Buffer::Buffer( std::shared_ptr<Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> resource )
: Resource( device, resource )
{}
