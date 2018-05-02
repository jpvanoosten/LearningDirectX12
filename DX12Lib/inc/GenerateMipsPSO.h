/**
 * Pipeline state object for generating mip maps.
 */
#pragma once

#include <RootSignature.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>


struct alignas(16) GenerateMipsCB
{
	uint32_t SrcMipLevel;	// Texture level of source mip
	uint32_t NumMipLevels;	// Number of OutMips to write: [1-4]
	DirectX::XMFLOAT2 TexelSize;	// 1.0 / OutMip1.Dimensions
};

namespace GenerateMips
{
	enum
	{
		GenerateMipsCB,
		SrcMip,
		OutMip,
		NumRootParameters
	};
}

class GenerateMipsPSO
{
public:
	GenerateMipsPSO();

	const RootSignature& GetRootSignature() const
	{
		return m_RootSignature;
	}

	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState() const
	{
		return m_PipelineState;
	}

private:
	RootSignature m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
};