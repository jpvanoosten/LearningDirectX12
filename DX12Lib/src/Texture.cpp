#include <DX12LibPCH.h>

#include <Texture.h>

#include <Application.h>

Texture::Texture(const std::wstring& name)
    : Resource(name)
{
    m_ShaderResourceView = Application::Get().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// Maximum size of a texture in either dimension is 16,384. That means a maximum of 15 mips (0-14).
	m_UnorderedAccessViews = Application::Get().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 15);

	m_DescriptorHandleIncrementSize = Application::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

Texture::Texture(ID3D12Resource* resource, const std::wstring& name)
	: Resource(resource, name)
{
	m_ShaderResourceView = Application::Get().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// Maximum size of a texture in either dimension is 16,384. That means a maximum of 15 mips (0-14).
	m_UnorderedAccessViews = Application::Get().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 15);

	m_DescriptorHandleIncrementSize = Application::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}


Texture::~Texture()
{}

// Get a UAV description that matches the resource description.
D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc(const D3D12_RESOURCE_DESC& resDesc, UINT mipSlice, UINT firstArraySlice = 0, UINT planeSlice = 0 )
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = resDesc.Format;

	switch (resDesc.Dimension)
	{
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		if (resDesc.DepthOrArraySize > 1)
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
			uavDesc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize;
			uavDesc.Texture1DArray.FirstArraySlice = firstArraySlice;
			uavDesc.Texture1DArray.MipSlice = mipSlice;
		}
		else
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			uavDesc.Texture1D.MipSlice = mipSlice;
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (resDesc.DepthOrArraySize > 1)
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			uavDesc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize;
			uavDesc.Texture2DArray.FirstArraySlice = firstArraySlice;
			uavDesc.Texture2DArray.PlaneSlice = planeSlice;
			uavDesc.Texture2DArray.MipSlice = mipSlice;
		}
		else
		{
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			uavDesc.Texture2D.PlaneSlice = planeSlice;
			uavDesc.Texture2D.MipSlice = mipSlice;
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
		uavDesc.Texture3D.WSize = resDesc.DepthOrArraySize;
		uavDesc.Texture3D.FirstWSlice = firstArraySlice;
		uavDesc.Texture3D.MipSlice = mipSlice;
		break;
	default:
		throw std::exception("Invalid resource dimension.");
	}

	return uavDesc;
}

void Texture::CreateViews()
{
    if (m_d3d12Resource)
    {
        auto device = Application::Get().GetDevice();
        device->CreateShaderResourceView(m_d3d12Resource.Get(), nullptr, m_ShaderResourceView);

		auto desc = m_d3d12Resource->GetDesc();
		if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0 && IsFormatUAVCompatible(desc.Format))
		{
			for( UINT mip = 0; mip < desc.MipLevels; ++mip )
			{
				auto uavDesc = GetUAVDesc(desc, mip);
				device->CreateUnorderedAccessView(m_d3d12Resource.Get(), 
					nullptr, &uavDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(m_UnorderedAccessViews, mip * m_DescriptorHandleIncrementSize));
			}
		}
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetShaderResourceView() const
{
    return m_ShaderResourceView;
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetUnorderedAccessView(uint32_t subresource) const
{
	assert(subresource < 15);
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_UnorderedAccessViews, subresource * m_DescriptorHandleIncrementSize);
}

bool Texture::IsFormatUAVCompatible(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SINT:
		return true;
	default:
		return false;
	}
}

bool Texture::IsFormatSRGB(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return true;
	default:
		return false;
	}
}

bool Texture::IsFormatBGR(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return true;
	default:
		return false;
	}

}

DXGI_FORMAT Texture::GetTypelessFormat(DXGI_FORMAT format)
{
	DXGI_FORMAT typelessFormat = format;

	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		typelessFormat = DXGI_FORMAT_R32G32B32A32_TYPELESS;
		break;
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		typelessFormat = DXGI_FORMAT_R32G32B32_TYPELESS;
		break;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
		typelessFormat = DXGI_FORMAT_R16G16B16A16_TYPELESS;
		break;
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
		typelessFormat = DXGI_FORMAT_R32G32_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		typelessFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
		break;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
		typelessFormat = DXGI_FORMAT_R10G10B10A2_TYPELESS;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
		typelessFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS;
		break;
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
		typelessFormat = DXGI_FORMAT_R16G16_TYPELESS;
		break;
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
		typelessFormat = DXGI_FORMAT_R32_TYPELESS;
		break;

		// TODO: Thomas is being anoying.
	}
}
