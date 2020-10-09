#include "DX12LibPCH.h"

#include <dx12lib/Adapter.h>
#include <dx12lib/DescriptorAllocator.h>
#include <dx12lib/Device.h>

using namespace dx12lib;

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
    // Create command queues.
}