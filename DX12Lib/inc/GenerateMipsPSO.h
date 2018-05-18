/**
 * Pipeline state object for generating mip maps.
 */
#pragma once

#include "RootSignature.h"
#include "DescriptorAllocation.h"

#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl.h>


struct alignas( 16 ) GenerateMipsCB
{
    uint32_t SrcMipLevel;	// Texture level of source mip
    uint32_t NumMipLevels;	// Number of OutMips to write: [1-4]
    uint32_t SrcDimension;  // Width and height of the source texture are even or odd.
    uint32_t Padding;       // Pad to 16 byte alignment.
    DirectX::XMFLOAT2 TexelSize;	// 1.0 / OutMip1.Dimensions
};

// I don't use scoped enums to avoid the explicit cast that is required to 
// treat these as root indices.
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

    D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultUAV() const
    {
        return m_DefaultUAV.GetDescriptorHandle();
    }

private:
    RootSignature m_RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
    // Default (no resource) UAV's to padd the unused UAV descriptors.
    // If generating less than 4 mip map levels, the unused mip maps
    // need to be padded with default UAVs (to keep the DX runtime happy).
    DescriptorAllocation m_DefaultUAV;
};