#include <DirectX12TemplatePCH.h>
#include <DX12FrameworkView.h>

using namespace Windows::Foundation;                    // For TypedEventHandler
using namespace Windows::ApplicationModel::Core;        // For CoreApplicationView
using namespace Windows::ApplicationModel::Activation;  // For IActivatedEventArgs
using namespace Windows::UI::Core;                      // For CoreWindow and events.


using namespace Windows::UI::Core;

DX12FrameworkView::DX12FrameworkView( DX12Game* pGame)
    : m_pGame(pGame)
{}

void DX12FrameworkView::Initialize( CoreApplicationView^ applicationView )
{
    applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &DX12FrameworkView::OnActivated);
}

void DX12FrameworkView::SetWindow( CoreWindow^ window )
{

}

void DX12FrameworkView::Load( Platform::String^ entryPoint)
{

}

void DX12FrameworkView::Run()
{

}

void DX12FrameworkView::Uninitialize()
{

}

void DX12FrameworkView::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    CoreWindow::GetForCurrentThread()->Activate();
}

void DX12FrameworkView::OnKeyDown(CoreWindow^ window, KeyEventArgs^ keyEventArgs)
{

}

void DX12FrameworkView::OnKeyUp(CoreWindow^ window, KeyEventArgs^ keyEventArgs)
{

}

void DX12FrameworkView::OnClosed(CoreWindow^ window, CoreWindowEventArgs^ keyEventArgs)
{

}
