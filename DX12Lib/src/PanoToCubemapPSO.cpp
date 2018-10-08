#include <DX12LibPCH.h>

#include <PanoToCubemapPSO.h>

// Compiled shader
#include <PanoToCubemap_CS.h>

#include <Application.h>

#include <d3dx12.h>

PanoToCubemapPSO::PanoToCubemapPSO()
{
    auto device = Application::Get().GetDevice();

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    CD3DX12_DESCRIPTOR_RANGE1 srcMip(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
    CD3DX12_DESCRIPTOR_RANGE1 outMip(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

    CD3DX12_ROOT_PARAMETER1 rootParameters[PanoToCubemapRS::NumRootParameters];
    rootParameters[PanoToCubemapRS::PanoToCubemapCB].InitAsConstants(sizeof(PanoToCubemapCB) / 4, 0);
    rootParameters[PanoToCubemapRS::SrcTexture].InitAsDescriptorTable(1, &srcMip);
    rootParameters[PanoToCubemapRS::DstMips].InitAsDescriptorTable(1, &outMip);

    CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler(0,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_TEXTURE_ADDRESS_MODE_WRAP
    );

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(PanoToCubemapRS::NumRootParameters,
        rootParameters, 1, &linearRepeatSampler);

    m_RootSignature.SetRootSignatureDesc(rootSignatureDesc.Desc_1_1, featureData.HighestVersion);

    // Create the PSO for GenerateMips shader.
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_CS CS;
    } pipelineStateStream;

    pipelineStateStream.pRootSignature = m_RootSignature.GetRootSignature().Get();
    pipelineStateStream.CS = { g_PanoToCubemap_CS, sizeof(g_PanoToCubemap_CS) };

    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof(PipelineStateStream), &pipelineStateStream
    };

    ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

    // Create some default texture UAV's to pad any unused UAV's during mip map generation.
    m_DefaultUAV = Application::Get().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 5);
    UINT descriptorHandleIncrementSize = Application::Get().GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    for (UINT i = 0; i < 5; ++i)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        uavDesc.Texture2DArray.ArraySize = 6; // Cubemap.
        uavDesc.Texture2DArray.FirstArraySlice = 0;
        uavDesc.Texture2DArray.MipSlice = i;
        uavDesc.Texture2DArray.PlaneSlice = 0;

        device->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, m_DefaultUAV.GetDescriptorHandle(i));
    }
}