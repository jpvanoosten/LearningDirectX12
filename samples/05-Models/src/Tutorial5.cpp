#include <Tutorial5.h>

#include <GameFramework/Window.h>

#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/Device.h>
#include <dx12lib/GUI.h>
#include <dx12lib/Scene.h>
#include <dx12lib/SwapChain.h>

#include <DirectXMath.h>
#include <assimp/DefaultLogger.hpp>

#include <regex>

using namespace DirectX;
using namespace dx12lib;

// Matrices to be sent to the vertex shader.
struct Matrices
{
    XMMATRIX ModelMatrix;
    XMMATRIX ModelViewMatrix;
    XMMATRIX InverseTransposeModelViewMatrix;
    XMMATRIX ModelViewProjectionMatrix;
};

// Light properties for the pixel shader.
struct LightProperties
{
    uint32_t NumPointLights;
    uint32_t NumSpotLights;
};

// An enum for root signature parameters.
// I'm not using scoped enums to avoid the explicit cast that would be required
// to use these as root indices in the root signature.
enum RootParameters
{
    MatricesCB,         // ConstantBuffer<Matrices> MatCB : register(b0);
    MaterialCB,         // ConstantBuffer<Material> MaterialCB : register( b0, space1 );
    LightPropertiesCB,  // ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );
    PointLights,        // StructuredBuffer<PointLight> PointLights : register( t0 );
    SpotLights,         // StructuredBuffer<SpotLight> SpotLights : register( t1 );
    Textures,           // Texture2D DiffuseTexture : register( t2 );
    NumRootParameters
};

// A regular express used to extract the relavent part of an Assimp log message.
static std::regex gs_AssimpLogRegex( R"((?:Debug|Info|Warn|Error),\s*(.*)\n)" );

template<spdlog::level::level_enum lvl>
class LogStream : public Assimp::LogStream
{
public:
    LogStream( Logger logger )
    : m_Logger( logger )
    {}

    virtual void write( const char* message ) override
    {
        // Extract just the part of the message we want to log with spdlog.
        std::cmatch match;
        std::regex_search( message, match, gs_AssimpLogRegex );

        if ( match.size() > 1 )
        {
            m_Logger->log( lvl, match.str( 1 ) );
        }
    }

private:
    Logger m_Logger;
};

using DebugLogStream = LogStream<spdlog::level::debug>;
using InfoLogStream  = LogStream<spdlog::level::info>;
using WarnLogStream  = LogStream<spdlog::level::warn>;
using ErrorLogStream = LogStream<spdlog::level::err>;

Tutorial5::Tutorial5( const std::wstring& name, int width, int height, bool vSync )
: m_ScissorRect { 0, 0, LONG_MAX, LONG_MAX }
, m_CameraController(m_Camera)
, m_Fullscreen( false )
, m_AllowFullscreenToggle( true )
, m_IsLoading( true )
{
#if _DEBUG
    Device::EnableDebugLayer();
#endif
    // Create a spdlog logger for the demo.
    m_Logger = GameFramework::Get().CreateLogger( "05-Models" );
    // Create logger for assimp.
    auto assimpLogger = GameFramework::Get().CreateLogger( "ASSIMP" );

    // Setup assimp logging.
#if defined( _DEBUG )
    Assimp::Logger::LogSeverity logSeverity = Assimp::Logger::VERBOSE;
#else
    Assimp::Logger::LogSeverity logSeverity = Assimp::Logger::NORMAL;
#endif
    // Create a default logger with no streams (we'll supply our own).
    auto assimpDefaultLogger = Assimp::DefaultLogger::create( "", logSeverity, 0 );
    assimpDefaultLogger->attachStream( new DebugLogStream( assimpLogger ), Assimp::Logger::Debugging );
    assimpDefaultLogger->attachStream( new InfoLogStream( assimpLogger ), Assimp::Logger::Info );
    assimpDefaultLogger->attachStream( new WarnLogStream( assimpLogger ), Assimp::Logger::Warn );
    assimpDefaultLogger->attachStream( new ErrorLogStream( assimpLogger ), Assimp::Logger::Err );

    // Create  window for rendering to.
    m_Window = GameFramework::Get().CreateWindow( name, width, height );

    m_Window->Update += UpdateEvent::slot( &Tutorial5::OnUpdate, this );
    m_Window->Resize += ResizeEvent::slot( &Tutorial5::OnResize, this );
    m_Window->DPIScaleChanged += DPIScaleEvent::slot( &Tutorial5::OnDPIScaleChanged, this );
    m_Window->KeyPressed += KeyboardEvent::slot( &Tutorial5::OnKeyPressed, this );
    m_Window->KeyReleased += KeyboardEvent::slot( &Tutorial5::OnKeyReleased, this );
    m_Window->MouseMoved += MouseMotionEvent::slot( &Tutorial5::OnMouseMoved, this );
    m_Window->MouseWheel += MouseWheelEvent::slot( &Tutorial5::OnMouseWheel, this );

    XMVECTOR cameraPos    = XMVectorSet( 0, 5, -20, 1 );
    XMVECTOR cameraTarget = XMVectorSet( 0, 5, 0, 1 );
    XMVECTOR cameraUp     = XMVectorSet( 0, 1, 0, 0 );

    m_Camera.set_LookAt( cameraPos, cameraTarget, cameraUp );
    m_Camera.set_Projection( 45.0f, width / (float)height, 0.1f, 100.0f );

    m_pAlignedCameraData = (CameraData*)_aligned_malloc( sizeof( CameraData ), 16 );

    m_pAlignedCameraData->m_InitialCamPos = m_Camera.get_Translation();
    m_pAlignedCameraData->m_InitialCamRot = m_Camera.get_Rotation();
}

Tutorial5::~Tutorial5()
{
    Assimp::DefaultLogger::kill();

    _aligned_free( m_pAlignedCameraData );
}

uint32_t Tutorial5::Run()
{

    LoadContent();

    m_Window->Show();

    auto retCode = GameFramework::Get().Run();

    // Make sure the loading task is finished
    m_LoadingTask.get();

    UnloadContent();

    return retCode;
}

bool Tutorial5::LoadingProgress( float loadingProgress )
{
    m_LoadingProgress = loadingProgress;

    return true;
}

bool Tutorial5::LoadAssets()
{
    using namespace std::placeholders;  // For _1 used to denote a placeholder argument for std::bind.

    auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );
    auto  commandList  = commandQueue.GetCommandList();

    // Load a scene:
    m_LoadingText = "Loading Assets/Models/crytek-sponza/sponza_nobanner.obj ...";
    m_Scene       = commandList->LoadSceneFromFile( L"Assets/Models/crytek-sponza/sponza_nobanner.obj",
                                              std::bind( &Tutorial5::LoadingProgress, this, _1 ) );

    commandQueue.ExecuteCommandList( commandList );

    // Ensure that the scene is completely loaded before rendering.
    commandQueue.Flush();

    // Loading is finished.
    m_IsLoading = false;

    return true;
}

void Tutorial5::LoadContent()
{
    m_Device = Device::Create();
    m_Logger->info( L"Device created: {}", m_Device->GetDescription() );

    m_SwapChain = m_Device->CreateSwapChain( m_Window->GetWindowHandle() );
    m_GUI       = m_Device->CreateGUI( m_Window->GetWindowHandle(), m_SwapChain->GetRenderTarget() );

    // This magic here allows ImGui to process window messages.
    GameFramework::Get().WndProcHandler += WndProcEvent::slot( &GUI::WndProcHandler, m_GUI );

    // Start the loading task.
    m_LoadingTask = std::async( std::launch::async, std::bind( &Tutorial5::LoadAssets, this ) );
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
        ::swprintf_s( buffer, L"Models [FPS: %f]", fps );
        m_Window->SetWindowTitle( buffer );

        frameCount = 0;
        totalTime  = 0.0;
    }

    // Process keyboard, mouse, and pad input.
    GameFramework::Get().ProcessInput();
    m_CameraController.Update( e );

    OnRender();
}

void Tutorial5::OnResize( ResizeEventArgs& e )
{
    m_Logger->info( "Resize: {}, {}", e.Width, e.Height );

    // This is required for gainput to normalize mouse movement.
    GameFramework::Get().SetDisplaySize( e.Width, e.Height );

    auto width  = std::max( 1, e.Width );
    auto height = std::max( 1, e.Height );

    m_SwapChain->Resize( width, height );
}

void Tutorial5::OnRender()
{
    m_Window->SetFullscreen( m_Fullscreen );

    auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
    auto  commandList  = commandQueue.GetCommandList();

    auto& renderTarget = m_SwapChain->GetRenderTarget();

    const FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
    commandList->ClearTexture( renderTarget.GetTexture( AttachmentPoint::Color0 ), clearColor );

    OnGUI( commandList, renderTarget );

    commandQueue.ExecuteCommandList( commandList );

    m_SwapChain->Present();
}

void Tutorial5::OnKeyPressed( KeyEventArgs& e )
{
    if ( !ImGui::GetIO().WantCaptureKeyboard )
    {
        switch ( e.Key )
        {
        case KeyCode::Escape:
            GameFramework::Get().Stop();
            break;
        case KeyCode::Enter:
            if ( e.Alt )
            {
            case KeyCode::F11:
                if ( m_AllowFullscreenToggle )
                {
                    m_Fullscreen = !m_Fullscreen;  // Defer window resizing until OnUpdate();
                    // Prevent the key repeat to cause multiple resizes.
                    m_AllowFullscreenToggle = false;
                }
                break;
            }
        case KeyCode::V:
            m_SwapChain->ToggleVSync();
            break;
        case KeyCode::R:
            // Reset camera transform
            m_Camera.set_Translation( m_pAlignedCameraData->m_InitialCamPos );
            m_Camera.set_Rotation( m_pAlignedCameraData->m_InitialCamRot );
            break;
        }
    }
}

void Tutorial5::OnKeyReleased( KeyEventArgs& e )
{
    if ( !ImGui::GetIO().WantCaptureKeyboard )
    {
        switch ( e.Key )
        {
        case KeyCode::Enter:
            if ( e.Alt )
            {
            case KeyCode::F11:
                m_AllowFullscreenToggle = true;
            }
            break;
        }
    }
}

void Tutorial5::OnMouseMoved( MouseMotionEventArgs& e )
{
    const float mouseSpeed = 0.1f;

    if ( !ImGui::GetIO().WantCaptureMouse )
    {
    }
}

void Tutorial5::OnMouseWheel( MouseWheelEventArgs& e )
{
    if ( !ImGui::GetIO().WantCaptureMouse )
    {
        auto fov = m_Camera.get_FoV();

        fov -= e.WheelDelta;
        fov = std::clamp( fov, 12.0f, 90.0f );

        m_Camera.set_FoV( fov );

        m_Logger->info( "FoV: {:.7}", fov );
    }
}

void Tutorial5::OnDPIScaleChanged( DPIScaleEventArgs& e )
{
    m_GUI->SetScaling( e.DPIScale );
}

void Tutorial5::OnGUI( const std::shared_ptr<CommandList>& commandList, const RenderTarget& renderTarget )
{
    m_GUI->NewFrame();

    if ( m_IsLoading )
    {
        // Show a progress bar.
        ImGui::SetNextWindowPos( ImVec2( m_Window->GetClientWidth() / 2.0f, m_Window->GetClientHeight() / 2.0f ), 0,
                                 ImVec2( 0.5, 0.5 ) );
        ImGui::SetNextWindowSize( ImVec2( m_Window->GetClientWidth() / 2.0f, 0 ) );

        ImGui::Begin( "Loading", nullptr,
                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                          ImGuiWindowFlags_NoScrollbar );
        ImGui::ProgressBar( m_LoadingProgress );
        ImGui::Text( m_LoadingText.c_str() );

        ImGui::End();
    }

    m_GUI->Render( commandList, renderTarget );
}
