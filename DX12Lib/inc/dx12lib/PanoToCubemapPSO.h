#pragma once

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file PanoToCubemapPSO.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Pipeline State Object and root signature for the conversion of a 
 *  panoramic texture to a cubemap texture.
 */


#include "RootSignature.h"
#include "DescriptorAllocation.h"

#include <cstdint>

// Struct used in the PanoToCubemap_CS compute shader.
struct PanoToCubemapCB
{
    // Size of the cubemap face in pixels at the current mipmap level.
    uint32_t CubemapSize;
    // The first mip level to generate.
    uint32_t FirstMip;
    // The number of mips to generate.
    uint32_t NumMips;
};

// I don't use scoped enums to avoid the explicit cast that is required to 
// treat these as root indices into the root signature.
namespace PanoToCubemapRS
{
    enum
    {
        PanoToCubemapCB,
        SrcTexture,
        DstMips,
        NumRootParameters
    };
}

class PanoToCubemapPSO
{
public:
    PanoToCubemapPSO();

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
    // Default (no resource) UAV's to pad the unused UAV descriptors.
    // If generating less than 5 mip map levels, the unused mip maps
    // need to be padded with default UAVs (to keep the DX12 runtime happy).
    DescriptorAllocation m_DefaultUAV;
};