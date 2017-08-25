#include <DirectX12TemplatePCH.h>
#include <DX12FrameworkViewSource.h>
#include <DX12FrameworkView.h>

using namespace Windows::ApplicationModel::Core;

DX12FrameworkViewSource::DX12FrameworkViewSource( size_t pGame )
    : m_pGame( reinterpret_cast<DX12Game*>( pGame ) )
{}

IFrameworkView^ DX12FrameworkViewSource::CreateView()
{
    return ref new DX12FrameworkView(m_pGame);
}
