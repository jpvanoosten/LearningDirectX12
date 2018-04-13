/**
 * The RootSignature class encapsulates both the ID3D12RootSignature and the
 * D3D12_ROOT_SIGNATURE_DESC used to create it. This provides the functionality 
 * necessary for the DynamicDescriptorHeap to determine the layout of the root 
 * signature at runtime.
 */
#pragma once

#include "d3dx12.h"

#include <wrl.h>

#include <vector>

class RootSignature
{
public:
    RootSignature(
        const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion
    );

    virtual ~RootSignature();

    Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const
    {
        return m_RootSignature;
    }

    const D3D12_ROOT_SIGNATURE_DESC1& GetRootSignatureDesc() const
    {
        return m_RootSignatureDesc;
    }

protected:

private:
    D3D12_ROOT_SIGNATURE_DESC1 m_RootSignatureDesc;
    D3D_ROOT_SIGNATURE_VERSION m_RootSignatureVersion;
    
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
};