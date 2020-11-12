#include <Tutorial4.h>

#include <Light.h>
#include <SceneVisitor.h>

#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/Device.h>
#include <dx12lib/GUI.h>
#include <dx12lib/Helpers.h>
#include <dx12lib/Material.h>
#include <dx12lib/Mesh.h>
#include <dx12lib/PipelineStateObject.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/Scene.h>
#include <dx12lib/SwapChain.h>
#include <dx12lib/Texture.h>

#include <GameFramework/Window.h>

#include <wrl/client.h>
using namespace Microsoft::WRL;

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

using namespace dx12lib;
using namespace DirectX;

#include <algorithm>  // For std::min, std::max, and std::clamp.

struct Mat
{
    XMMATRIX ModelMatrix;
    XMMATRIX ModelViewMatrix;
    XMMATRIX InverseTransposeModelViewMatrix;
    XMMATRIX ModelViewProjectionMatrix;
};

struct LightProperties
{
    uint32_t NumPointLights;
    uint32_t NumSpotLights;
};

enum TonemapMethod : uint32_t
{
    TM_Linear,
    TM_Reinhard,
    TM_ReinhardSq,
    TM_ACESFilmic,
};

struct TonemapParameters
{
    TonemapParameters()
    : TonemapMethod( TM_Reinhard )
    , Exposure( 0.0f )
    , MaxLuminance( 1.0f )
    , K( 1.0f )
    , A( 0.22f )
    , B( 0.3f )
    , C( 0.1f )
    , D( 0.2f )
    , E( 0.01f )
    , F( 0.3f )
    , LinearWhite( 11.2f )
    , Gamma( 2.2f )
    {}

    // The method to use to perform tonemapping.
    TonemapMethod TonemapMethod;
    // Exposure should be expressed as a relative exposure value (-2, -1, 0, +1, +2 )
    float Exposure;

    // The maximum luminance to use for linear tonemapping.
    float MaxLuminance;

    // Reinhard constant. Generally this is 1.0.
    float K;

    // ACES Filmic parameters
    // See: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/142
    float A;  // Shoulder strength
    float B;  // Linear strength
    float C;  // Linear angle
    float D;  // Toe strength
    float E;  // Toe Numerator
    float F;  // Toe denominator
    // Note E/F = Toe angle.
    float LinearWhite;
    float Gamma;
};

TonemapParameters g_TonemapParameters;

// An enum for root signature parameters.
// I'm not using scoped enums to avoid the explicit cast that would be required
// to use these as root indices in the root signature.
enum RootParameters
{
    MatricesCB,         // ConstantBuffer<Mat> MatCB : register(b0);
    MaterialCB,         // ConstantBuffer<Material> MaterialCB : register( b0, space1 );
    LightPropertiesCB,  // ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );
    PointLights,        // StructuredBuffer<PointLight> PointLights : register( t0 );
    SpotLights,         // StructuredBuffer<SpotLight> SpotLights : register( t1 );
    Textures,           // Texture2D DiffuseTexture : register( t2 );
    NumRootParameters
};

// Builds a look-at (world) matrix from a point, up and direction vectors.
XMMATRIX XM_CALLCONV LookAtMatrix( FXMVECTOR Position, FXMVECTOR Direction, FXMVECTOR Up )
{
    assert( !XMVector3Equal( Direction, XMVectorZero() ) );
    assert( !XMVector3IsInfinite( Direction ) );
    assert( !XMVector3Equal( Up, XMVectorZero() ) );
    assert( !XMVector3IsInfinite( Up ) );

    XMVECTOR R2 = XMVector3Normalize( Direction );

    XMVECTOR R0 = XMVector3Cross( Up, R2 );
    R0          = XMVector3Normalize( R0 );

    XMVECTOR R1 = XMVector3Cross( R2, R0 );

    XMMATRIX M( R0, R1, R2, Position );

    return M;
}

Tutorial4::Tutorial4( const std::wstring& name, int width, int height, bool vSync )
: m_ScissorRect( CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ) )
, m_Forward( 0 )
, m_Backward( 0 )
, m_Left( 0 )
, m_Right( 0 )
, m_Up( 0 )
, m_Down( 0 )
, m_Pitch( 0 )
, m_Yaw( 0 )
, m_AnimateLights( false )
, m_Shift( false )
, m_Width( width )
, m_Height( height )
, m_VSync( vSync )
, m_Fullscreen( false )
, m_RenderScale( 1.0f )
{
    m_Logger = GameFramework::Get().CreateLogger( "HDR" );
    m_Window = GameFramework::Get().CreateWindow( name, width, height );

    m_Window->Update += UpdateEvent::slot( &Tutorial4::OnUpdate, this );
    m_Window->KeyPressed += KeyboardEvent::slot( &Tutorial4::OnKeyPressed, this );
    m_Window->KeyReleased += KeyboardEvent::slot( &Tutorial4::OnKeyReleased, this );
    m_Window->MouseMoved += MouseMotionEvent::slot( &Tutorial4::OnMouseMoved, this );
    m_Window->MouseWheel += MouseWheelEvent::slot( &Tutorial4::OnMouseWheel, this );
    m_Window->Resize += ResizeEvent::slot( &Tutorial4::OnResize, this );
    m_Window->DPIScaleChanged += DPIScaleEvent::slot( &Tutorial4::OnDPIScaleChanged, this );

    XMVECTOR cameraPos    = XMVectorSet( 0, 5, -20, 1 );
    XMVECTOR cameraTarget = XMVectorSet( 0, 5, 0, 1 );
    XMVECTOR cameraUp     = XMVectorSet( 0, 1, 0, 0 );

    m_Camera.set_LookAt( cameraPos, cameraTarget, cameraUp );
    m_Camera.set_Projection( 45.0f, width / (float)height, 0.1f, 100.0f );

    m_pAlignedCameraData = (CameraData*)_aligned_malloc( sizeof( CameraData ), 16 );

    m_pAlignedCameraData->m_InitialCamPos = m_Camera.get_Translation();
    m_pAlignedCameraData->m_InitialCamRot = m_Camera.get_Rotation();
    m_pAlignedCameraData->m_InitialFov    = m_Camera.get_FoV();
}

Tutorial4::~Tutorial4()
{
    _aligned_free( m_pAlignedCameraData );
}

uint32_t Tutorial4::Run()
{
    LoadContent();

    m_Window->Show();

    uint32_t retCode = GameFramework::Get().Run();

    UnloadContent();

    return retCode;
}

bool Tutorial4::LoadContent()
{
    m_Device    = Device::Create();
    m_SwapChain = m_Device->CreateSwapChain( m_Window->GetWindowHandle(), DXGI_FORMAT_B8G8R8A8_UNORM );
    m_SwapChain->SetVSync( m_VSync );

    m_GUI = m_Device->CreateGUI( m_Window->GetWindowHandle(), m_SwapChain->GetRenderTarget() );

    // This magic here allows ImGui to process window messages.
    GameFramework::Get().WndProcHandler += WndProcEvent::slot( &GUI::WndProcHandler, m_GUI );

    auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );
    auto  commandList  = commandQueue.GetCommandList();

    // Create some geometry to render.
    m_Cube     = commandList->CreateCube();
    m_Sphere   = commandList->CreateSphere();
    m_Cone     = commandList->CreateCone();
    m_Cylinder = commandList->CreateCylinder();
    m_Torus    = commandList->CreateTorus();
    m_Plane    = commandList->CreatePlane();

    // Create an inverted (reverse winding order) cube so the insides are not clipped.
    m_Skybox = commandList->CreateCube( 1.0f, true );

    // Load some textures
    m_DefaultTexture        = commandList->LoadTextureFromFile( L"Assets/Textures/DefaultWhite.bmp" );
    m_DirectXTexture        = commandList->LoadTextureFromFile( L"Assets/Textures/Directx9.png" );
    m_EarthTexture          = commandList->LoadTextureFromFile( L"Assets/Textures/earth.dds" );
    m_MonaLisaTexture       = commandList->LoadTextureFromFile( L"Assets/Textures/Mona_Lisa.jpg" );
    m_GraceCathedralTexture = commandList->LoadTextureFromFile( L"Assets/Textures/grace-new.hdr" );

    // m_GraceCathedralTexture = commandList->LoadTextureFromFile( L"Assets/Textures/UV_Test_Pattern.png" );

    // Create a cubemap for the HDR panorama.
    auto cubemapDesc  = m_GraceCathedralTexture->GetD3D12ResourceDesc();
    cubemapDesc.Width = cubemapDesc.Height = 1024;
    cubemapDesc.DepthOrArraySize           = 6;
    cubemapDesc.MipLevels                  = 0;

    m_GraceCathedralCubemap = m_Device->CreateTexture( cubemapDesc, TextureUsage::Albedo );
    m_GraceCathedralCubemap->SetName( L"Grace Cathedral Cubemap" );

    // Convert the 2D panorama to a 3D cubemap.
    commandList->PanoToCubemap( m_GraceCathedralCubemap, m_GraceCathedralTexture );

    // Start loading resources while the rest of the resources are created.
    commandQueue.ExecuteCommandList( commandList );

    D3D12_SHADER_RESOURCE_VIEW_DESC cubeMapSRVDesc = {};
    cubeMapSRVDesc.Format                          = cubemapDesc.Format;
    cubeMapSRVDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    cubeMapSRVDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
    cubeMapSRVDesc.TextureCube.MipLevels           = (UINT)-1;  // Use all mips.

    m_GraceCathedralCubemapSRV = m_Device->CreateShaderResourceView( m_GraceCathedralCubemap, &cubeMapSRVDesc );

    // Create an HDR intermediate render target.
    DXGI_FORMAT HDRFormat         = DXGI_FORMAT_R16G16B16A16_FLOAT;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;

    // Create an off-screen render target with a single color buffer and a depth buffer.
    auto colorDesc  = CD3DX12_RESOURCE_DESC::Tex2D( HDRFormat, m_Width, m_Height );
    colorDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE colorClearValue;
    colorClearValue.Format   = colorDesc.Format;
    colorClearValue.Color[0] = 0.4f;
    colorClearValue.Color[1] = 0.6f;
    colorClearValue.Color[2] = 0.9f;
    colorClearValue.Color[3] = 1.0f;

    m_HDRTexture = m_Device->CreateTexture( colorDesc, TextureUsage::RenderTarget, &colorClearValue );
    m_HDRTexture->SetName( L"HDR Texture" );

    // Create a depth buffer for the HDR render target.
    auto depthDesc  = CD3DX12_RESOURCE_DESC::Tex2D( depthBufferFormat, m_Width, m_Height );
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE depthClearValue;
    depthClearValue.Format       = depthDesc.Format;
    depthClearValue.DepthStencil = { 1.0f, 0 };

    auto depthTexture = m_Device->CreateTexture( depthDesc, TextureUsage::Depth, &depthClearValue );
    depthTexture->SetName( L"Depth Render Target" );

    // Attach the HDR texture to the HDR render target.
    m_HDRRenderTarget.AttachTexture( AttachmentPoint::Color0, m_HDRTexture );
    m_HDRRenderTarget.AttachTexture( AttachmentPoint::DepthStencil, depthTexture );

    // Create a root signature and PSO for the skybox shaders.
    {
        // Load the Skybox shaders.
        ComPtr<ID3DBlob> vs;
        ComPtr<ID3DBlob> ps;
        ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/04-HDR/Skybox_VS.cso", &vs ) );
        ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/04-HDR/Skybox_PS.cso", &ps ) );

        // Setup the input layout for the skybox vertex shader.
        D3D12_INPUT_ELEMENT_DESC inputLayout[1] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
              D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        // Allow input layout and deny unnecessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_DESCRIPTOR_RANGE1 descriptorRange( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 );

        CD3DX12_ROOT_PARAMETER1 rootParameters[2];
        rootParameters[0].InitAsConstants( sizeof( DirectX::XMMATRIX ) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX );
        rootParameters[1].InitAsDescriptorTable( 1, &descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL );

        CD3DX12_STATIC_SAMPLER_DESC linearClampSampler(
            0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init_1_1( 2, rootParameters, 1, &linearClampSampler, rootSignatureFlags );

        m_SkyboxSignature = m_Device->CreateRootSignature( rootSignatureDescription.Desc_1_1 );

        // Setup the Skybox pipeline state.
        struct SkyboxPipelineState
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        } skyboxPipelineStateStream;

        skyboxPipelineStateStream.pRootSignature        = m_SkyboxSignature->GetD3D12RootSignature().Get();
        skyboxPipelineStateStream.InputLayout           = { inputLayout, 1 };
        skyboxPipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        skyboxPipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE( vs.Get() );
        skyboxPipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE( ps.Get() );
        skyboxPipelineStateStream.RTVFormats            = m_HDRRenderTarget.GetRenderTargetFormats();

        m_SkyboxPipelineState = m_Device->CreatePipelineStateObject( skyboxPipelineStateStream );
    }

    // Create a root signature for the HDR pipeline.
    {
        // Load the HDR shaders.
        ComPtr<ID3DBlob> vs;
        ComPtr<ID3DBlob> ps;
        ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/04-HDR/HDR_VS.cso", &vs ) );
        ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/04-HDR/HDR_PS.cso", &ps ) );

        // Allow input layout and deny unnecessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_DESCRIPTOR_RANGE1 descriptorRange( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2 );

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
        rootParameters[RootParameters::Textures].InitAsDescriptorTable( 1, &descriptorRange,
                                                                        D3D12_SHADER_VISIBILITY_PIXEL );

        CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler( 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR );
        CD3DX12_STATIC_SAMPLER_DESC anisotropicSampler( 0, D3D12_FILTER_ANISOTROPIC );

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init_1_1( RootParameters::NumRootParameters, rootParameters, 1, &linearRepeatSampler,
                                           rootSignatureFlags );

        m_HDRRootSignature = m_Device->CreateRootSignature( rootSignatureDescription.Desc_1_1 );

        // Setup the HDR pipeline state.
        struct HDRPipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        } hdrPipelineStateStream;

        hdrPipelineStateStream.pRootSignature        = m_HDRRootSignature->GetD3D12RootSignature().Get();
        hdrPipelineStateStream.InputLayout           = VertexPositionNormalTangentBitangentTexture::InputLayout;
        hdrPipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        hdrPipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE( vs.Get() );
        hdrPipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE( ps.Get() );
        hdrPipelineStateStream.DSVFormat             = m_HDRRenderTarget.GetDepthStencilFormat();
        hdrPipelineStateStream.RTVFormats            = m_HDRRenderTarget.GetRenderTargetFormats();

        m_HDRPipelineState = m_Device->CreatePipelineStateObject( hdrPipelineStateStream );

        // The unlit pipeline state is similar to the HDR pipeline state except a different pixel shader.
        ComPtr<ID3DBlob> unlitPixelShader;
        ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/04-HDR/Unlit_PS.cso", &unlitPixelShader ) );

        hdrPipelineStateStream.PS = CD3DX12_SHADER_BYTECODE( unlitPixelShader.Get() );

        m_UnlitPipelineState = m_Device->CreatePipelineStateObject( hdrPipelineStateStream );
    }

    // Create the SDR Root Signature
    {
        CD3DX12_DESCRIPTOR_RANGE1 descriptorRange( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 );

        CD3DX12_ROOT_PARAMETER1 rootParameters[2];
        rootParameters[0].InitAsConstants( sizeof( TonemapParameters ) / 4, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL );
        rootParameters[1].InitAsDescriptorTable( 1, &descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL );

        CD3DX12_STATIC_SAMPLER_DESC linearClampsSampler(
            0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP );

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init_1_1( 2, rootParameters, 1, &linearClampsSampler );

        m_SDRRootSignature = m_Device->CreateRootSignature( rootSignatureDescription.Desc_1_1 );

        // Create the SDR PSO
        ComPtr<ID3DBlob> vs;
        ComPtr<ID3DBlob> ps;
        ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/04-HDR/HDRtoSDR_VS.cso", &vs ) );
        ThrowIfFailed( D3DReadFileToBlob( L"data/shaders/04-HDR/HDRtoSDR_PS.cso", &ps ) );

        CD3DX12_RASTERIZER_DESC rasterizerDesc( D3D12_DEFAULT );
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

        struct SDRPipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
            CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER            Rasterizer;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        } sdrPipelineStateStream;

        sdrPipelineStateStream.pRootSignature        = m_SDRRootSignature->GetD3D12RootSignature().Get();
        sdrPipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        sdrPipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE( vs.Get() );
        sdrPipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE( ps.Get() );
        sdrPipelineStateStream.Rasterizer            = rasterizerDesc;
        sdrPipelineStateStream.RTVFormats            = m_SwapChain->GetRenderTarget().GetRenderTargetFormats();

        m_SDRPipelineState = m_Device->CreatePipelineStateObject( sdrPipelineStateStream );
    }

    // Make sure the command queue is finished loading resources before rendering the first frame.
    commandQueue.Flush();

    return true;
}

void Tutorial4::RescaleHDRRenderTarget( float scale )
{
    uint32_t width  = static_cast<uint32_t>( m_Width * scale );
    uint32_t height = static_cast<uint32_t>( m_Height * scale );

    width  = std::clamp<uint32_t>( width, 1, D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION );
    height = std::clamp<uint32_t>( height, 1, D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION );

    m_HDRRenderTarget.Resize( width, height );
}

void Tutorial4::OnResize( ResizeEventArgs& e )
{
    m_Width  = std::max( 1, e.Width );
    m_Height = std::max( 1, e.Height );

    float fov         = m_Camera.get_FoV();
    float aspectRatio = m_Width / (float)m_Height;
    m_Camera.set_Projection( fov, aspectRatio, 0.1f, 100.0f );

    RescaleHDRRenderTarget( m_RenderScale );

    m_SwapChain->Resize( m_Width, m_Height );
}

void Tutorial4::OnDPIScaleChanged( DPIScaleEventArgs& e )
{
    m_GUI->SetScaling( e.DPIScale );
}

void Tutorial4::UnloadContent()
{
    m_Sphere.reset();
    m_Cone.reset();
    m_Torus.reset();
    m_Plane.reset();
    m_Skybox.reset();

    m_DefaultTexture.reset();
    m_DirectXTexture.reset();
    m_EarthTexture.reset();
    m_MonaLisaTexture.reset();
    m_GraceCathedralTexture.reset();
    m_GraceCathedralCubemap.reset();

    m_SkyboxSignature.reset();
    m_HDRRootSignature.reset();
    m_SDRRootSignature.reset();
    m_SkyboxPipelineState.reset();
    m_HDRPipelineState.reset();
    m_SDRPipelineState.reset();
    m_UnlitPipelineState.reset();

    m_HDRRenderTarget.Reset();

    m_GUI.reset();
    m_SwapChain.reset();
    m_Device.reset();
}

static double g_FPS = 0.0;

void Tutorial4::OnUpdate( UpdateEventArgs& e )
{
    static uint64_t frameCount = 0;
    static double   totalTime  = 0.0;

    totalTime += e.DeltaTime;
    frameCount++;

    if ( totalTime > 1.0 )
    {
        g_FPS = frameCount / totalTime;

        m_Logger->info( "FPS: {:.7}", g_FPS );

        wchar_t buffer[512];
        ::swprintf_s( buffer, L"HDR [FPS: %f]", g_FPS );
        m_Window->SetWindowTitle( buffer );

        frameCount = 0;
        totalTime  = 0.0;
    }

    m_Window->SetFullscreen( m_Fullscreen );

    // To reduce potential input lag, wait for the swap chain to be ready to present before updating the camera.
    m_SwapChain->WaitForSwapChain();

    // Update the camera.
    float speedMultipler = ( m_Shift ? 16.0f : 4.0f );

    XMVECTOR cameraTranslate = XMVectorSet( m_Right - m_Left, 0.0f, m_Forward - m_Backward, 1.0f ) * speedMultipler *
                               static_cast<float>( e.DeltaTime );
    XMVECTOR cameraPan =
        XMVectorSet( 0.0f, m_Up - m_Down, 0.0f, 1.0f ) * speedMultipler * static_cast<float>( e.DeltaTime );
    m_Camera.Translate( cameraTranslate, Space::Local );
    m_Camera.Translate( cameraPan, Space::Local );

    XMVECTOR cameraRotation =
        XMQuaternionRotationRollPitchYaw( XMConvertToRadians( m_Pitch ), XMConvertToRadians( m_Yaw ), 0.0f );
    m_Camera.set_Rotation( cameraRotation );

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

        l.Color       = XMFLOAT4( LightColors[i] );
        l.Intensity   = 1.0f;
        l.Attenuation = 0.0f;
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

        l.Color       = XMFLOAT4( LightColors[numPointLights + i] );
        l.Intensity   = 1.0f;
        l.SpotAngle   = XMConvertToRadians( 45.0f );
        l.Attenuation = 0.0f;
    }

    OnRender();
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
static void ShowHelpMarker( const char* desc )
{
    ImGui::TextDisabled( "(?)" );
    if ( ImGui::IsItemHovered() )
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
        ImGui::TextUnformatted( desc );
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

// Number of values to plot in the tonemapping curves.
static const int VALUES_COUNT = 256;
// Maximum HDR value to normalize the plot samples.
static const float HDR_MAX = 12.0f;

float LinearTonemapping( float HDR, float max )
{
    if ( max > 0.0f )
    {
        return std::clamp( HDR / max, 0.0f, 1.0f );
    }
    return HDR;
}

float LinearTonemappingPlot( void*, int index )
{
    return LinearTonemapping( index / (float)VALUES_COUNT * HDR_MAX, g_TonemapParameters.MaxLuminance );
}

// Reinhard tone mapping.
// See: http://www.cs.utah.edu/~reinhard/cdrom/tonemap.pdf
float ReinhardTonemapping( float HDR, float k )
{
    return HDR / ( HDR + k );
}

float ReinhardTonemappingPlot( void*, int index )
{
    return ReinhardTonemapping( index / (float)VALUES_COUNT * HDR_MAX, g_TonemapParameters.K );
}

float ReinhardSqrTonemappingPlot( void*, int index )
{
    float reinhard = ReinhardTonemapping( index / (float)VALUES_COUNT * HDR_MAX, g_TonemapParameters.K );
    return reinhard * reinhard;
}

// ACES Filmic
// See: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/142
float ACESFilmicTonemapping( float x, float A, float B, float C, float D, float E, float F )
{
    return ( ( ( x * ( A * x + C * B ) + D * E ) / ( x * ( A * x + B ) + D * F ) ) - ( E / F ) );
}

float ACESFilmicTonemappingPlot( void*, int index )
{
    float HDR = index / (float)VALUES_COUNT * HDR_MAX;
    return ACESFilmicTonemapping( HDR, g_TonemapParameters.A, g_TonemapParameters.B, g_TonemapParameters.C,
                                  g_TonemapParameters.D, g_TonemapParameters.E, g_TonemapParameters.F ) /
           ACESFilmicTonemapping( g_TonemapParameters.LinearWhite, g_TonemapParameters.A, g_TonemapParameters.B,
                                  g_TonemapParameters.C, g_TonemapParameters.D, g_TonemapParameters.E,
                                  g_TonemapParameters.F );
}

void Tutorial4::OnGUI( const std::shared_ptr<dx12lib::CommandList>& commandList,
                       const dx12lib::RenderTarget&                 renderTarget )
{
    static bool showDemoWindow = false;
    static bool showOptions    = true;

    m_GUI->NewFrame();

    if ( ImGui::BeginMainMenuBar() )
    {
        if ( ImGui::BeginMenu( "File" ) )
        {
            if ( ImGui::MenuItem( "Exit", "Esc" ) )
            {
                GameFramework::Get().Stop();
            }
            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( "View" ) )
        {
            ImGui::MenuItem( "ImGui Demo", nullptr, &showDemoWindow );
            ImGui::MenuItem( "Tonemapping", nullptr, &showOptions );

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( "Options" ) )
        {
            bool vSync = m_SwapChain->GetVSync();
            if ( ImGui::MenuItem( "V-Sync", "V", &vSync ) )
            {
                m_SwapChain->SetVSync( vSync );
            }

            bool fullscreen = m_Window->IsFullscreen();
            if ( ImGui::MenuItem( "Full screen", "Alt+Enter", &fullscreen ) )
            {
                // m_Window->SetFullscreen( fullscreen );
                // Defer the window resizing until the reference to the render target is released.
                m_Fullscreen = fullscreen;
            }

            ImGui::EndMenu();
        }

        char buffer[256];
        {
            // Output a slider to scale the resolution of the HDR render target.
            float renderScale = m_RenderScale;
            ImGui::PushItemWidth( 300.0f );
            ImGui::SliderFloat( "Resolution Scale", &renderScale, 0.1f, 2.0f );
            // Using Ctrl+Click on the slider, the user can set values outside of the
            // specified range. Make sure to clamp to a sane range to avoid creating
            // giant render targets.
            renderScale = std::clamp( renderScale, 0.0f, 2.0f );

            // Output current resolution of render target.
            auto size = m_HDRRenderTarget.GetSize();
            ImGui::SameLine();
            sprintf_s( buffer, _countof( buffer ), "(%ux%u)", size.x, size.y );
            ImGui::Text( buffer );

            // Resize HDR render target if the scale changed.
            if ( renderScale != m_RenderScale )
            {
                m_RenderScale = renderScale;
                RescaleHDRRenderTarget( m_RenderScale );
            }
        }

        {
            sprintf_s( buffer, _countof( buffer ), "FPS: %.2f (%.2f ms)  ", g_FPS, 1.0 / g_FPS * 1000.0 );
            auto fpsTextSize = ImGui::CalcTextSize( buffer );
            ImGui::SameLine( ImGui::GetWindowWidth() - fpsTextSize.x );
            ImGui::Text( buffer );
        }

        ImGui::EndMainMenuBar();
    }

    if ( showDemoWindow )
    {
        ImGui::ShowDemoWindow( &showDemoWindow );
    }

    if ( showOptions )
    {
        ImGui::Begin( "Tonemapping", &showOptions );
        {
            ImGui::TextWrapped( "Use the Exposure slider to adjust the overall exposure of the HDR scene." );
            ImGui::SliderFloat( "Exposure", &g_TonemapParameters.Exposure, -10.0f, 10.0f );
            ImGui::SameLine();
            ShowHelpMarker( "Adjust the overall exposure of the HDR scene." );
            ImGui::SliderFloat( "Gamma", &g_TonemapParameters.Gamma, 0.01f, 5.0f );
            ImGui::SameLine();
            ShowHelpMarker( "Adjust the Gamma of the output image." );

            const char* toneMappingMethods[] = { "Linear", "Reinhard", "Reinhard Squared", "ACES Filmic" };

            ImGui::Combo( "Tonemapping Methods", (int*)( &g_TonemapParameters.TonemapMethod ), toneMappingMethods, 4 );

            switch ( g_TonemapParameters.TonemapMethod )
            {
            case TonemapMethod::TM_Linear:
                ImGui::PlotLines( "Linear Tonemapping", &LinearTonemappingPlot, nullptr, VALUES_COUNT, 0, nullptr, 0.0f,
                                  1.0f, ImVec2( 0, 250 ) );
                ImGui::SliderFloat( "Max Brightness", &g_TonemapParameters.MaxLuminance, 1.0f, HDR_MAX );
                ImGui::SameLine();
                ShowHelpMarker( "Linearly scale the HDR image by the maximum brightness." );
                break;
            case TonemapMethod::TM_Reinhard:
                ImGui::PlotLines( "Reinhard Tonemapping", &ReinhardTonemappingPlot, nullptr, VALUES_COUNT, 0, nullptr,
                                  0.0f, 1.0f, ImVec2( 0, 250 ) );
                ImGui::SliderFloat( "Reinhard Constant", &g_TonemapParameters.K, 0.01f, 10.0f );
                ImGui::SameLine();
                ShowHelpMarker( "The Reinhard constant is used in the denominator." );
                break;
            case TonemapMethod::TM_ReinhardSq:
                ImGui::PlotLines( "Reinhard Squared Tonemapping", &ReinhardSqrTonemappingPlot, nullptr, VALUES_COUNT, 0,
                                  nullptr, 0.0f, 1.0f, ImVec2( 0, 250 ) );
                ImGui::SliderFloat( "Reinhard Constant", &g_TonemapParameters.K, 0.01f, 10.0f );
                ImGui::SameLine();
                ShowHelpMarker( "The Reinhard constant is used in the denominator." );
                break;
            case TonemapMethod::TM_ACESFilmic:
                ImGui::PlotLines( "ACES Filmic Tonemapping", &ACESFilmicTonemappingPlot, nullptr, VALUES_COUNT, 0,
                                  nullptr, 0.0f, 1.0f, ImVec2( 0, 250 ) );
                ImGui::SliderFloat( "Shoulder Strength", &g_TonemapParameters.A, 0.01f, 5.0f );
                ImGui::SliderFloat( "Linear Strength", &g_TonemapParameters.B, 0.0f, 100.0f );
                ImGui::SliderFloat( "Linear Angle", &g_TonemapParameters.C, 0.0f, 1.0f );
                ImGui::SliderFloat( "Toe Strength", &g_TonemapParameters.D, 0.01f, 1.0f );
                ImGui::SliderFloat( "Toe Numerator", &g_TonemapParameters.E, 0.0f, 10.0f );
                ImGui::SliderFloat( "Toe Denominator", &g_TonemapParameters.F, 1.0f, 10.0f );
                ImGui::SliderFloat( "Linear White", &g_TonemapParameters.LinearWhite, 1.0f, 120.0f );
                break;
            default:
                break;
            }
        }

        if ( ImGui::Button( "Reset to Defaults" ) )
        {
            TonemapMethod method              = g_TonemapParameters.TonemapMethod;
            g_TonemapParameters               = TonemapParameters();
            g_TonemapParameters.TonemapMethod = method;
        }

        ImGui::End();
    }

    m_GUI->Render( commandList, renderTarget );
}

void XM_CALLCONV ComputeMatrices( FXMMATRIX model, CXMMATRIX view, CXMMATRIX viewProjection, Mat& mat )
{
    mat.ModelMatrix                     = model;
    mat.ModelViewMatrix                 = model * view;
    mat.InverseTransposeModelViewMatrix = XMMatrixTranspose( XMMatrixInverse( nullptr, mat.ModelViewMatrix ) );
    mat.ModelViewProjectionMatrix       = model * viewProjection;
}

void Tutorial4::OnRender()
{
    auto& commandQueue = m_Device->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
    auto  commandList  = commandQueue.GetCommandList();

    // Create a scene visitor that is used to perform the actual rendering of the meshes in the scenes.
    SceneVisitor visitor( *commandList );

    // Clear the render targets.
    {
        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

        commandList->ClearTexture( m_HDRRenderTarget.GetTexture( AttachmentPoint::Color0 ), clearColor );
        commandList->ClearDepthStencilTexture( m_HDRRenderTarget.GetTexture( AttachmentPoint::DepthStencil ),
                                               D3D12_CLEAR_FLAG_DEPTH );
    }

    commandList->SetRenderTarget( m_HDRRenderTarget );
    commandList->SetViewport( m_HDRRenderTarget.GetViewport() );
    commandList->SetScissorRect( m_ScissorRect );

    // Render the skybox.
    {
        // The view matrix should only consider the camera's rotation, but not the translation.
        auto viewMatrix     = XMMatrixTranspose( XMMatrixRotationQuaternion( m_Camera.get_Rotation() ) );
        auto projMatrix     = m_Camera.get_ProjectionMatrix();
        auto viewProjMatrix = viewMatrix * projMatrix;

        commandList->SetPipelineState( m_SkyboxPipelineState );
        commandList->SetGraphicsRootSignature( m_SkyboxSignature );

        commandList->SetGraphics32BitConstants( 0, viewProjMatrix );

        commandList->SetShaderResourceView( 1, 0, m_GraceCathedralCubemapSRV,
                                            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

        m_Skybox->Accept( visitor );
    }

    commandList->SetPipelineState( m_HDRPipelineState );
    commandList->SetGraphicsRootSignature( m_HDRRootSignature );

    // Upload lights
    LightProperties lightProps;
    lightProps.NumPointLights = static_cast<uint32_t>( m_PointLights.size() );
    lightProps.NumSpotLights  = static_cast<uint32_t>( m_SpotLights.size() );

    commandList->SetGraphics32BitConstants( RootParameters::LightPropertiesCB, lightProps );
    commandList->SetGraphicsDynamicStructuredBuffer( RootParameters::PointLights, m_PointLights );
    commandList->SetGraphicsDynamicStructuredBuffer( RootParameters::SpotLights, m_SpotLights );

    // Draw the earth sphere
    XMMATRIX translationMatrix    = XMMatrixTranslation( -4.0f, 2.0f, -4.0f );
    XMMATRIX rotationMatrix       = XMMatrixIdentity();
    XMMATRIX scaleMatrix          = XMMatrixScaling( 4.0f, 4.0f, 4.0f );
    XMMATRIX worldMatrix          = scaleMatrix * rotationMatrix * translationMatrix;
    XMMATRIX viewMatrix           = m_Camera.get_ViewMatrix();
    XMMATRIX viewProjectionMatrix = viewMatrix * m_Camera.get_ProjectionMatrix();

    Mat matrices;
    ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );
    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MaterialCB, Material::White );
    commandList->SetShaderResourceView( RootParameters::Textures, 0, m_EarthTexture,
                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    m_Sphere->Accept( visitor );

    // Draw a cube
    translationMatrix = XMMatrixTranslation( 4.0f, 4.0f, 4.0f );
    rotationMatrix    = XMMatrixRotationY( XMConvertToRadians( 45.0f ) );
    scaleMatrix       = XMMatrixScaling( 4.0f, 8.0f, 4.0f );
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );
    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MaterialCB, Material::White );
    commandList->SetShaderResourceView( RootParameters::Textures, 0, m_MonaLisaTexture,
                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    m_Cube->Accept( visitor );

    // Draw a torus
    translationMatrix = XMMatrixTranslation( 4.0f, 0.6f, -4.0f );
    rotationMatrix    = XMMatrixRotationY( XMConvertToRadians( 45.0f ) );
    scaleMatrix       = XMMatrixScaling( 4.0f, 4.0f, 4.0f );
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );
    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MaterialCB, Material::Ruby );
    commandList->SetShaderResourceView( RootParameters::Textures, 0, m_DefaultTexture,
                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    m_Torus->Accept( visitor );

    // Draw a cylinder
    translationMatrix = XMMatrixTranslation( -4.0f, 4.0f, 4.0f );
    rotationMatrix    = XMMatrixRotationY( XMConvertToRadians( 45.0f ) );
    scaleMatrix       = XMMatrixScaling( 4.0f, 8.0f, 4.0f );
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );
    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MaterialCB, Material::Gold );
    commandList->SetShaderResourceView( RootParameters::Textures, 0, m_DefaultTexture,
                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    m_Cylinder->Accept( visitor );

    // Floor plane.
    float scalePlane      = 20.0f;
    float translateOffset = scalePlane / 2.0f;

    translationMatrix = XMMatrixTranslation( 0.0f, 0.0f, 0.0f );
    rotationMatrix    = XMMatrixIdentity();
    scaleMatrix       = XMMatrixScaling( scalePlane, 1.0f, scalePlane );
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );
    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MaterialCB, Material::White );
    commandList->SetShaderResourceView( RootParameters::Textures, 0, m_DirectXTexture,
                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    m_Plane->Accept( visitor );

    // Back wall
    translationMatrix = XMMatrixTranslation( 0, translateOffset, translateOffset );
    rotationMatrix    = XMMatrixRotationX( XMConvertToRadians( -90 ) );
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );

    m_Plane->Accept( visitor );

    // Ceiling plane
    translationMatrix = XMMatrixTranslation( 0, translateOffset * 2.0f, 0 );
    rotationMatrix    = XMMatrixRotationX( XMConvertToRadians( 180 ) );
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );

    m_Plane->Accept( visitor );

    // Front wall
    translationMatrix = XMMatrixTranslation( 0, translateOffset, -translateOffset );
    rotationMatrix    = XMMatrixRotationX( XMConvertToRadians( 90 ) );
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );

    m_Plane->Accept( visitor );

    // Left wall
    translationMatrix = XMMatrixTranslation( -translateOffset, translateOffset, 0 );
    rotationMatrix    = XMMatrixRotationX( XMConvertToRadians( -90 ) ) * XMMatrixRotationY( XMConvertToRadians( -90 ) );
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );
    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MaterialCB, Material::Red );
    commandList->SetShaderResourceView( RootParameters::Textures, 0, m_DefaultTexture,
                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    m_Plane->Accept( visitor );

    // Right wall
    translationMatrix = XMMatrixTranslation( translateOffset, translateOffset, 0 );
    rotationMatrix    = XMMatrixRotationX( XMConvertToRadians( -90 ) ) * XMMatrixRotationY( XMConvertToRadians( 90 ) );
    worldMatrix       = scaleMatrix * rotationMatrix * translationMatrix;

    ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );
    commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MaterialCB, Material::Blue );

    m_Plane->Accept( visitor );

    // Draw shapes to visualize the position of the lights in the scene.
    commandList->SetPipelineState( m_UnlitPipelineState );

    MaterialProperties lightMaterial = Material::Zero;
    for ( const auto& l: m_PointLights )
    {
        lightMaterial.Emissive = l.Color;
        XMVECTOR lightPos      = XMLoadFloat4( &l.PositionWS );
        worldMatrix            = XMMatrixTranslationFromVector( lightPos );
        ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

        commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );
        commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MaterialCB, lightMaterial );

        m_Sphere->Accept( visitor );
    }

    for ( const auto& l: m_SpotLights )
    {
        lightMaterial.Emissive = l.Color;
        XMVECTOR lightPos      = XMLoadFloat4( &l.PositionWS );
        XMVECTOR lightDir      = XMLoadFloat4( &l.DirectionWS );
        XMVECTOR up            = XMVectorSet( 0, 1, 0, 0 );

        // Rotate the cone so it is facing the Z axis.
        rotationMatrix = XMMatrixRotationX( XMConvertToRadians( -90.0f ) );
        worldMatrix    = rotationMatrix * LookAtMatrix( lightPos, lightDir, up );

        ComputeMatrices( worldMatrix, viewMatrix, viewProjectionMatrix, matrices );

        commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, matrices );
        commandList->SetGraphicsDynamicConstantBuffer( RootParameters::MaterialCB, lightMaterial );

        m_Cone->Accept( visitor );
    }

    // Perform HDR -> SDR tonemapping directly to the SwapChain's render target.
    commandList->SetRenderTarget( m_SwapChain->GetRenderTarget() );
    commandList->SetViewport( m_SwapChain->GetRenderTarget().GetViewport() );
    commandList->SetPipelineState( m_SDRPipelineState );
    commandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    commandList->SetGraphicsRootSignature( m_SDRRootSignature );
    commandList->SetGraphics32BitConstants( 0, g_TonemapParameters );
    commandList->SetShaderResourceView( 1, 0, m_HDRTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    commandList->Draw( 3 );

    // Render GUI.
    OnGUI( commandList, m_SwapChain->GetRenderTarget() );

    commandQueue.ExecuteCommandList( commandList );

    // Present
    m_SwapChain->Present();
}

static bool g_AllowFullscreenToggle = true;

void Tutorial4::OnKeyPressed( KeyEventArgs& e )
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
                if ( g_AllowFullscreenToggle )
                {
                    m_Fullscreen = !m_Fullscreen;  // Defer window resizing until OnUpdate();
                    // Prevent the key repeat to cause multiple resizes.
                    g_AllowFullscreenToggle = false;
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
            m_Camera.set_FoV( m_pAlignedCameraData->m_InitialFov );
            m_Pitch = 0.0f;
            m_Yaw   = 0.0f;
            break;
        case KeyCode::Up:
        case KeyCode::W:
            m_Forward = 1.0f;
            break;
        case KeyCode::Left:
        case KeyCode::A:
            m_Left = 1.0f;
            break;
        case KeyCode::Down:
        case KeyCode::S:
            m_Backward = 1.0f;
            break;
        case KeyCode::Right:
        case KeyCode::D:
            m_Right = 1.0f;
            break;
        case KeyCode::Q:
            m_Down = 1.0f;
            break;
        case KeyCode::E:
            m_Up = 1.0f;
            break;
        case KeyCode::Space:
            m_AnimateLights = !m_AnimateLights;
            break;
        case KeyCode::ShiftKey:
            m_Shift = true;
            break;
        }
    }
}

void Tutorial4::OnKeyReleased( KeyEventArgs& e )
{
    if ( !ImGui::GetIO().WantCaptureKeyboard )
    {
        switch ( e.Key )
        {
        case KeyCode::Enter:
            if ( e.Alt )
            {
            case KeyCode::F11:
                g_AllowFullscreenToggle = true;
            }
            break;
        case KeyCode::Up:
        case KeyCode::W:
            m_Forward = 0.0f;
            break;
        case KeyCode::Left:
        case KeyCode::A:
            m_Left = 0.0f;
            break;
        case KeyCode::Down:
        case KeyCode::S:
            m_Backward = 0.0f;
            break;
        case KeyCode::Right:
        case KeyCode::D:
            m_Right = 0.0f;
            break;
        case KeyCode::Q:
            m_Down = 0.0f;
            break;
        case KeyCode::E:
            m_Up = 0.0f;
            break;
        case KeyCode::ShiftKey:
            m_Shift = false;
            break;
        }
    }
}

void Tutorial4::OnMouseMoved( MouseMotionEventArgs& e )
{
    const float mouseSpeed = 0.1f;
    if ( !ImGui::GetIO().WantCaptureMouse )
    {
        if ( e.LeftButton )
        {
            m_Pitch -= e.RelY * mouseSpeed;

            m_Pitch = std::clamp( m_Pitch, -90.0f, 90.0f );

            m_Yaw -= e.RelX * mouseSpeed;
        }
    }
}

void Tutorial4::OnMouseWheel( MouseWheelEventArgs& e )
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
