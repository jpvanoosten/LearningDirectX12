#include <Tutorial5.h>

#include <SceneVisitor.h>

#include <GameFramework/Window.h>

#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/Device.h>
#include <dx12lib/GUI.h>
#include <dx12lib/Helpers.h>
#include <dx12lib/Material.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/Scene.h>
#include <dx12lib/SceneNode.h>
#include <dx12lib/SwapChain.h>
#include <dx12lib/Texture.h>

#include <assimp/DefaultLogger.hpp>

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <d3dx12.h>

#include <regex>

using namespace Microsoft::WRL;
using namespace DirectX;
using namespace dx12lib;

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
, m_Viewport( CD3DX12_VIEWPORT( 0.0f, 0.0f, static_cast<float>( width ), static_cast<float>( height ) ) )
, m_CameraController( m_Camera )
, m_Fullscreen( false )
, m_AllowFullscreenToggle( true )
, m_Width( width )
, m_Height( height )
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

    XMVECTOR cameraPos    = XMVectorSet( 0, 1, 0, 1 );
    XMVECTOR cameraTarget = XMVectorSet( 1, 0, 0, 1 );
    XMVECTOR cameraUp     = XMVectorSet( 0, 1, 0, 0 );

    m_Camera.set_LookAt( cameraPos, cameraTarget, cameraUp );
    m_Camera.set_Projection( 45.0f, width / (float)height, 0.1f, 1000.0f );

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

    // Load a scene, passing an optional function object for receiving loading progress events.
    m_LoadingText = "Loading Assets/Models/crytek-sponza/sponza_nobanner.obj ...";
    m_Scene       = commandList->LoadSceneFromFile( L"Assets/Models/crytek-sponza/sponza_nobanner.obj",
                                              std::bind( &Tutorial5::LoadingProgress, this, _1 ) );

    m_Scene->GetRootNode()->SetLocalTransform( XMMatrixScaling( 0.01f, 0.01f, 0.01f ) );

    commandQueue.ExecuteCommandList( commandList );

    // Load the vertex shader.
    ComPtr<ID3DBlob> vertexShaderBlob;
    ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/05-Models/VertexShader.cso", &vertexShaderBlob ) );

    // Load the pixel shader.
    ComPtr<ID3DBlob> pixelShaderBlob;
    ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/05-Models/PixelShader.cso", &pixelShaderBlob ) );

    // Create a root signature.
    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                    D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    CD3DX12_DESCRIPTOR_RANGE1 descriptorRage( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2 );

    CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
    rootParameters[RootParameters::MatricesCB].InitAsConstantBufferView( 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                                                                         D3D12_SHADER_VISIBILITY_VERTEX );
    rootParameters[RootParameters::MaterialCB].InitAsConstantBufferView( 0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                                                                         D3D12_SHADER_VISIBILITY_PIXEL );
    rootParameters[RootParameters::LightPropertiesCB].InitAsConstants( sizeof( LightProperties ) / 4, 1, 0,
                                                                       D3D12_SHADER_VISIBILITY_PIXEL );
    rootParameters[RootParameters::PointLights].InitAsShaderResourceView( 0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                                                                          D3D12_SHADER_VISIBILITY_PIXEL );
    rootParameters[RootParameters::SpotLights].InitAsShaderResourceView( 1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                                                                         D3D12_SHADER_VISIBILITY_PIXEL );
    rootParameters[RootParameters::Textures].InitAsDescriptorTable( 1, &descriptorRage, D3D12_SHADER_VISIBILITY_PIXEL );

    CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler( 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR );
    CD3DX12_STATIC_SAMPLER_DESC anisotropicSampler( 0, D3D12_FILTER_ANISOTROPIC );

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1( RootParameters::NumRootParameters, rootParameters, 1, &linearRepeatSampler,
                                       rootSignatureFlags );

    m_RootSignature = m_Device->CreateRootSignature( rootSignatureDescription.Desc_1_1 );

    // Setup the pipeline state.
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC           SampleDesc;
    } pipelineStateStream;

    // Create a color buffer with sRGB for gamma correction.
    DXGI_FORMAT backBufferFormat  = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

    // Check the best multisample quality level that can be used for the given back buffer format.
    DXGI_SAMPLE_DESC sampleDesc = m_Device->GetMultisampleQualityLevels( backBufferFormat );

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets      = 1;
    rtvFormats.RTFormats[0]          = backBufferFormat;

    pipelineStateStream.pRootSignature        = m_RootSignature->GetD3D12RootSignature().Get();
    pipelineStateStream.InputLayout           = Mesh::Vertex::InputLayout;
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE( vertexShaderBlob.Get() );
    pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE( pixelShaderBlob.Get() );
    pipelineStateStream.DSVFormat             = depthBufferFormat;
    pipelineStateStream.RTVFormats            = rtvFormats;
    pipelineStateStream.SampleDesc            = sampleDesc;

    m_PipelineState = m_Device->CreatePipelineStateObject( pipelineStateStream );

    // Create an off-screen render target with a single color buffer and a depth buffer.
    auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D( backBufferFormat, m_Width, m_Height, 1, 1, sampleDesc.Count,
                                                   sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET );
    D3D12_CLEAR_VALUE colorClearValue;
    colorClearValue.Format   = colorDesc.Format;
    colorClearValue.Color[0] = 0.4f;
    colorClearValue.Color[1] = 0.6f;
    colorClearValue.Color[2] = 0.9f;
    colorClearValue.Color[3] = 1.0f;

    auto colorTexture = m_Device->CreateTexture( colorDesc, TextureUsage::RenderTarget, &colorClearValue );
    colorTexture->SetName( L"Color Render Target" );

    // Create a depth buffer.
    auto depthDesc = CD3DX12_RESOURCE_DESC::Tex2D( depthBufferFormat, m_Width, m_Height, 1, 1, sampleDesc.Count,
                                                   sampleDesc.Quality, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL );
    D3D12_CLEAR_VALUE depthClearValue;
    depthClearValue.Format       = depthDesc.Format;
    depthClearValue.DepthStencil = { 1.0f, 0 };

    auto depthTexture = m_Device->CreateTexture( depthDesc, TextureUsage::Depth, &depthClearValue );
    depthTexture->SetName( L"Depth Render Target" );

    // Attach the textures to the render target.
    m_RenderTarget.AttachTexture( AttachmentPoint::Color0, colorTexture );
    m_RenderTarget.AttachTexture( AttachmentPoint::DepthStencil, depthTexture );

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

    m_SwapChain = m_Device->CreateSwapChain( m_Window->GetWindowHandle(), DXGI_FORMAT_R8G8B8A8_UNORM );
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

    m_SwapChain->WaitForSwapChain();

    // Process keyboard, mouse, and pad input.
    GameFramework::Get().ProcessInput();
    m_CameraController.Update( e );

    XMMATRIX viewMatrix = m_Camera.get_ViewMatrix();

    const int numPointLights = 4;
    const int numSpotLights  = 4;

    static const XMVECTORF32 LightColors[] = { Colors::White, Colors::Orange, Colors::Yellow, Colors::Green,
                                               Colors::Blue,  Colors::Indigo, Colors::Violet, Colors::White };

    static float lightAnimTime = 0.0f;
    if ( m_AnimateLights )
    {
        lightAnimTime += static_cast<float>( e.DeltaTime ) * 0.5f * XM_PI;
    }

    const float radius  = 8.0f;
    const float offset  = 2.0f * XM_PI / numPointLights;
    const float offset2 = offset + ( offset / 2.0f );

    // Setup the light buffers.
    m_PointLights.resize( numPointLights );
    for ( int i = 0; i < numPointLights; ++i )
    {
        PointLight& l = m_PointLights[i];

        l.PositionWS        = { static_cast<float>( std::sin( lightAnimTime + offset * i ) ) * radius, 9.0f,
                         static_cast<float>( std::cos( lightAnimTime + offset * i ) ) * radius, 1.0f };
        XMVECTOR positionWS = XMLoadFloat4( &l.PositionWS );
        XMVECTOR positionVS = XMVector3TransformCoord( positionWS, viewMatrix );
        XMStoreFloat4( &l.PositionVS, positionVS );

        l.Color                = XMFLOAT4( LightColors[i] );
        l.ConstantAttenuation  = 1.0f;
        l.LinearAttenuation    = 0.08f;
        l.QuadraticAttenuation = 0.0f;
    }

    m_SpotLights.resize( numSpotLights );
    for ( int i = 0; i < numSpotLights; ++i )
    {
        SpotLight& l = m_SpotLights[i];

        l.PositionWS        = { static_cast<float>( std::sin( lightAnimTime + offset * i + offset2 ) ) * radius, 9.0f,
                         static_cast<float>( std::cos( lightAnimTime + offset * i + offset2 ) ) * radius, 1.0f };
        XMVECTOR positionWS = XMLoadFloat4( &l.PositionWS );
        XMVECTOR positionVS = XMVector3TransformCoord( positionWS, viewMatrix );
        XMStoreFloat4( &l.PositionVS, positionVS );

        XMVECTOR directionWS = XMVector3Normalize( XMVectorSetW( XMVectorNegate( positionWS ), 0 ) );
        XMVECTOR directionVS = XMVector3Normalize( XMVector3TransformNormal( directionWS, viewMatrix ) );
        XMStoreFloat4( &l.DirectionWS, directionWS );
        XMStoreFloat4( &l.DirectionVS, directionVS );

        l.Color                = XMFLOAT4( LightColors[numPointLights + i] );
        l.SpotAngle            = XMConvertToRadians( 45.0f );
        l.ConstantAttenuation  = 1.0f;
        l.LinearAttenuation    = 0.08f;
        l.QuadraticAttenuation = 0.0f;
    }

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

    const auto& renderTarget = m_IsLoading ? m_SwapChain->GetRenderTarget() : m_RenderTarget;

    if ( m_IsLoading )
    {
        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
        commandList->ClearTexture( renderTarget.GetTexture( AttachmentPoint::Color0 ), clearColor );

        // TODO: Render a loading screen.
    }
    else
    {
        SceneVisitor visitor( *commandList, m_Camera );

        // Clear the render targets.
        {
            FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

            commandList->ClearTexture( renderTarget.GetTexture( AttachmentPoint::Color0 ), clearColor );
            commandList->ClearDepthStencilTexture( renderTarget.GetTexture( AttachmentPoint::DepthStencil ),
                                                   D3D12_CLEAR_FLAG_DEPTH );
        }

        commandList->SetPipelineState( m_PipelineState );
        commandList->SetGraphicsRootSignature( m_RootSignature );

        // Upload lights
        LightProperties lightProps;
        lightProps.NumPointLights = static_cast<uint32_t>( m_PointLights.size() );
        lightProps.NumSpotLights  = static_cast<uint32_t>( m_SpotLights.size() );

        commandList->SetGraphics32BitConstants( RootParameters::LightPropertiesCB, lightProps );
        commandList->SetGraphicsDynamicStructuredBuffer( RootParameters::PointLights, m_PointLights );
        commandList->SetGraphicsDynamicStructuredBuffer( RootParameters::SpotLights, m_SpotLights );

        commandList->SetViewport( m_Viewport );
        commandList->SetScissorRect( m_ScissorRect );

        commandList->SetRenderTarget( m_RenderTarget );

        // Render the model.
        m_Scene->Accept( visitor );

        // Resolve the MSAA render target to the swapchain's backbuffer.
        auto swapChainBackBuffer = m_SwapChain->GetRenderTarget().GetTexture( AttachmentPoint::Color0 );
        auto msaaRenderTarget    = m_RenderTarget.GetTexture( AttachmentPoint::Color0 );

        commandList->ResolveSubresource( swapChainBackBuffer, msaaRenderTarget );
    }

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

    if ( !ImGui::GetIO().WantCaptureMouse ) {}
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
