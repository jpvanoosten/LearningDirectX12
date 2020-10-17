#include "DX12LibPCH.h"

#include <dx12lib/ConstantBuffer.h>
#include <dx12lib/Device.h>
#include <dx12lib/d3dx12.h>

using namespace dx12lib;

ConstantBuffer::ConstantBuffer( Device& device, const D3D12_RESOURCE_DESC& resourceDesc )
: Buffer( device, resourceDesc )
, m_SizeInBytes( resourceDesc.Width )
{}

ConstantBuffer::~ConstantBuffer() {}
