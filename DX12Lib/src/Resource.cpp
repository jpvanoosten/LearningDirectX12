#include <DX12LibPCH.h>

#include <Resource.h>

#include <Application.h>
#include <ResourceStateTracker.h>

Resource::Resource(const std::wstring& name)
    : m_ResourceName(name)
{}

Resource::~Resource()
{
    ResourceStateTracker::RemoveGlobalResourceState(m_d3d12Resource.Get());
}

void Resource::SetD3D12Resource(Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource, D3D12_RESOURCE_STATES initialResourceState)
{
    ResourceStateTracker::RemoveGlobalResourceState(m_d3d12Resource.Get());
    ResourceStateTracker::AddGlobalResourceState(d3d12Resource.Get(), initialResourceState);

    m_d3d12Resource = d3d12Resource;
    SetName(m_ResourceName);
}

void Resource::SetName(const std::wstring& name)
{
    m_ResourceName = name;
    if (m_d3d12Resource)
    {
        m_d3d12Resource->SetName(m_ResourceName.c_str());
    }
}