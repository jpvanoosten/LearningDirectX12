#include "DX12LibPCH.h"

#include <dx12lib/CommandList.h>

#include <dx12lib/ByteAddressBuffer.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/ConstantBuffer.h>
#include <dx12lib/ConstantBufferView.h>
#include <dx12lib/Device.h>
#include <dx12lib/DynamicDescriptorHeap.h>
#include <dx12lib/GenerateMipsPSO.h>
#include <dx12lib/IndexBuffer.h>
#include <dx12lib/Material.h>
#include <dx12lib/PanoToCubemapPSO.h>
#include <dx12lib/PipelineStateObject.h>
#include <dx12lib/RenderTarget.h>
#include <dx12lib/Resource.h>
#include <dx12lib/ResourceStateTracker.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/Scene.h>
#include <dx12lib/SceneNode.h>
#include <dx12lib/ShaderResourceView.h>
#include <dx12lib/StructuredBuffer.h>
#include <dx12lib/Texture.h>
#include <dx12lib/UnorderedAccessView.h>
#include <dx12lib/UploadBuffer.h>
#include <dx12lib/VertexBuffer.h>

using namespace dx12lib;

// Adapter for std::make_unique
class MakeUploadBuffer : public UploadBuffer
{
public:
    MakeUploadBuffer( Device& device, size_t pageSize = _2MB )
    : UploadBuffer( device, pageSize )
    {}

    virtual ~MakeUploadBuffer() {}
};

std::map<std::wstring, ID3D12Resource*> CommandList::ms_TextureCache;
std::mutex                              CommandList::ms_TextureCacheMutex;

CommandList::CommandList( Device& device, D3D12_COMMAND_LIST_TYPE type )
: m_Device( device )
, m_d3d12CommandListType( type )
{
    auto d3d12Device = m_Device.GetD3D12Device();

    ThrowIfFailed(
        d3d12Device->CreateCommandAllocator( m_d3d12CommandListType, IID_PPV_ARGS( &m_d3d12CommandAllocator ) ) );

    ThrowIfFailed( d3d12Device->CreateCommandList( 0, m_d3d12CommandListType, m_d3d12CommandAllocator.Get(), nullptr,
                                                   IID_PPV_ARGS( &m_d3d12CommandList ) ) );

    m_UploadBuffer = std::make_unique<MakeUploadBuffer>( device );

    m_ResourceStateTracker = std::make_unique<ResourceStateTracker>();

    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    {
        m_DynamicDescriptorHeap[i] =
            std::make_unique<DynamicDescriptorHeap>( device, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>( i ) );
        m_DescriptorHeaps[i] = nullptr;
    }
}

CommandList::~CommandList() {}

void CommandList::TransitionBarrier( Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES stateAfter,
                                     UINT subresource, bool flushBarriers )
{
    if ( resource )
    {
        // The "before" state is not important. It will be resolved by the resource state tracker.
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition( resource.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter,
                                                             subresource );
        m_ResourceStateTracker->ResourceBarrier( barrier );
    }

    if ( flushBarriers )
    {
        FlushResourceBarriers();
    }
}

void CommandList::TransitionBarrier( const std::shared_ptr<Resource>& resource, D3D12_RESOURCE_STATES stateAfter,
                                     UINT subresource, bool flushBarriers )
{
    if ( resource )
    {
        TransitionBarrier( resource->GetD3D12Resource(), stateAfter, subresource, flushBarriers );
    }
}

void CommandList::UAVBarrier( Microsoft::WRL::ComPtr<ID3D12Resource> resource, bool flushBarriers )
{
    auto barrier = CD3DX12_RESOURCE_BARRIER::UAV( resource.Get() );

    m_ResourceStateTracker->ResourceBarrier( barrier );

    if ( flushBarriers )
    {
        FlushResourceBarriers();
    }
}

void CommandList::UAVBarrier( const std::shared_ptr<Resource>& resource, bool flushBarriers )
{
    auto d3d12Resource = resource ? resource->GetD3D12Resource() : nullptr;
    UAVBarrier( d3d12Resource, flushBarriers );
}

void CommandList::AliasingBarrier( Microsoft::WRL::ComPtr<ID3D12Resource> beforeResource,
                                   Microsoft::WRL::ComPtr<ID3D12Resource> afterResource, bool flushBarriers )
{
    auto barrier = CD3DX12_RESOURCE_BARRIER::Aliasing( beforeResource.Get(), afterResource.Get() );

    m_ResourceStateTracker->ResourceBarrier( barrier );

    if ( flushBarriers )
    {
        FlushResourceBarriers();
    }
}

void CommandList::AliasingBarrier( const std::shared_ptr<Resource>& beforeResource,
                                   const std::shared_ptr<Resource>& afterResource, bool flushBarriers )
{
    auto d3d12BeforeResource = beforeResource ? beforeResource->GetD3D12Resource() : nullptr;
    auto d3d12AfterResource  = afterResource ? afterResource->GetD3D12Resource() : nullptr;

    AliasingBarrier( d3d12BeforeResource, d3d12AfterResource, flushBarriers );
}

void CommandList::FlushResourceBarriers()
{
    m_ResourceStateTracker->FlushResourceBarriers( shared_from_this() );
}

void CommandList::CopyResource( Microsoft::WRL::ComPtr<ID3D12Resource> dstRes,
                                Microsoft::WRL::ComPtr<ID3D12Resource> srcRes )
{
    assert( dstRes );
    assert( srcRes );

    TransitionBarrier( dstRes, D3D12_RESOURCE_STATE_COPY_DEST );
    TransitionBarrier( srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE );

    FlushResourceBarriers();

    m_d3d12CommandList->CopyResource( dstRes.Get(), srcRes.Get() );

    TrackResource( dstRes );
    TrackResource( srcRes );
}

void CommandList::CopyResource( const std::shared_ptr<Resource>& dstRes, const std::shared_ptr<Resource>& srcRes )
{
    assert( dstRes && srcRes );

    CopyResource( dstRes->GetD3D12Resource(), srcRes->GetD3D12Resource() );
}

void CommandList::ResolveSubresource( const std::shared_ptr<Resource>& dstRes, const std::shared_ptr<Resource>& srcRes,
                                      uint32_t dstSubresource, uint32_t srcSubresource )
{
    assert( dstRes && srcRes );

    TransitionBarrier( dstRes, D3D12_RESOURCE_STATE_RESOLVE_DEST, dstSubresource );
    TransitionBarrier( srcRes, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, srcSubresource );

    FlushResourceBarriers();

    m_d3d12CommandList->ResolveSubresource( dstRes->GetD3D12Resource().Get(), dstSubresource,
                                            srcRes->GetD3D12Resource().Get(), srcSubresource,
                                            dstRes->GetD3D12ResourceDesc().Format );

    TrackResource( srcRes );
    TrackResource( dstRes );
}

ComPtr<ID3D12Resource> CommandList::CopyBuffer( size_t bufferSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags )
{
    ComPtr<ID3D12Resource> d3d12Resource;
    if ( bufferSize == 0 )
    {
        // This will result in a NULL resource (which may be desired to define a default null resource).
    }
    else
    {
        auto d3d12Device = m_Device.GetD3D12Device();

        ThrowIfFailed( d3d12Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( bufferSize, flags ), D3D12_RESOURCE_STATE_COMMON, nullptr,
            IID_PPV_ARGS( &d3d12Resource ) ) );

        // Add the resource to the global resource state tracker.
        ResourceStateTracker::AddGlobalResourceState( d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON );

        if ( bufferData != nullptr )
        {
            // Create an upload resource to use as an intermediate buffer to copy the buffer resource
            ComPtr<ID3D12Resource> uploadResource;
            ThrowIfFailed( d3d12Device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ), D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer( bufferSize ), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS( &uploadResource ) ) );

            D3D12_SUBRESOURCE_DATA subresourceData = {};
            subresourceData.pData                  = bufferData;
            subresourceData.RowPitch               = bufferSize;
            subresourceData.SlicePitch             = subresourceData.RowPitch;

            m_ResourceStateTracker->TransitionResource( d3d12Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST );
            FlushResourceBarriers();

            UpdateSubresources( m_d3d12CommandList.Get(), d3d12Resource.Get(), uploadResource.Get(), 0, 0, 1,
                                &subresourceData );

            // Add references to resources so they stay in scope until the command list is reset.
            TrackResource( uploadResource );
        }
        TrackResource( d3d12Resource );
    }

    return d3d12Resource;
}

std::shared_ptr<VertexBuffer> CommandList::CopyVertexBuffer( size_t numVertices, size_t vertexStride,
                                                             const void* vertexBufferData )
{
    auto                          d3d12Resource = CopyBuffer( numVertices * vertexStride, vertexBufferData );
    std::shared_ptr<VertexBuffer> vertexBuffer =
        m_Device.CreateVertexBuffer( d3d12Resource, numVertices, vertexStride );

    return vertexBuffer;
}

std::shared_ptr<IndexBuffer> CommandList::CopyIndexBuffer( size_t numIndicies, DXGI_FORMAT indexFormat,
                                                           const void* indexBufferData )
{
    size_t elementSize = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;

    auto d3d12Resource = CopyBuffer( numIndicies * elementSize, indexBufferData );

    std::shared_ptr<IndexBuffer> indexBuffer = m_Device.CreateIndexBuffer( d3d12Resource, numIndicies, indexFormat );

    return indexBuffer;
}

std::shared_ptr<ConstantBuffer> CommandList::CopyConstantBuffer( size_t bufferSize, const void* bufferData )
{
    auto d3d12Resource = CopyBuffer( bufferSize, bufferData );

    std::shared_ptr<ConstantBuffer> constantBuffer = m_Device.CreateConstantBuffer( d3d12Resource );

    return constantBuffer;
}

std::shared_ptr<ByteAddressBuffer> CommandList::CopyByteAddressBuffer( size_t bufferSize, const void* bufferData )
{
    auto d3d12Resource = CopyBuffer( bufferSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS );

    std::shared_ptr<ByteAddressBuffer> byteAddressBuffer = m_Device.CreateByteAddressBuffer( d3d12Resource );

    return byteAddressBuffer;
}

std::shared_ptr<StructuredBuffer> CommandList::CopyStructuredBuffer( size_t numElements, size_t elementSize,
                                                                     const void* bufferData )
{
    auto d3d12Resource =
        CopyBuffer( numElements * elementSize, bufferData, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS );

    std::shared_ptr<StructuredBuffer> structuredBuffer =
        m_Device.CreateStructuredBuffer( d3d12Resource, numElements, elementSize );

    return structuredBuffer;
}

void CommandList::SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY primitiveTopology )
{
    m_d3d12CommandList->IASetPrimitiveTopology( primitiveTopology );
}

std::shared_ptr<Texture> CommandList::LoadTextureFromFile( const std::wstring& fileName, TextureUsage textureUsage )
{
    std::shared_ptr<Texture> texture;
    fs::path                 filePath( fileName );
    if ( !fs::exists( filePath ) )
    {
        throw std::exception( "File not found." );
    }

    std::lock_guard<std::mutex> lock( ms_TextureCacheMutex );
    auto                        iter = ms_TextureCache.find( fileName );
    if ( iter != ms_TextureCache.end() )
    {
        texture = m_Device.CreateTexture( iter->second, textureUsage );
    }
    else
    {
        TexMetadata  metadata;
        ScratchImage scratchImage;

        if ( filePath.extension() == ".dds" )
        {
            ThrowIfFailed( LoadFromDDSFile( fileName.c_str(), DDS_FLAGS_FORCE_RGB, &metadata, scratchImage ) );
        }
        else if ( filePath.extension() == ".hdr" )
        {
            ThrowIfFailed( LoadFromHDRFile( fileName.c_str(), &metadata, scratchImage ) );
        }
        else if ( filePath.extension() == ".tga" )
        {
            ThrowIfFailed( LoadFromTGAFile( fileName.c_str(), &metadata, scratchImage ) );
        }
        else
        {
            ThrowIfFailed( LoadFromWICFile( fileName.c_str(), WIC_FLAGS_FORCE_RGB, &metadata, scratchImage ) );
        }

        // Force albedo textures to use sRGB
        if ( textureUsage == TextureUsage::Albedo )
        {
            metadata.format = MakeSRGB( metadata.format );
        }

        D3D12_RESOURCE_DESC textureDesc = {};
        switch ( metadata.dimension )
        {
        case TEX_DIMENSION_TEXTURE1D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex1D( metadata.format, static_cast<UINT64>( metadata.width ),
                                                        static_cast<UINT16>( metadata.arraySize ) );
            break;
        case TEX_DIMENSION_TEXTURE2D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex2D( metadata.format, static_cast<UINT64>( metadata.width ),
                                                        static_cast<UINT>( metadata.height ),
                                                        static_cast<UINT16>( metadata.arraySize ) );
            break;
        case TEX_DIMENSION_TEXTURE3D:
            textureDesc = CD3DX12_RESOURCE_DESC::Tex3D( metadata.format, static_cast<UINT64>( metadata.width ),
                                                        static_cast<UINT>( metadata.height ),
                                                        static_cast<UINT16>( metadata.depth ) );
            break;
        default:
            throw std::exception( "Invalid texture dimension." );
            break;
        }

        auto                                   d3d12Device = m_Device.GetD3D12Device();
        Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;

        ThrowIfFailed( d3d12Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ), D3D12_HEAP_FLAG_NONE, &textureDesc,
            D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS( &textureResource ) ) );

        texture = m_Device.CreateTexture( textureResource, textureUsage );
        texture->SetName( fileName );

        // Update the global state tracker.
        ResourceStateTracker::AddGlobalResourceState( textureResource.Get(), D3D12_RESOURCE_STATE_COMMON );

        std::vector<D3D12_SUBRESOURCE_DATA> subresources( scratchImage.GetImageCount() );
        const Image*                        pImages = scratchImage.GetImages();
        for ( int i = 0; i < scratchImage.GetImageCount(); ++i )
        {
            auto& subresource      = subresources[i];
            subresource.RowPitch   = pImages[i].rowPitch;
            subresource.SlicePitch = pImages[i].slicePitch;
            subresource.pData      = pImages[i].pixels;
        }

        CopyTextureSubresource( texture, 0, static_cast<uint32_t>( subresources.size() ), subresources.data() );

        if ( subresources.size() < textureResource->GetDesc().MipLevels )
        {
            GenerateMips( texture );
        }

        // Add the texture resource to the texture cache.
        ms_TextureCache[fileName] = textureResource.Get();
    }

    return texture;
}

void CommandList::GenerateMips( const std::shared_ptr<Texture>& texture )
{
    if ( !texture )
        return;

    auto d3d12Device = m_Device.GetD3D12Device();

    if ( m_d3d12CommandListType == D3D12_COMMAND_LIST_TYPE_COPY )
    {
        if ( !m_ComputeCommandList )
        {
            m_ComputeCommandList = m_Device.GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COMPUTE ).GetCommandList();
        }
        m_ComputeCommandList->GenerateMips( texture );
        return;
    }

    auto d3d12Resource = texture->GetD3D12Resource();

    // If the texture doesn't have a valid resource? Do nothing...
    if ( !d3d12Resource )
        return;
    auto resourceDesc = d3d12Resource->GetDesc();

    // If the texture only has a single mip level (level 0)
    // do nothing.
    if ( resourceDesc.MipLevels == 1 )
        return;
    // Currently, only non-multi-sampled 2D textures are supported.
    if ( resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || resourceDesc.DepthOrArraySize != 1 ||
         resourceDesc.SampleDesc.Count > 1 )
    {
        throw std::exception( "GenerateMips is only supported for non-multi-sampled 2D Textures." );
    }

    ComPtr<ID3D12Resource> uavResource = d3d12Resource;
    // Create an alias of the original resource.
    // This is done to perform a GPU copy of resources with different formats.
    // BGR -> RGB texture copies will fail GPU validation unless performed
    // through an alias of the BRG resource in a placed heap.
    ComPtr<ID3D12Resource> aliasResource;

    // If the passed-in resource does not allow for UAV access
    // then create a staging resource that is used to generate
    // the mipmap chain.
    if ( !texture->CheckUAVSupport() || ( resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ) == 0 )
    {
        // Describe an alias resource that is used to copy the original texture.
        auto aliasDesc = resourceDesc;
        // Placed resources can't be render targets or depth-stencil views.
        aliasDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        aliasDesc.Flags &= ~( D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL );

        // Describe a UAV compatible resource that is used to perform
        // mipmapping of the original texture.
        auto uavDesc   = aliasDesc;  // The flags for the UAV description must match that of the alias description.
        uavDesc.Format = Texture::GetUAVCompatableFormat( resourceDesc.Format );

        D3D12_RESOURCE_DESC resourceDescs[] = { aliasDesc, uavDesc };

        // Create a heap that is large enough to store a copy of the original resource.
        auto allocationInfo = d3d12Device->GetResourceAllocationInfo( 0, _countof( resourceDescs ), resourceDescs );

        D3D12_HEAP_DESC heapDesc                 = {};
        heapDesc.SizeInBytes                     = allocationInfo.SizeInBytes;
        heapDesc.Alignment                       = allocationInfo.Alignment;
        heapDesc.Flags                           = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
        heapDesc.Properties.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapDesc.Properties.Type                 = D3D12_HEAP_TYPE_DEFAULT;

        ComPtr<ID3D12Heap> heap;
        ThrowIfFailed( d3d12Device->CreateHeap( &heapDesc, IID_PPV_ARGS( &heap ) ) );

        // Make sure the heap does not go out of scope until the command list
        // is finished executing on the command queue.
        TrackResource( heap );

        // Create a placed resource that matches the description of the
        // original resource. This resource is used to copy the original
        // texture to the UAV compatible resource.
        ThrowIfFailed( d3d12Device->CreatePlacedResource( heap.Get(), 0, &aliasDesc, D3D12_RESOURCE_STATE_COMMON,
                                                          nullptr, IID_PPV_ARGS( &aliasResource ) ) );

        ResourceStateTracker::AddGlobalResourceState( aliasResource.Get(), D3D12_RESOURCE_STATE_COMMON );
        // Ensure the scope of the alias resource.
        TrackResource( aliasResource );

        // Create a UAV compatible resource in the same heap as the alias
        // resource.
        ThrowIfFailed( d3d12Device->CreatePlacedResource( heap.Get(), 0, &uavDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                          IID_PPV_ARGS( &uavResource ) ) );

        ResourceStateTracker::AddGlobalResourceState( uavResource.Get(), D3D12_RESOURCE_STATE_COMMON );

        // Ensure the scope of the UAV compatible resource.
        TrackResource( uavResource );

        // Add an aliasing barrier for the alias resource.
        AliasingBarrier( nullptr, aliasResource );

        // Copy the original resource to the alias resource.
        // This ensures GPU validation.
        CopyResource( aliasResource, d3d12Resource );

        // Add an aliasing barrier for the UAV compatible resource.
        AliasingBarrier( aliasResource, uavResource );
    }

    // Generate mips with the UAV compatible resource.
    auto uavTexture = m_Device.CreateTexture( uavResource, texture->GetTextureUsage() );
    GenerateMips_UAV( uavTexture, Texture::IsSRGBFormat( resourceDesc.Format ) );

    if ( aliasResource )
    {
        AliasingBarrier( uavResource, aliasResource );
        // Copy the alias resource back to the original resource.
        CopyResource( d3d12Resource, aliasResource );
    }
}

void CommandList::GenerateMips_UAV( const std::shared_ptr<Texture>& texture, bool isSRGB )
{
    if ( !m_GenerateMipsPSO )
    {
        m_GenerateMipsPSO = std::make_unique<GenerateMipsPSO>( m_Device );
    }

    SetPipelineState( m_GenerateMipsPSO->GetPipelineState() );
    SetComputeRootSignature( m_GenerateMipsPSO->GetRootSignature() );

    GenerateMipsCB generateMipsCB;
    generateMipsCB.IsSRGB = isSRGB;

    auto resource     = texture->GetD3D12Resource();
    auto resourceDesc = resource->GetDesc();

    // Create an SRV that uses the format of the original texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                  = isSRGB ? Texture::GetSRGBFormat( resourceDesc.Format ) : resourceDesc.Format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension =
        D3D12_SRV_DIMENSION_TEXTURE2D;  // Only 2D textures are supported (this was checked in the calling function).
    srvDesc.Texture2D.MipLevels = resourceDesc.MipLevels;

    auto srv = m_Device.CreateShaderResourceView( texture, &srvDesc );

    for ( uint32_t srcMip = 0; srcMip < resourceDesc.MipLevels - 1u; )
    {
        uint64_t srcWidth  = resourceDesc.Width >> srcMip;
        uint32_t srcHeight = resourceDesc.Height >> srcMip;
        uint32_t dstWidth  = static_cast<uint32_t>( srcWidth >> 1 );
        uint32_t dstHeight = srcHeight >> 1;

        // 0b00(0): Both width and height are even.
        // 0b01(1): Width is odd, height is even.
        // 0b10(2): Width is even, height is odd.
        // 0b11(3): Both width and height are odd.
        generateMipsCB.SrcDimension = ( srcHeight & 1 ) << 1 | ( srcWidth & 1 );

        // How many mipmap levels to compute this pass (max 4 mips per pass)
        DWORD mipCount;

        // The number of times we can half the size of the texture and get
        // exactly a 50% reduction in size.
        // A 1 bit in the width or height indicates an odd dimension.
        // The case where either the width or the height is exactly 1 is handled
        // as a special case (as the dimension does not require reduction).
        _BitScanForward( &mipCount,
                         ( dstWidth == 1 ? dstHeight : dstWidth ) | ( dstHeight == 1 ? dstWidth : dstHeight ) );
        // Maximum number of mips to generate is 4.
        mipCount = std::min<DWORD>( 4, mipCount + 1 );
        // Clamp to total number of mips left over.
        mipCount = ( srcMip + mipCount ) >= resourceDesc.MipLevels ? resourceDesc.MipLevels - srcMip - 1 : mipCount;

        // Dimensions should not reduce to 0.
        // This can happen if the width and height are not the same.
        dstWidth  = std::max<DWORD>( 1, dstWidth );
        dstHeight = std::max<DWORD>( 1, dstHeight );

        generateMipsCB.SrcMipLevel  = srcMip;
        generateMipsCB.NumMipLevels = mipCount;
        generateMipsCB.TexelSize.x  = 1.0f / (float)dstWidth;
        generateMipsCB.TexelSize.y  = 1.0f / (float)dstHeight;

        SetCompute32BitConstants( GenerateMips::GenerateMipsCB, generateMipsCB );

        SetShaderResourceView( GenerateMips::SrcMip, 0, srv, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, srcMip,
                               1 );

        for ( uint32_t mip = 0; mip < mipCount; ++mip )
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format                           = resourceDesc.Format;
            uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice               = srcMip + mip + 1;

            auto uav = m_Device.CreateUnorderedAccessView( texture, nullptr, &uavDesc );
            SetUnorderedAccessView( GenerateMips::OutMip, mip, uav, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                                    srcMip + mip + 1, 1 );
        }

        // Pad any unused mip levels with a default UAV. Doing this keeps the DX12 runtime happy.
        if ( mipCount < 4 )
        {
            m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
                GenerateMips::OutMip, mipCount, 4 - mipCount, m_GenerateMipsPSO->GetDefaultUAV() );
        }

        Dispatch( Math::DivideByMultiple( dstWidth, 8 ), Math::DivideByMultiple( dstHeight, 8 ) );

        UAVBarrier( texture );

        srcMip += mipCount;
    }
}

void CommandList::PanoToCubemap( const std::shared_ptr<Texture>& cubemapTexture,
                                 const std::shared_ptr<Texture>& panoTexture )
{
    assert( cubemapTexture && panoTexture );

    if ( m_d3d12CommandListType == D3D12_COMMAND_LIST_TYPE_COPY )
    {
        if ( !m_ComputeCommandList )
        {
            m_ComputeCommandList = m_Device.GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COMPUTE ).GetCommandList();
        }
        m_ComputeCommandList->PanoToCubemap( cubemapTexture, panoTexture );
        return;
    }

    if ( !m_PanoToCubemapPSO )
    {
        m_PanoToCubemapPSO = std::make_unique<PanoToCubemapPSO>( m_Device );
    }

    auto cubemapResource = cubemapTexture->GetD3D12Resource();
    if ( !cubemapResource )
        return;

    CD3DX12_RESOURCE_DESC cubemapDesc( cubemapResource->GetDesc() );

    auto stagingResource = cubemapResource;
    auto stagingTexture  = m_Device.CreateTexture( stagingResource );
    // If the passed-in resource does not allow for UAV access
    // then create a staging resource that is used to generate
    // the cubemap.
    if ( ( cubemapDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ) == 0 )
    {
        auto d3d12Device = m_Device.GetD3D12Device();

        auto stagingDesc   = cubemapDesc;
        stagingDesc.Format = Texture::GetUAVCompatableFormat( cubemapDesc.Format );
        stagingDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        ThrowIfFailed( d3d12Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_DEFAULT ), D3D12_HEAP_FLAG_NONE, &stagingDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS( &stagingResource )

                ) );

        ResourceStateTracker::AddGlobalResourceState( stagingResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST );

        stagingTexture = m_Device.CreateTexture( stagingResource );
        stagingTexture->SetName( L"Pano to Cubemap Staging Texture" );

        CopyResource( stagingTexture, cubemapTexture );
    }

    TransitionBarrier( stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS );

    SetPipelineState( m_PanoToCubemapPSO->GetPipelineState() );
    SetComputeRootSignature( m_PanoToCubemapPSO->GetRootSignature() );

    PanoToCubemapCB panoToCubemapCB;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format                           = Texture::GetUAVCompatableFormat( cubemapDesc.Format );
    uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Texture2DArray.FirstArraySlice   = 0;
    uavDesc.Texture2DArray.ArraySize         = 6;

    auto srv = m_Device.CreateShaderResourceView( panoTexture );
    SetShaderResourceView( PanoToCubemapRS::SrcTexture, 0, srv, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE );

    for ( uint32_t mipSlice = 0; mipSlice < cubemapDesc.MipLevels; )
    {
        // Maximum number of mips to generate per pass is 5.
        uint32_t numMips = std::min<uint32_t>( 5, cubemapDesc.MipLevels - mipSlice );

        panoToCubemapCB.FirstMip = mipSlice;
        panoToCubemapCB.CubemapSize =
            std::max<uint32_t>( static_cast<uint32_t>( cubemapDesc.Width ), cubemapDesc.Height ) >> mipSlice;
        panoToCubemapCB.NumMips = numMips;

        SetCompute32BitConstants( PanoToCubemapRS::PanoToCubemapCB, panoToCubemapCB );

        for ( uint32_t mip = 0; mip < numMips; ++mip )
        {
            uavDesc.Texture2DArray.MipSlice = mipSlice + mip;

            auto uav = m_Device.CreateUnorderedAccessView( stagingTexture, nullptr, &uavDesc );
            SetUnorderedAccessView( PanoToCubemapRS::DstMips, mip, uav, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 0, 0 );
        }

        if ( numMips < 5 )
        {
            // Pad unused mips. This keeps DX12 runtime happy.
            m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
                PanoToCubemapRS::DstMips, panoToCubemapCB.NumMips, 5 - numMips, m_PanoToCubemapPSO->GetDefaultUAV() );
        }

        Dispatch( Math::DivideByMultiple( panoToCubemapCB.CubemapSize, 16 ),
                  Math::DivideByMultiple( panoToCubemapCB.CubemapSize, 16 ), 6 );

        mipSlice += numMips;
    }

    if ( stagingResource != cubemapResource )
    {
        CopyResource( cubemapTexture, stagingTexture );
    }
}

std::shared_ptr<Scene> CommandList::LoadSceneFromFile( const std::wstring&                 fileName,
                                                       const std::function<bool( float )>& loadingProgress )
{
    auto scene = std::make_shared<Scene>();

    scene->LoadSceneFromFile( *this, fileName, loadingProgress );

    return scene;
}

std::shared_ptr<Scene> CommandList::LoadSceneFromString( const std::string& sceneString, const std::string& format )
{
    auto scene = std::make_shared<Scene>();

    scene->LoadSceneFromString( *this, sceneString, format );

    return scene;
}

// Helper function to create a Scene from an index and vertex buffer.
std::shared_ptr<Scene> CommandList::CreateScene( const VertexCollection& vertices, const IndexCollection& indices )
{
    if ( vertices.empty() )
    {
        return nullptr;
    }

    auto vertexBuffer = CopyVertexBuffer( vertices );
    auto indexBuffer  = CopyIndexBuffer( indices );

    auto mesh = std::make_shared<Mesh>();
    // Create a default white material for new meshes.
    auto material = std::make_shared<Material>( Material::White );

    mesh->SetVertexBuffer( 0, vertexBuffer );
    mesh->SetIndexBuffer( indexBuffer );
    mesh->SetMaterial( material );

    auto node = std::make_shared<SceneNode>();
    node->AddMesh( mesh );

    auto scene = std::make_shared<Scene>();
    scene->SetRootNode( node );

    return scene;
}

std::shared_ptr<Scene> CommandList::CreateCube( float size, bool reverseWinding )
{
    // Cube is centered at 0,0,0.
    float s = size * 0.5f;

    // 8 edges of cube.
    XMFLOAT3 p[8] = { { s, s, -s }, { s, s, s },   { s, -s, s },   { s, -s, -s },
                      { -s, s, s }, { -s, s, -s }, { -s, -s, -s }, { -s, -s, s } };
    // 6 face normals
    XMFLOAT3 n[6] = { { 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 } };
    // 4 unique texture coordinates
    XMFLOAT3 t[4] = { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 } };

    // Indices for the vertex positions.
    uint16_t i[24] = {
        0, 1, 2, 3,  // +X
        4, 5, 6, 7,  // -X
        4, 1, 0, 5,  // +Y
        2, 7, 6, 3,  // -Y
        1, 4, 7, 2,  // +Z
        5, 0, 3, 6   // -Z
    };

    VertexCollection vertices;
    IndexCollection  indices;

    for ( uint16_t f = 0; f < 6; ++f )  // For each face of the cube.
    {
        // Four vertices per face.
        vertices.emplace_back( p[i[f * 4 + 0]], n[f], t[0] );
        vertices.emplace_back( p[i[f * 4 + 1]], n[f], t[1] );
        vertices.emplace_back( p[i[f * 4 + 2]], n[f], t[2] );
        vertices.emplace_back( p[i[f * 4 + 3]], n[f], t[3] );

        // First triangle.
        indices.emplace_back( f * 4 + 0 );
        indices.emplace_back( f * 4 + 1 );
        indices.emplace_back( f * 4 + 2 );

        // Second triangle
        indices.emplace_back( f * 4 + 2 );
        indices.emplace_back( f * 4 + 3 );
        indices.emplace_back( f * 4 + 0 );
    }

    if ( reverseWinding )
    {
        ReverseWinding( indices, vertices );
    }

    return CreateScene( vertices, indices );
}

std::shared_ptr<Scene> CommandList::CreateSphere( float radius, uint32_t tessellation, bool reversWinding )
{

    if ( tessellation < 3 )
        throw std::out_of_range( "tessellation parameter out of range" );

    VertexCollection vertices;
    IndexCollection  indices;

    size_t verticalSegments   = tessellation;
    size_t horizontalSegments = tessellation * 2;

    // Create rings of vertices at progressively higher latitudes.
    for ( size_t i = 0; i <= verticalSegments; i++ )
    {
        float v = 1 - (float)i / verticalSegments;

        float latitude = ( i * XM_PI / verticalSegments ) - XM_PIDIV2;
        float dy, dxz;

        XMScalarSinCos( &dy, &dxz, latitude );

        // Create a single ring of vertices at this latitude.
        for ( size_t j = 0; j <= horizontalSegments; j++ )
        {
            float u = (float)j / horizontalSegments;

            float longitude = j * XM_2PI / horizontalSegments;
            float dx, dz;

            XMScalarSinCos( &dx, &dz, longitude );

            dx *= dxz;
            dz *= dxz;

            auto normal            = XMVectorSet( dx, dy, dz, 0 );
            auto textureCoordinate = XMVectorSet( u, v, 0, 0 );
            auto position          = normal * radius;

            vertices.emplace_back( position, normal, textureCoordinate );
        }
    }

    // Fill the index buffer with triangles joining each pair of latitude rings.
    size_t stride = horizontalSegments + 1;

    for ( size_t i = 0; i < verticalSegments; i++ )
    {
        for ( size_t j = 0; j <= horizontalSegments; j++ )
        {
            size_t nextI = i + 1;
            size_t nextJ = ( j + 1 ) % stride;

            indices.push_back( i * stride + nextJ );
            indices.push_back( nextI * stride + j );
            indices.push_back( i * stride + j );

            indices.push_back( nextI * stride + nextJ );
            indices.push_back( nextI * stride + j );
            indices.push_back( i * stride + nextJ );
        }
    }

    if ( reversWinding )
    {
        ReverseWinding( indices, vertices );
    }

    return CreateScene( vertices, indices );
}

void CommandList::CreateCylinderCap( VertexCollection& vertices, IndexCollection& indices, size_t tessellation,
                                     float height, float radius, bool isTop )
{
    // Create cap indices.
    for ( size_t i = 0; i < tessellation - 2; i++ )
    {
        size_t i1 = ( i + 1 ) % tessellation;
        size_t i2 = ( i + 2 ) % tessellation;

        if ( isTop )
        {
            std::swap( i1, i2 );
        }

        size_t vbase = vertices.size();
        indices.push_back( vbase + i2 );
        indices.push_back( vbase + i1 );
        indices.push_back( vbase );
    }

    // Which end of the cylinder is this?
    XMVECTOR normal       = g_XMIdentityR1;
    XMVECTOR textureScale = g_XMNegativeOneHalf;

    if ( !isTop )
    {
        normal       = XMVectorNegate( normal );
        textureScale = XMVectorMultiply( textureScale, g_XMNegateX );
    }

    // Create cap vertices.
    for ( size_t i = 0; i < tessellation; i++ )
    {
        XMVECTOR circleVector = GetCircleVector( i, tessellation );
        XMVECTOR position     = XMVectorAdd( XMVectorScale( circleVector, radius ), XMVectorScale( normal, height ) );
        XMVECTOR textureCoordinate =
            XMVectorMultiplyAdd( XMVectorSwizzle<0, 2, 3, 3>( circleVector ), textureScale, g_XMOneHalf );

        vertices.emplace_back( position, normal, textureCoordinate );
    }
}

std::shared_ptr<Scene> CommandList::CreateCylinder( float radius, float height, uint32_t tessellation,
                                                    bool reverseWinding )
{
    if ( tessellation < 3 )
        throw std::out_of_range( "tessellation parameter out of range" );

    VertexCollection vertices;
    IndexCollection  indices;

    height /= 2;

    XMVECTOR topOffset = XMVectorScale( g_XMIdentityR1, height );

    size_t stride = tessellation + 1;

    // Create a ring of triangles around the outside of the cylinder.
    for ( size_t i = 0; i <= tessellation; i++ )
    {
        XMVECTOR normal = GetCircleVector( i, tessellation );

        XMVECTOR sideOffset = XMVectorScale( normal, radius );

        float u = float( i ) / float( tessellation );

        XMVECTOR textureCoordinate = XMLoadFloat( &u );

        vertices.emplace_back( XMVectorAdd( sideOffset, topOffset ), normal, textureCoordinate );
        vertices.emplace_back( XMVectorSubtract( sideOffset, topOffset ), normal,
                               XMVectorAdd( textureCoordinate, g_XMIdentityR1 ) );

        indices.push_back( i * 2 );
        indices.push_back( ( i * 2 + 2 ) % ( stride * 2 ) );
        indices.push_back( i * 2 + 1 );

        indices.push_back( i * 2 + 1 );
        indices.push_back( ( i * 2 + 2 ) % ( stride * 2 ) );
        indices.push_back( ( i * 2 + 3 ) % ( stride * 2 ) );
    }

    // Create flat triangle fan caps to seal the top and bottom.
    CreateCylinderCap( vertices, indices, tessellation, height, radius, true );
    CreateCylinderCap( vertices, indices, tessellation, height, radius, false );

    // Build RH above
    if ( reverseWinding )
    {
        ReverseWinding( indices, vertices );
    }

    return CreateScene( vertices, indices );
}

std::shared_ptr<Scene> CommandList::CreateCone( float radius, float height, uint32_t tessellation, bool reverseWinding )
{
    if ( tessellation < 3 )
        throw std::out_of_range( "tessellation parameter out of range" );

    VertexCollection vertices;
    IndexCollection  indices;

    height /= 2;

    XMVECTOR topOffset = XMVectorScale( g_XMIdentityR1, height );

    size_t stride = tessellation + 1;

    // Create a ring of triangles around the outside of the cone.
    for ( size_t i = 0; i <= tessellation; i++ )
    {
        XMVECTOR circlevec = GetCircleVector( i, tessellation );

        XMVECTOR sideOffset = XMVectorScale( circlevec, radius );

        float u = float( i ) / float( tessellation );

        XMVECTOR textureCoordinate = XMLoadFloat( &u );

        XMVECTOR pt = XMVectorSubtract( sideOffset, topOffset );

        XMVECTOR normal = XMVector3Cross( GetCircleTangent( i, tessellation ), XMVectorSubtract( topOffset, pt ) );
        normal          = XMVector3Normalize( normal );

        // Duplicate the top vertex for distinct normals
        vertices.emplace_back( topOffset, normal, g_XMZero );
        vertices.emplace_back( pt, normal, XMVectorAdd( textureCoordinate, g_XMIdentityR1 ) );

        indices.push_back( ( i * 2 + 1 ) % ( stride * 2 ) );
        indices.push_back( ( i * 2 + 3 ) % ( stride * 2 ) );
        indices.push_back( i * 2 );
    }

    // Create flat triangle fan caps to seal the bottom.
    CreateCylinderCap( vertices, indices, tessellation, height, radius, false );

    // Build RH above
    if ( reverseWinding )
    {
        ReverseWinding( indices, vertices );
    }

    return CreateScene( vertices, indices );
}

std::shared_ptr<Scene> CommandList::CreateTorus( float radius, float thickness, uint32_t tessellation,
                                                 bool reverseWinding )
{
    assert( tessellation > 3 );

    VertexCollection verticies;
    IndexCollection  indices;

    size_t stride = tessellation + 1;

    // First we loop around the main ring of the torus.
    for ( size_t i = 0; i <= tessellation; i++ )
    {
        float u = (float)i / tessellation;

        float outerAngle = i * XM_2PI / tessellation - XM_PIDIV2;

        // Create a transform matrix that will align geometry to
        // slice perpendicularly though the current ring position.
        XMMATRIX transform = XMMatrixTranslation( radius, 0, 0 ) * XMMatrixRotationY( outerAngle );

        // Now we loop along the other axis, around the side of the tube.
        for ( size_t j = 0; j <= tessellation; j++ )
        {
            float v = 1 - (float)j / tessellation;

            float innerAngle = j * XM_2PI / tessellation + XM_PI;
            float dx, dy;

            XMScalarSinCos( &dy, &dx, innerAngle );

            // Create a vertex.
            auto normal            = XMVectorSet( dx, dy, 0, 0 );
            auto position          = normal * thickness / 2;
            auto textureCoordinate = XMVectorSet( u, v, 0, 0 );

            position = XMVector3Transform( position, transform );
            normal   = XMVector3TransformNormal( normal, transform );

            verticies.emplace_back( position, normal, textureCoordinate );

            // And create indices for two triangles.
            size_t nextI = ( i + 1 ) % stride;
            size_t nextJ = ( j + 1 ) % stride;

            indices.push_back( nextI * stride + j );
            indices.push_back( i * stride + nextJ );
            indices.push_back( i * stride + j );

            indices.push_back( nextI * stride + j );
            indices.push_back( nextI * stride + nextJ );
            indices.push_back( i * stride + nextJ );
        }
    }

    if ( reverseWinding )
    {
        ReverseWinding( indices, verticies );
    }

    return CreateScene( verticies, indices );
}

std::shared_ptr<Scene> CommandList::CreatePlane( float width, float height, bool reverseWinding )
{
    VertexCollection vertices = {
        Mesh::Vertex( XMFLOAT3( -0.5f * width, 0.0f, 0.5f * height ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT3( 0.0f, 0.0f, 0.0f ) ),  // 0
        Mesh::Vertex( XMFLOAT3( 0.5f * width, 0.0f, 0.5f * height ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ) ),   // 1
        Mesh::Vertex( XMFLOAT3( 0.5f * width, 0.0f, -0.5f * height ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT3( 1.0f, 1.0f, 0.0f ) ),  // 2
        Mesh::Vertex( XMFLOAT3( -0.5f * width, 0.0f, -0.5f * height ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ) )  // 3
    };

    IndexCollection indices = { 1, 3, 0, 2, 3, 1 };

    if ( reverseWinding )
    {
        ReverseWinding( indices, vertices );
    }

    return CreateScene( vertices, indices );
}

void CommandList::ClearTexture( const std::shared_ptr<Texture>& texture, const float clearColor[4] )
{
    assert( texture );

    TransitionBarrier( texture, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true );
    m_d3d12CommandList->ClearRenderTargetView( texture->GetRenderTargetView(), clearColor, 0, nullptr );

    TrackResource( texture );
}

void CommandList::ClearDepthStencilTexture( const std::shared_ptr<Texture>& texture, D3D12_CLEAR_FLAGS clearFlags,
                                            float depth, uint8_t stencil )
{
    assert( texture );

    TransitionBarrier( texture, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, true );
    m_d3d12CommandList->ClearDepthStencilView( texture->GetDepthStencilView(), clearFlags, depth, stencil, 0, nullptr );

    TrackResource( texture );
}

void CommandList::CopyTextureSubresource( const std::shared_ptr<Texture>& texture, uint32_t firstSubresource,
                                          uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData )
{
    assert( texture );

    auto d3d12Device         = m_Device.GetD3D12Device();
    auto destinationResource = texture->GetD3D12Resource();

    if ( destinationResource )
    {
        // Resource must be in the copy-destination state.
        TransitionBarrier( texture, D3D12_RESOURCE_STATE_COPY_DEST );
        FlushResourceBarriers();

        UINT64 requiredSize =
            GetRequiredIntermediateSize( destinationResource.Get(), firstSubresource, numSubresources );

        // Create a temporary (intermediate) resource for uploading the subresources
        ComPtr<ID3D12Resource> intermediateResource;
        ThrowIfFailed( d3d12Device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES( D3D12_HEAP_TYPE_UPLOAD ), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer( requiredSize ), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS( &intermediateResource ) ) );

        UpdateSubresources( m_d3d12CommandList.Get(), destinationResource.Get(), intermediateResource.Get(), 0,
                            firstSubresource, numSubresources, subresourceData );

        TrackResource( intermediateResource );
        TrackResource( destinationResource );
    }
}

void CommandList::SetGraphicsDynamicConstantBuffer( uint32_t rootParameterIndex, size_t sizeInBytes,
                                                    const void* bufferData )
{
    // Constant buffers must be 256-byte aligned.
    auto heapAllococation = m_UploadBuffer->Allocate( sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT );
    memcpy( heapAllococation.CPU, bufferData, sizeInBytes );

    m_d3d12CommandList->SetGraphicsRootConstantBufferView( rootParameterIndex, heapAllococation.GPU );
}

void CommandList::SetGraphics32BitConstants( uint32_t rootParameterIndex, uint32_t numConstants, const void* constants )
{
    m_d3d12CommandList->SetGraphicsRoot32BitConstants( rootParameterIndex, numConstants, constants, 0 );
}

void CommandList::SetCompute32BitConstants( uint32_t rootParameterIndex, uint32_t numConstants, const void* constants )
{
    m_d3d12CommandList->SetComputeRoot32BitConstants( rootParameterIndex, numConstants, constants, 0 );
}

void CommandList::SetVertexBuffers( uint32_t                                          startSlot,
                                    const std::vector<std::shared_ptr<VertexBuffer>>& vertexBuffers )
{
    std::vector<D3D12_VERTEX_BUFFER_VIEW> views;
    views.reserve( vertexBuffers.size() );

    for ( auto vertexBuffer: vertexBuffers )
    {
        if ( vertexBuffer )
        {
            TransitionBarrier( vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER );
            TrackResource( vertexBuffer );

            views.push_back( vertexBuffer->GetVertexBufferView() );
        }
    }

    m_d3d12CommandList->IASetVertexBuffers( startSlot, views.size(), views.data() );
}

void CommandList::SetVertexBuffer( uint32_t slot, const std::shared_ptr<VertexBuffer>& vertexBuffer )
{
    SetVertexBuffers( slot, { vertexBuffer } );
}

void CommandList::SetDynamicVertexBuffer( uint32_t slot, size_t numVertices, size_t vertexSize,
                                          const void* vertexBufferData )
{
    size_t bufferSize = numVertices * vertexSize;

    auto heapAllocation = m_UploadBuffer->Allocate( bufferSize, vertexSize );
    memcpy( heapAllocation.CPU, vertexBufferData, bufferSize );

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
    vertexBufferView.BufferLocation           = heapAllocation.GPU;
    vertexBufferView.SizeInBytes              = static_cast<UINT>( bufferSize );
    vertexBufferView.StrideInBytes            = static_cast<UINT>( vertexSize );

    m_d3d12CommandList->IASetVertexBuffers( slot, 1, &vertexBufferView );
}

void CommandList::SetIndexBuffer( const std::shared_ptr<IndexBuffer>& indexBuffer )
{
    if ( indexBuffer )
    {
        TransitionBarrier( indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER );
        TrackResource( indexBuffer );
        m_d3d12CommandList->IASetIndexBuffer( &( indexBuffer->GetIndexBufferView() ) );
    }
}

void CommandList::SetDynamicIndexBuffer( size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData )
{
    size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
    size_t bufferSize       = numIndicies * indexSizeInBytes;

    auto heapAllocation = m_UploadBuffer->Allocate( bufferSize, indexSizeInBytes );
    memcpy( heapAllocation.CPU, indexBufferData, bufferSize );

    D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
    indexBufferView.BufferLocation          = heapAllocation.GPU;
    indexBufferView.SizeInBytes             = static_cast<UINT>( bufferSize );
    indexBufferView.Format                  = indexFormat;

    m_d3d12CommandList->IASetIndexBuffer( &indexBufferView );
}

void CommandList::SetGraphicsDynamicStructuredBuffer( uint32_t slot, size_t numElements, size_t elementSize,
                                                      const void* bufferData )
{
    size_t bufferSize = numElements * elementSize;

    auto heapAllocation = m_UploadBuffer->Allocate( bufferSize, elementSize );

    memcpy( heapAllocation.CPU, bufferData, bufferSize );

    m_d3d12CommandList->SetGraphicsRootShaderResourceView( slot, heapAllocation.GPU );
}
void CommandList::SetViewport( const D3D12_VIEWPORT& viewport )
{
    SetViewports( { viewport } );
}

void CommandList::SetViewports( const std::vector<D3D12_VIEWPORT>& viewports )
{
    assert( viewports.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE );
    m_d3d12CommandList->RSSetViewports( static_cast<UINT>( viewports.size() ), viewports.data() );
}

void CommandList::SetScissorRect( const D3D12_RECT& scissorRect )
{
    SetScissorRects( { scissorRect } );
}

void CommandList::SetScissorRects( const std::vector<D3D12_RECT>& scissorRects )
{
    assert( scissorRects.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE );
    m_d3d12CommandList->RSSetScissorRects( static_cast<UINT>( scissorRects.size() ), scissorRects.data() );
}

void CommandList::SetPipelineState( const std::shared_ptr<PipelineStateObject>& pipelineState )
{
    assert( pipelineState );

    auto d3d12PipelineStateObject = pipelineState->GetD3D12PipelineState();

    m_d3d12CommandList->SetPipelineState( d3d12PipelineStateObject.Get() );

    TrackResource( d3d12PipelineStateObject );
}

void CommandList::SetGraphicsRootSignature( const std::shared_ptr<RootSignature>& rootSignature )
{
    assert( rootSignature );

    auto d3d12RootSignature = rootSignature->GetD3D12RootSignature().Get();
    if ( m_RootSignature != d3d12RootSignature )
    {
        m_RootSignature = d3d12RootSignature;

        for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
        {
            m_DynamicDescriptorHeap[i]->ParseRootSignature( rootSignature );
        }

        m_d3d12CommandList->SetGraphicsRootSignature( m_RootSignature );

        TrackResource( m_RootSignature );
    }
}

void CommandList::SetComputeRootSignature( const std::shared_ptr<RootSignature>& rootSignature )
{
    assert( rootSignature );

    auto d3d12RootSignature = rootSignature->GetD3D12RootSignature().Get();
    if ( m_RootSignature != d3d12RootSignature )
    {
        m_RootSignature = d3d12RootSignature;

        for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
        {
            m_DynamicDescriptorHeap[i]->ParseRootSignature( rootSignature );
        }

        m_d3d12CommandList->SetComputeRootSignature( m_RootSignature );

        TrackResource( m_RootSignature );
    }
}

void CommandList::SetConstantBufferView( uint32_t rootParameterIndex, const std::shared_ptr<ConstantBuffer>& buffer,
                                         D3D12_RESOURCE_STATES stateAfter, size_t bufferOffset )
{
    if ( buffer )
    {
        auto d3d12Resource = buffer->GetD3D12Resource();
        TransitionBarrier( d3d12Resource, stateAfter );

        m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageInlineCBV(
            rootParameterIndex, d3d12Resource->GetGPUVirtualAddress() + bufferOffset );

        TrackResource( buffer );
    }
}

void CommandList::SetShaderResourceView( uint32_t rootParameterIndex, const std::shared_ptr<Buffer>& buffer,
                                         D3D12_RESOURCE_STATES stateAfter, size_t bufferOffset )
{
    if ( buffer )
    {
        auto d3d12Resource = buffer->GetD3D12Resource();
        TransitionBarrier( d3d12Resource, stateAfter );

        m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageInlineSRV(
            rootParameterIndex, d3d12Resource->GetGPUVirtualAddress() + bufferOffset );

        TrackResource( buffer );
    }
}

void CommandList::SetUnorderedAccessView( uint32_t rootParameterIndex, const std::shared_ptr<Buffer>& buffer,
                                          D3D12_RESOURCE_STATES stateAfter, size_t bufferOffset )
{
    if ( buffer )
    {
        auto d3d12Resource = buffer->GetD3D12Resource();
        TransitionBarrier( d3d12Resource, stateAfter );

        m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageInlineUAV(
            rootParameterIndex, d3d12Resource->GetGPUVirtualAddress() + bufferOffset );

        TrackResource( buffer );
    }
}

void CommandList::SetShaderResourceView( uint32_t rootParameterIndex, uint32_t descriptorOffset,
                                         const std::shared_ptr<ShaderResourceView>& srv,
                                         D3D12_RESOURCE_STATES stateAfter, UINT firstSubresource, UINT numSubresources )
{
    assert( srv );

    auto resource = srv->GetResource();
    if ( resource )
    {
        if ( numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES )
        {
            for ( uint32_t i = 0; i < numSubresources; ++i )
            {
                TransitionBarrier( resource, stateAfter, firstSubresource + i );
            }
        }
        else
        {
            TransitionBarrier( resource, stateAfter );
        }
        TrackResource( resource );
    }

    m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
        rootParameterIndex, descriptorOffset, 1, srv->GetDescriptorHandle() );
}

void CommandList::SetUnorderedAccessView( uint32_t rootParameterIndex, uint32_t descrptorOffset,
                                          const std::shared_ptr<UnorderedAccessView>& uav,
                                          D3D12_RESOURCE_STATES stateAfter, UINT firstSubresource,
                                          UINT numSubresources )
{
    assert( uav );

    auto resource = uav->GetResource();
    if ( resource )
    {
        if ( numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES )
        {
            for ( uint32_t i = 0; i < numSubresources; ++i )
            {
                TransitionBarrier( resource, stateAfter, firstSubresource + i );
            }
        }
        else
        {
            TransitionBarrier( resource, stateAfter );
        }

        TrackResource( resource );
    }

    m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
        rootParameterIndex, descrptorOffset, 1, uav->GetDescriptorHandle() );
}

void CommandList::SetConstantBufferView( uint32_t rootParameterIndex, uint32_t descriptorOffset,
                                         const std::shared_ptr<ConstantBufferView>& cbv,
                                         D3D12_RESOURCE_STATES                      stateAfter )
{
    assert( cbv );

    auto constantBuffer = cbv->GetConstantBuffer();
    if ( constantBuffer )
    {
        TransitionBarrier( constantBuffer, stateAfter );
        TrackResource( constantBuffer );
    }

    m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(
        rootParameterIndex, descriptorOffset, 1, cbv->GetDescriptorHandle() );
}

void CommandList::SetRenderTarget( const RenderTarget& renderTarget )
{
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
    renderTargetDescriptors.reserve( AttachmentPoint::NumAttachmentPoints );

    const auto& textures = renderTarget.GetTextures();

    // Bind color targets (max of 8 render targets can be bound to the rendering pipeline.
    for ( int i = 0; i < 8; ++i )
    {
        auto texture = textures[i];

        if ( texture )
        {
            TransitionBarrier( texture, D3D12_RESOURCE_STATE_RENDER_TARGET );
            renderTargetDescriptors.push_back( texture->GetRenderTargetView() );

            TrackResource( texture );
        }
    }

    auto depthTexture = renderTarget.GetTexture( AttachmentPoint::DepthStencil );

    CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilDescriptor( D3D12_DEFAULT );
    if ( depthTexture )
    {
        TransitionBarrier( depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE );
        depthStencilDescriptor = depthTexture->GetDepthStencilView();

        TrackResource( depthTexture );
    }

    D3D12_CPU_DESCRIPTOR_HANDLE* pDSV = depthStencilDescriptor.ptr != 0 ? &depthStencilDescriptor : nullptr;

    m_d3d12CommandList->OMSetRenderTargets( static_cast<UINT>( renderTargetDescriptors.size() ),
                                            renderTargetDescriptors.data(), FALSE, pDSV );
}

void CommandList::Draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance )
{
    FlushResourceBarriers();

    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    {
        m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw( *this );
    }

    m_d3d12CommandList->DrawInstanced( vertexCount, instanceCount, startVertex, startInstance );
}

void CommandList::DrawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex,
                               uint32_t startInstance )
{
    FlushResourceBarriers();

    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    {
        m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw( *this );
    }

    m_d3d12CommandList->DrawIndexedInstanced( indexCount, instanceCount, startIndex, baseVertex, startInstance );
}

void CommandList::Dispatch( uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ )
{
    FlushResourceBarriers();

    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    {
        m_DynamicDescriptorHeap[i]->CommitStagedDescriptorsForDispatch( *this );
    }

    m_d3d12CommandList->Dispatch( numGroupsX, numGroupsY, numGroupsZ );
}

bool CommandList::Close( const std::shared_ptr<CommandList>& pendingCommandList )
{
    // Flush any remaining barriers.
    FlushResourceBarriers();

    m_d3d12CommandList->Close();

    // Flush pending resource barriers.
    uint32_t numPendingBarriers = m_ResourceStateTracker->FlushPendingResourceBarriers( pendingCommandList );
    // Commit the final resource state to the global state.
    m_ResourceStateTracker->CommitFinalResourceStates();

    return numPendingBarriers > 0;
}

void CommandList::Close()
{
    FlushResourceBarriers();
    m_d3d12CommandList->Close();
}

void CommandList::Reset()
{
    ThrowIfFailed( m_d3d12CommandAllocator->Reset() );
    ThrowIfFailed( m_d3d12CommandList->Reset( m_d3d12CommandAllocator.Get(), nullptr ) );

    m_ResourceStateTracker->Reset();
    m_UploadBuffer->Reset();

    ReleaseTrackedObjects();

    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    {
        m_DynamicDescriptorHeap[i]->Reset();
        m_DescriptorHeaps[i] = nullptr;
    }

    m_RootSignature      = nullptr;
    m_ComputeCommandList = nullptr;
}

void CommandList::TrackResource( Microsoft::WRL::ComPtr<ID3D12Object> object )
{
    m_TrackedObjects.push_back( object );
}

void CommandList::TrackResource( const std::shared_ptr<Resource>& res )
{
    assert( res );

    TrackResource( res->GetD3D12Resource() );
}

void CommandList::ReleaseTrackedObjects()
{
    m_TrackedObjects.clear();
}

void CommandList::SetDescriptorHeap( D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap )
{
    if ( m_DescriptorHeaps[heapType] != heap )
    {
        m_DescriptorHeaps[heapType] = heap;
        BindDescriptorHeaps();
    }
}

void CommandList::BindDescriptorHeaps()
{
    UINT                  numDescriptorHeaps                                    = 0;
    ID3D12DescriptorHeap* descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

    for ( uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    {
        ID3D12DescriptorHeap* descriptorHeap = m_DescriptorHeaps[i];
        if ( descriptorHeap )
        {
            descriptorHeaps[numDescriptorHeaps++] = descriptorHeap;
        }
    }

    m_d3d12CommandList->SetDescriptorHeaps( numDescriptorHeaps, descriptorHeaps );
}
