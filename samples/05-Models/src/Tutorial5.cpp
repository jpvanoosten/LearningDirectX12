#include <Tutorial5.h>

#include <GameFramework/Window.h>

#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/Device.h>
#include <dx12lib/GUI.h>
#include <dx12lib/SwapChain.h>

#include <DirectXMath.h>

using namespace DirectX;
using namespace dx12lib;

Tutorial5::Tutorial5( const std::wstring& name, int width, int height, bool vSync )
: m_ScissorRect { 0, 0, LONG_MAX, LONG_MAX }
{
#if _DEBUG
    Device::EnableDebugLayer();
#endif

    m_Logger = GameFramework::Get().CreateLogger( "Models" );
    m_Window = GameFramework::Get().CreateWindow( name, width, height );

    m_Window->Update += UpdateEvent::slot( &Tutorial5::OnUpdate, this );
    m_Window->Resize += ResizeEvent::slot( &Tutorial5::OnResize, this );
    m_Window->DPIScaleChanged += DPIScaleEvent::slot( &Tutorial5::OnDPIScaleChanged, this );

    XMVECTOR cameraPos    = XMVectorSet( 0, 5, -20, 1 );
    XMVECTOR cameraTarget = XMVectorSet( 0, 5, 0, 1 );
    XMVECTOR cameraUp     = XMVectorSet( 0, 1, 0, 0 );

    m_Camera.set_LookAt( cameraPos, cameraTarget, cameraUp );
    m_Camera.set_Projection( 45.0f, width / (float)height, 0.1f, 100.0f );
}

uint32_t Tutorial5::Run()
{

    LoadContent();

    m_Window->Show();

    auto retCode = GameFramework::Get().Run();

    UnloadContent();

    return retCode;
}

bool Tutorial5::LoadContent()
{
    m_Device    = Device::Create();
    m_SwapChain = m_Device->CreateSwapChain( m_Window->GetWindowHandle() );
    m_GUI       = m_Device->CreateGUI( m_Window->GetWindowHandle(), m_SwapChain->GetRenderTarget() );

    // This magic here allows ImGui to process window messages.
    GameFramework::Get().WndProcHandler += WndProcEvent::slot( &GUI::WndProcHandler, m_GUI );

    return true;
}

void Tutorial5::UnloadContent() {}

void Tutorial5::OnUpdate( UpdateEventArgs& e )
{
    static uint64_t frameCount = 0;
    static double   totalTime  = 0.0;

    totalTime += e.DeltaTime;
    frameCount++;

    if ( totalTime > 1.0 )
    {
        double fps = frameCount / totalTime;

        m_Logger->info( "FPS: {:.7}", fps );

        wchar_t buffer[512];
        ::swprintf_s( buffer, L"HDR [FPS: %f]", fps );
        m_Window->SetWindowTitle( buffer );

        frameCount = 0;
        totalTime  = 0.0;
    }

    OnRender();
}

void Tutorial5::OnResize( ResizeEventArgs& e )
{
    auto width  = std::max( 1, e.Width );
    auto height = std::max( 1, e.Height );

    m_SwapChain->Resize( width, height );
}

void Tutorial5::OnRender()
{
    auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
    auto  commandList  = commandQueue.GetCommandList();

    auto& renderTarget = m_SwapChain->GetRenderTarget();

    const FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
    commandList->ClearTexture( renderTarget.GetTexture( AttachmentPoint::Color0 ), clearColor );

    OnGUI( commandList, renderTarget );

    commandQueue.ExecuteCommandList( commandList );

    m_SwapChain->Present();
}

void Tutorial5::OnDPIScaleChanged( DPIScaleEventArgs& e ) 
{
    m_GUI->SetScaling( e.DPIScale );
}

void Tutorial5::OnGUI( const std::shared_ptr<CommandList>& commandList, const RenderTarget& renderTarget )
{
    m_GUI->NewFrame();

    m_GUI->Render( commandList, renderTarget );
}
