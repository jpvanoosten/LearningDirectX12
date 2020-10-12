#include "DX12LibPCH.h"

#include <dx12lib/Adapter.h>
#include <dx12lib/ByteAddressBuffer.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/DescriptorAllocator.h>
#include <dx12lib/Device.h>
#include <dx12lib/SwapChain.h>

using namespace dx12lib;

// Adapter for std::make_shared
class MakeByteAddressBuffer : public ByteAddressBuffer
{
public:
    MakeByteAddressBuffer(std::shared_ptr<Device> device, const D3D12_RESOURCE_DESC& desc )
    : ByteAddressBuffer(device, desc )
    {}

    virtual ~MakeByteAddressBuffer() {}
};

// Adapter for std::make_unique
class MakeDescriptorAllocator : public DescriptorAllocator
{
public:
    MakeDescriptorAllocator( std::shared_ptr<Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type,
                             uint32_t numDescriptorsPerHeap = 256 )
    : DescriptorAllocator(device, type, numDescriptorsPerHeap )
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

std::shared_ptr<Device> Device::Create( std::shared_ptr<Adapter> adapter )
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

void Device::ReleaseStaleDescriptors( uint64_t finishedFrame )
{
    for ( int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i )
    { m_DescriptorAllocators[i]->ReleaseStaleDescriptors( finishedFrame ); }
}

std::shared_ptr<SwapChain> Device::CreateSwapChain( HWND hWnd )
{
    std::shared_ptr<SwapChain> swapChain;
    swapChain = std::make_shared<MakeSwapChain>( shared_from_this(), hWnd );

    return swapChain;
}

std::shared_ptr<ByteAddressBuffer>
    Device::CreateByteAddressBuffer( const D3D12_RESOURCE_DESC& resDesc )
{
    std::shared_ptr<ByteAddressBuffer> buffer =
        std::make_shared<MakeByteAddressBuffer>( shared_from_this(), resDesc );

    return buffer;
}
