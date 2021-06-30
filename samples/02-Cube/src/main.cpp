#include <GameFramework/GameFramework.h>
#include <GameFramework/Window.h>

#include <dx12lib/Adapter.h>
#include <dx12lib/CommandList.h>
#include <dx12lib/CommandQueue.h>
#include <dx12lib/Device.h>
#include <dx12lib/Helpers.h>
#include <dx12lib/IndexBuffer.h>
#include <dx12lib/PipelineStateObject.h>
#include <dx12lib/RootSignature.h>
#include <dx12lib/SwapChain.h>
#include <dx12lib/VertexBuffer.h>

#include <spdlog/spdlog.h>

#include <shlwapi.h>  // for CommandLineToArgvW

#include <d3dcompiler.h>  // For D3DReadFileToBlob
#include <dxgidebug.h>    // For ReportLiveObjects.

#include <DirectXMath.h>  // For DirectX Math types.

using namespace dx12lib;
using namespace DirectX;
using namespace Microsoft::WRL;

void OnUpdate( UpdateEventArgs& e );
void OnRender( RenderEventArgs& e );
void OnKeyPressed( KeyEventArgs& e );
void OnMouseWheel( MouseWheelEventArgs& e );
void OnResized( ResizeEventArgs& e );
void OnWindowClose( WindowCloseEventArgs& e );

std::shared_ptr<Device>              pDevice              = nullptr;
std::shared_ptr<Window>              pGameWindow          = nullptr;
std::shared_ptr<SwapChain>           pSwapChain           = nullptr;
std::shared_ptr<Texture>             pDepthTexture        = nullptr;
std::shared_ptr<VertexBuffer>        pVertexBuffer        = nullptr;
std::shared_ptr<IndexBuffer>         pIndexBuffer         = nullptr;
std::shared_ptr<RootSignature>       pRootSignature       = nullptr;
std::shared_ptr<PipelineStateObject> pPipelineStateObject = nullptr;

Logger logger;

// Vertex data for a colored cube.
struct VertexPosColor
{
    XMFLOAT3 Position;
    XMFLOAT3 Color;
};

static VertexPosColor g_Vertices[8] = {
    { XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, 0.0f ) },  // 0
    { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ) },   // 1
    { XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 1.0f, 0.0f ) },    // 2
    { XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ) },   // 3
    { XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ) },   // 4
    { XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f, 1.0f ) },    // 5
    { XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 1.0f, 1.0f, 1.0f ) },     // 6
    { XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f, 1.0f ) }     // 7
};

static WORD g_Indicies[36] = { 0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 4, 5, 1, 4, 1, 0,
                               3, 2, 6, 3, 6, 7, 1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7 };

float fieldOfView = 45.0f;

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow )
{
    int retCode = 0;

#if defined( _DEBUG )
    Device::EnableDebugLayer();
#endif

    // Set the working directory to the path of the executable.
    WCHAR   path[MAX_PATH];
    HMODULE hModule = ::GetModuleHandleW( NULL );
    if ( ::GetModuleFileNameW( hModule, path, MAX_PATH ) > 0 )
    {
        ::PathRemoveFileSpecW( path );
        ::SetCurrentDirectoryW( path );
    }

    auto& gf = GameFramework::Create( hInstance );
    {
        // Create a logger for logging messages.
        logger = gf.CreateLogger( "Cube" );

        // Create a GPU device using the default adapter selection.
        pDevice = Device::Create();

        auto description = pDevice->GetDescription();
        logger->info( L"Device Created: {}", description );

        // Use a copy queue to copy GPU resources.
        auto& commandQueue = pDevice->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );
        auto  commandList  = commandQueue.GetCommandList();

        // Load vertex data:
        pVertexBuffer = commandList->CopyVertexBuffer( _countof( g_Vertices ), sizeof( VertexPosColor ), g_Vertices );

        // Load index data:
        pIndexBuffer = commandList->CopyIndexBuffer( _countof( g_Indicies ), DXGI_FORMAT_R16_UINT, g_Indicies );

        // Execute the command list to upload the resources to the GPU.
        commandQueue.ExecuteCommandList( commandList );

        // Create a window:
        pGameWindow = gf.CreateWindow( L"Cube", 1920, 1080 );

        // Create a swapchain for the window
        pSwapChain = pDevice->CreateSwapChain( pGameWindow->GetWindowHandle() );
        pSwapChain->SetVSync( false );

        // Register events.
        gf.Update += &OnUpdate;
        pGameWindow->KeyPressed += &OnKeyPressed;
        pGameWindow->MouseWheel += &OnMouseWheel;
        pGameWindow->Resize += &OnResized;
        pGameWindow->Render += &OnRender;
        pGameWindow->Close += &OnWindowClose;

        // Create the vertex input layout
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
              D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
              D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        // Create the root signature.
        // Allow input layout and deny unnecessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
                                                        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

        // A single 32-bit constant root parameter that is used by the vertex shader.
        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsConstants( sizeof( XMMATRIX ) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX );

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription( _countof( rootParameters ), rootParameters, 0,
                                                                        nullptr, rootSignatureFlags );

        pRootSignature = pDevice->CreateRootSignature( rootSignatureDescription.Desc_1_1 );

        // Load shaders
        // Load the vertex shader.
        ComPtr<ID3DBlob> vertexShaderBlob;
        ThrowIfFailed( D3DReadFileToBlob( L"VertexShader.cso", &vertexShaderBlob ) );

        // Load the pixel shader.
        ComPtr<ID3DBlob> pixelShaderBlob;
        ThrowIfFailed( D3DReadFileToBlob( L"PixelShader.cso", &pixelShaderBlob ) );

        // Create the pipeline state object.
        struct PipelineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE        pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT          InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY    PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS                    VS;
            CD3DX12_PIPELINE_STATE_STREAM_PS                    PS;
            CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT  DSVFormat;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        } pipelineStateStream;

        pipelineStateStream.pRootSignature        = pRootSignature->GetD3D12RootSignature().Get();
        pipelineStateStream.InputLayout           = { inputLayout, _countof( inputLayout ) };
        pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipelineStateStream.VS                    = CD3DX12_SHADER_BYTECODE( vertexShaderBlob.Get() );
        pipelineStateStream.PS                    = CD3DX12_SHADER_BYTECODE( pixelShaderBlob.Get() );
        pipelineStateStream.DSVFormat             = DXGI_FORMAT_D32_FLOAT;
        pipelineStateStream.RTVFormats            = pSwapChain->GetRenderTarget().GetRenderTargetFormats();

        pPipelineStateObject = pDevice->CreatePipelineStateObject( pipelineStateStream );

        // Make sure the index/vertex buffers are loaded before rendering the first frame.
        commandQueue.Flush();

        pGameWindow->Show();

        // Start the game loop.
        retCode = GameFramework::Get().Run();

        // Release globals.
        pIndexBuffer.reset();
        pVertexBuffer.reset();
        pPipelineStateObject.reset();
        pRootSignature.reset();
        pDepthTexture.reset();
        pDevice.reset();
        pSwapChain.reset();
        pGameWindow.reset();
    }
    // Destroy game framework resource.
    GameFramework::Destroy();

    atexit( &Device::ReportLiveObjects );

    return retCode;
}

void OnUpdate( UpdateEventArgs& e )
{
    static uint64_t frameCount = 0;
    static double   totalTime  = 0.0;

    totalTime += e.DeltaTime;
    frameCount++;

    if ( totalTime > 1.0 )
    {
        auto fps   = frameCount / totalTime;
        frameCount = 0;
        totalTime -= 1.0;

        logger->info( "FPS: {:.7}", fps );

        wchar_t buffer[256];
        ::swprintf_s( buffer, L"Cube [FPS: %f]", fps );
        pGameWindow->SetWindowTitle( buffer );
    }

    pGameWindow->Redraw();
}

void OnRender( RenderEventArgs& e )
{
    static HighResolutionTimer timer;
    timer.Tick();

    // Use the render target from the swapchain.
    auto renderTarget = pSwapChain->GetRenderTarget();
    // Set the render target (with the depth texture).
    renderTarget.AttachTexture( AttachmentPoint::DepthStencil, pDepthTexture );

    auto viewport = renderTarget.GetViewport();

    // Update the model matrix.
    float          angle        = static_cast<float>( timer.TotalSeconds() * 90.0 );
    const XMVECTOR rotationAxis = XMVectorSet( 0, 1, 1, 0 );
    XMMATRIX       modelMatrix  = XMMatrixRotationAxis( rotationAxis, XMConvertToRadians( angle ) );

    // Update the view matrix.
    const XMVECTOR eyePosition = XMVectorSet( 0, 0, -10, 1 );
    const XMVECTOR focusPoint  = XMVectorSet( 0, 0, 0, 1 );
    const XMVECTOR upDirection = XMVectorSet( 0, 1, 0, 0 );
    XMMATRIX       viewMatrix  = XMMatrixLookAtLH( eyePosition, focusPoint, upDirection );

    // Update the projection matrix.
    float    aspectRatio = viewport.Width / viewport.Height;
    XMMATRIX projectionMatrix =
        XMMatrixPerspectiveFovLH( XMConvertToRadians( fieldOfView ), aspectRatio, 0.1f, 100.0f );
    XMMATRIX mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
    mvpMatrix          = XMMatrixMultiply( mvpMatrix, projectionMatrix );

    auto& commandQueue = pDevice->GetCommandQueue( D3D12_COMMAND_LIST_TYPE_DIRECT );
    auto  commandList  = commandQueue.GetCommandList();

    // Set the pipeline state object
    commandList->SetPipelineState( pPipelineStateObject );
    // Set the root signatures.
    commandList->SetGraphicsRootSignature( pRootSignature );

    // Set root parameters
    commandList->SetGraphics32BitConstants( 0, mvpMatrix );

    // Clear the color and depth-stencil textures.
    const FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
    commandList->ClearTexture( renderTarget.GetTexture( AttachmentPoint::Color0 ), clearColor );
    commandList->ClearDepthStencilTexture( pDepthTexture, D3D12_CLEAR_FLAG_DEPTH );

    commandList->SetRenderTarget( renderTarget );
    commandList->SetViewport( renderTarget.GetViewport() );
    commandList->SetScissorRect( CD3DX12_RECT( 0, 0, LONG_MAX, LONG_MAX ) );

    // Render the cube.
    commandList->SetVertexBuffer( 0, pVertexBuffer );
    commandList->SetIndexBuffer( pIndexBuffer );
    commandList->SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    commandList->DrawIndexed( pIndexBuffer->GetNumIndicies() );

    commandQueue.ExecuteCommandList( commandList );

    // Present the image to the window.
    pSwapChain->Present();
}

void OnKeyPressed( KeyEventArgs& e )
{
    switch ( e.Key )
    {
    case KeyCode::V:
        pSwapChain->ToggleVSync();
        break;
    case KeyCode::Escape:
        // Stop the application if the Escape key is pressed.
        GameFramework::Get().Stop();
        break;
    case KeyCode::Enter:
        if ( e.Alt )
        {
            [[fallthrough]];
        case KeyCode::F11:
            pGameWindow->ToggleFullscreen();
            break;
        }
    }
}

void OnMouseWheel( MouseWheelEventArgs& e )
{
    fieldOfView -= e.WheelDelta;
    fieldOfView = std::clamp( fieldOfView, 12.0f, 90.0f );

    logger->info( "Field of View: {}", fieldOfView );
}

void OnResized( ResizeEventArgs& e )
{
    logger->info( "Window Resize: {}, {}", e.Width, e.Height );
    GameFramework::Get().SetDisplaySize( e.Width, e.Height );

    // Flush any pending commands before resizing resources.
    pDevice->Flush();

    // Resize the swap chain.
    pSwapChain->Resize( e.Width, e.Height );

    // Resize the depth texture.
    auto depthTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_D32_FLOAT, e.Width, e.Height );
    // Must be set on textures that will be used as a depth-stencil buffer.
    depthTextureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // Specify optimized clear values for the depth buffer.
    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format            = DXGI_FORMAT_D32_FLOAT;
    optimizedClearValue.DepthStencil      = { 1.0F, 0 };

    pDepthTexture = pDevice->CreateTexture( depthTextureDesc, &optimizedClearValue );
}

void OnWindowClose( WindowCloseEventArgs& e )
{
    // Stop the application if the window is closed.
    GameFramework::Get().Stop();
}
