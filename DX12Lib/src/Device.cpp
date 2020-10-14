#include "DX12LibPCH.h"

#include <dx12lib/Adapter.h>
#include <dx12lib/ByteAddressBuffer.h>
#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/DescriptorAllocator.h>
#include <dx12lib/Device.h>
#include <dx12lib/IndexBuffer.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/StructuredBuffer.h>
#include <dx12lib/SwapChain.h>
#include <dx12lib/Texture.h>
#include <dx12lib/VertexBuffer.h>

using namespace dx12lib;

class MakeRootSignature : public RootSignature
{
public:
    MakeRootSignature( std::shared_ptr<Device> device, const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc,
                       D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion )
    : RootSignature( device, rootSignatureDesc, rootSignatureVersion )
    {}

    virtual ~MakeRootSignature() {}
};

class MakeCommandList : public CommandList
{
public:
    MakeCommandList( std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type )
    : CommandList( device, type )
    {}

    virtual ~MakeCommandList() {}
};

// Adapter for std::make_shared
class MakeTexture : public Texture
{
public:
    MakeTexture( std::shared_ptr<Device> device, const D3D12_RESOURCE_DESC& resourceDesc,
                 const D3D12_CLEAR_VALUE* clearValue, TextureUsage texturUsage )
    : Texture( device, resourceDesc, clearValue, texturUsage )
    {}

    MakeTexture( std::shared_ptr<Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                 const D3D12_CLEAR_VALUE* clearValue, TextureUsage textureUsage )
    : Texture( device, resource, clearValue, textureUsage )
    {}

    virtual ~MakeTexture() {}
};

// Adapter for std::make_shared
class MakeStructuredBuffer : public StructuredBuffer
{
public:
    MakeStructuredBuffer( std::shared_ptr<Device> device, size_t numElements, size_t elementSize )
    : StructuredBuffer( device, numElements, elementSize )
    {}

    MakeStructuredBuffer( std::shared_ptr<Device> device, ComPtr<ID3D12Resource> resource, size_t numElements,
                          size_t elementSize )
    : StructuredBuffer( device, resource, numElements, elementSize )
    {}

    virtual ~MakeStructuredBuffer() {}
};

// Adapter for std::make_shared
class MakeVertexBuffer : public VertexBuffer
{
public:
    MakeVertexBuffer( std::shared_ptr<Device> device, size_t numVertices, size_t vertexStride )
    : VertexBuffer( device, numVertices, vertexStride )
    {}

    MakeVertexBuffer( std::shared_ptr<Device> device, ComPtr<ID3D12Resource> resource, size_t numVertices,
                      size_t vertexStride )
    : VertexBuffer( device, resource, numVertices, vertexStride )
    {}

    virtual ~MakeVertexBuffer() {}
};

// Adapter for std::make_shared
class MakeIndexBuffer : public IndexBuffer
{
public:
    MakeIndexBuffer( std::shared_ptr<Device> device, size_t numIndicies, DXGI_FORMAT indexFormat )
    : IndexBuffer( device, numIndicies, indexFormat )
    {}

    MakeIndexBuffer( std::shared_ptr<Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                     size_t numIndicies, DXGI_FORMAT indexFormat )
    : IndexBuffer( device, resource, numIndicies, indexFormat )
    {}

    virtual ~MakeIndexBuffer() {}
};

// Adapter for std::make_shared
class MakeByteAddressBuffer : public ByteAddressBuffer
{
public:
    MakeByteAddressBuffer( std::shared_ptr<Device> device, const D3D12_RESOURCE_DESC& desc )
    : ByteAddressBuffer( device, desc )
    {}

    MakeByteAddressBuffer( std::shared_ptr<Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> resoruce )
    : ByteAddressBuffer( device, resoruce )
    {}

    virtual ~MakeByteAddressBuffer() {}
};

// Adapter for std::make_unique
class MakeDescriptorAllocator : public DescriptorAllocator
{
public:
    MakeDescriptorAllocator( std::shared_ptr<Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type,
                             uint32_t numDescriptorsPerHeap = 256 )
    : DescriptorAllocator( device, type, numDescriptorsPerHeap )
    {}

    virtual ~MakeDescriptorAllocator() {}
};

// Adapter for std::make_shared
class MakeSwapChain : public SwapChain
{
public:
    MakeSwapChain( std::shared_ptr<Device> device, HWND hWnd )
    : SwapChain( device, hWnd )
    {}

    virtual ~MakeSwapChain() {}
};

// Adapter for std::make_shared
class MakeCommandQueue : public CommandQueue
{
public:
    MakeCommandQueue( std::shared_ptr<Device> device, D3D12_COMMAND_LIST_TYPE type )
    : CommandQueue( device, type )
    {}

    virtual ~MakeCommandQueue() {}
};

// An adapter for std::make_shared
class MakeDevice : public Device
{
public:
    MakeDevice( std::shared_ptr<Adapter> adapter )
    : Device( adapter )
    {}

    virtual ~MakeDevice() {}
};

std::shared_ptr<Device> Device::CreateDevice( std::shared_ptr<Adapter> adapter )
{
    std::shared_ptr<Device> device;

    device = std::make_shared<MakeDevice>( adapter );

    if ( device )
    {
        device->Init();
    }

    return device;
}

std::wstring Device::GetDescription() const
{
    return m_Adapter->GetDescription();
}

Device::Device( std::shared_ptr<Adapter> adapter )
: m_Adapter( adapter )
{
    if ( !m_Adapter )
    {
        m_Adapter = Adapter::Create();
        assert( m_Adapter );
    }

    auto dxgiAdapter = m_Adapter->GetDXGIAdapter();

    ThrowIfFailed( D3D12CreateDevice( dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &m_d3d12Device ) ) );

    // Enable debug messages in debug mode.
#if defined( _DEBUG )
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if ( SUCCEEDED( m_d3d12Device.As( &pInfoQueue ) ) )
    {
        pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE );
        pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, TRUE );
        pInfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, TRUE );

        // Suppress whole categories of messages
        // D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,  // I'm really not sure how to avoid this
                                                                           // message.

            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,  // This warning occurs when using capture frame while graphics
                                                     // debugging.

            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,  // This warning occurs when using capture frame while graphics
                                                       // debugging.
        };

        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        // NewFilter.DenyList.NumCategories = _countof(Categories);
        // NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof( Severities );
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs        = _countof( DenyIds );
        NewFilter.DenyList.pIDList       = DenyIds;

        ThrowIfFailed( pInfoQueue->PushStorageFilter( &NewFilter ) );
    }
#endif
}

Device::~Device() {}

void Device::Init()
{
    auto shared_this      = shared_from_this();
    m_DirectCommandQueue  = std::make_shared<MakeCommandQueue>( shared_this, D3D12_COMMAND_LIST_TYPE_DIRECT );
    m_ComputeCommandQueue = std::make_shared<MakeCommandQueue>( shared_this, D3D12_COMMAND_LIST_TYPE_COMPUTE );
    m_CopyCommandQueue    = std::make_shared<MakeCommandQueue>( shared_this, D3D12_COMMAND_LIST_TYPE_COPY );

    // Create descriptor allocators
    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    {
        m_DescriptorAllocators[i] =
            std::make_unique<MakeDescriptorAllocator>( shared_this, static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>( i ) );
    }
}

std::shared_ptr<CommandQueue> Device::GetCommandQueue( D3D12_COMMAND_LIST_TYPE type ) const
{
    std::shared_ptr<CommandQueue> commandQueue;
    switch ( type )
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        commandQueue = m_DirectCommandQueue;
        break;
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        commandQueue = m_ComputeCommandQueue;
        break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
        commandQueue = m_CopyCommandQueue;
        break;
    default:
        assert( false && "Invalid command queue type." );
    }

    return commandQueue;
}

ComPtr<ID3D12DescriptorHeap> Device::CreateDescriptorHeap( UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type )
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type                       = type;
    desc.NumDescriptors             = numDescriptors;
    desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask                   = 0;

    ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    ThrowIfFailed( m_d3d12Device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &descriptorHeap ) ) );

    return descriptorHeap;
}

void Device::Flush()
{
    m_DirectCommandQueue->Flush();
    m_ComputeCommandQueue->Flush();
    m_CopyCommandQueue->Flush();
}

DescriptorAllocation Device::AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors )
{
    return m_DescriptorAllocators[type]->Allocate( numDescriptors );
}

void Device::ReleaseStaleDescriptors( uint64_t fenceValue )
{
    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    { m_DescriptorAllocators[i]->ReleaseStaleDescriptors( fenceValue ); }
}

std::shared_ptr<SwapChain> Device::CreateSwapChain( HWND hWnd )
{
    std::shared_ptr<SwapChain> swapChain;
    swapChain = std::make_shared<MakeSwapChain>( shared_from_this(), hWnd );

    return swapChain;
}

std::shared_ptr<ByteAddressBuffer> Device::CreateByteAddressBuffer( size_t bufferSize )
{
    // Align-up to 4-bytes
    bufferSize = Math::AlignUp( bufferSize, 4 );

    std::shared_ptr<ByteAddressBuffer> buffer = std::make_shared<MakeByteAddressBuffer>(
        shared_from_this(), CD3DX12_RESOURCE_DESC::Buffer( bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ) );

    return buffer;
}

std::shared_ptr<ByteAddressBuffer> Device::CreateByteAddressBuffer( ComPtr<ID3D12Resource> resource )
{
    std::shared_ptr<ByteAddressBuffer> buffer = std::make_shared<MakeByteAddressBuffer>( shared_from_this(), resource );

    return buffer;
}

std::shared_ptr<StructuredBuffer> Device::CreateStructuredBuffer( size_t numElements, size_t elementSize )
{
    std::shared_ptr<StructuredBuffer> structuredBuffer =
        std::make_shared<MakeStructuredBuffer>( shared_from_this(), numElements, elementSize );

    return structuredBuffer;
}

std::shared_ptr<StructuredBuffer> Device::CreateStructuredBuffer( ComPtr<ID3D12Resource> resource, size_t numElements,
                                                                  size_t elementSize )
{
    std::shared_ptr<StructuredBuffer> structuredBuffer =
        std::make_shared<MakeStructuredBuffer>( shared_from_this(), resource, numElements, elementSize );

    return structuredBuffer;
}

std::shared_ptr<IndexBuffer> Device::CreateIndexBuffer( size_t numIndicies, DXGI_FORMAT indexFormat )
{
    std::shared_ptr<IndexBuffer> indexBuffer =
        std::make_shared<MakeIndexBuffer>( shared_from_this(), numIndicies, indexFormat );

    return indexBuffer;
}

std::shared_ptr<dx12lib::IndexBuffer>
    dx12lib::Device::CreateIndexBuffer( Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numIndices,
                                        DXGI_FORMAT indexFormat )
{
    std::shared_ptr<IndexBuffer> indexBuffer =
        std::make_shared<MakeIndexBuffer>( shared_from_this(), resource, numIndices, indexFormat );

    return indexBuffer;
}

std::shared_ptr<VertexBuffer> Device::CreateVertexBuffer( size_t numVertices, size_t vertexStride )
{
    std::shared_ptr<VertexBuffer> vertexBuffer =
        std::make_shared<MakeVertexBuffer>( shared_from_this(), numVertices, vertexStride );

    return vertexBuffer;
}

std::shared_ptr<dx12lib::VertexBuffer>
    dx12lib::Device::CreateVertexBuffer( Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices,
                                         size_t vertexStride )
{
    std::shared_ptr<VertexBuffer> vertexBuffer =
        std::make_shared<MakeVertexBuffer>( shared_from_this(), resource, numVertices, vertexStride );

    return vertexBuffer;
}

std::shared_ptr<Texture> Device::CreateTexture( const D3D12_RESOURCE_DESC& resourceDesc,
                                                const D3D12_CLEAR_VALUE* clearValue, TextureUsage textureUsage )
{
    std::shared_ptr<Texture> texture =
        std::make_shared<MakeTexture>( shared_from_this(), resourceDesc, clearValue, textureUsage );

    return texture;
}

std::shared_ptr<Texture> Device::CreateTexture( Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                                                const D3D12_CLEAR_VALUE* clearValue, TextureUsage textureUsage )
{
    std::shared_ptr<Texture> texture =
        std::make_shared<MakeTexture>( shared_from_this(), resource, clearValue, textureUsage );

    return texture;
}

std::shared_ptr<CommandList> Device::CreateCommandList( D3D12_COMMAND_LIST_TYPE type )
{
    std::shared_ptr<CommandList> commandList = std::make_shared<MakeCommandList>( shared_from_this(), type );

    return commandList;
}

std::shared_ptr<dx12lib::RootSignature>
    dx12lib::Device::CreateRootSignature( const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc,
                                          D3D_ROOT_SIGNATURE_VERSION        rootSignatureVersion )
{
    std::shared_ptr<RootSignature> rootSignature =
        std::make_shared<MakeRootSignature>( shared_from_this(), rootSignatureDesc, rootSignatureVersion );

    return rootSignature;
}
