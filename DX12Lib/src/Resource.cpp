#include <DX12LibPCH.h>

#include <Resource.h>

#include <Application.h>
#include <ResourceStateTracker.h>

Resource::Resource(const std::wstring& name)
    : m_ResourceName(name)
{}

Resource::Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const std::wstring& name )
	: m_d3d12Resource(resource)
{
	SetName(name);
}

Resource::Resource(const Resource& copy)
	: m_d3d12Resource(copy.m_d3d12Resource)
	, m_ResourceName(copy.m_ResourceName)
{}


Resource::~Resource()
{
}

void Resource::SetD3D12Resource(Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource)
{
    m_d3d12Resource = d3d12Resource;
    SetName(m_ResourceName);
}

void Resource::SetName(const std::wstring& name)
{
    m_ResourceName = name;
    if (m_d3d12Resource && !m_ResourceName.empty())
    {
        m_d3d12Resource->SetName(m_ResourceName.c_str());
    }
}