#include <DX12LibPCH.h>

#include <GUI.h>

#include <Application.h>
#include <CommandQueue.h>
#include <CommandList.h>
#include <RenderTarget.h>
#include <RootSignature.h>
#include <Texture.h>
#include <Window.h>

// Include compiled shaders for ImGui.
#include <ImGUI_VS.h>
#include <ImGUI_PS.h>

#include <imgui_impl_win32.h>

// Root parameters for the ImGui root signature.
enum RootParameters
{
    MatrixCB,           // cbuffer vertexBuffer : register(b0)
    FontTexture,        // Texture2D texture0 : register(t0);
    NumRootParameters
};

//--------------------------------------------------------------------------------------
// Get surface information for a particular format
//--------------------------------------------------------------------------------------
void GetSurfaceInfo(
    _In_ size_t width,
    _In_ size_t height,
    _In_ DXGI_FORMAT fmt,
    size_t* outNumBytes,
    _Out_opt_ size_t* outRowBytes,
    _Out_opt_ size_t* outNumRows );

GUI::GUI()
    : m_pImGuiCtx( nullptr )
{}

GUI::~GUI()
{
    Destroy();
}

bool GUI::Initialize( std::shared_ptr<Window> window )
{
    m_Window = window;
    m_pImGuiCtx = ImGui::CreateContext();
    ImGui::SetCurrentContext( m_pImGuiCtx );
    if ( !m_Window || !ImGui_ImplWin32_Init( m_Window->GetWindowHandle() ) )
    {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();

    // Build texture atlas
    unsigned char* pixelData = nullptr;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32( &pixelData, &width, &height );

    auto device = Application::Get().GetDevice();
    auto commandQueue = Application::Get().GetCommandQueue( D3D12_COMMAND_LIST_TYPE_COPY );
    auto commandList = commandQueue->GetCommandList();

    auto fontTextureDesc = CD3DX12_RESOURCE_DESC::Tex2D( DXGI_FORMAT_R8G8B8A8_UNORM, width, height );

    m_FontTexture = std::make_unique<Texture>( fontTextureDesc );
    m_FontTexture->SetName( L"ImGui Font Texture" );

    size_t rowPitch, slicePitch;
    GetSurfaceInfo( width, height, DXGI_FORMAT_R8G8B8A8_UNORM, &slicePitch, &rowPitch, nullptr );

    D3D12_SUBRESOURCE_DATA subresourceData;
    subresourceData.pData = pixelData;
    subresourceData.RowPitch = rowPitch;
    subresourceData.SlicePitch = slicePitch;

    commandList->CopyTextureSubresource( *m_FontTexture, 0, 1, &subresourceData );
    commandList->GenerateMips( *m_FontTexture );

    commandQueue->ExecuteCommandList( commandList );

    // Create the root signature for the ImGUI shaders.
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if ( FAILED( device->CheckFeatureSupport( D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof( featureData ) ) ) )
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    CD3DX12_DESCRIPTOR_RANGE1 descriptorRage( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 );

    CD3DX12_ROOT_PARAMETER1 rootParameters[RootParameters::NumRootParameters];
    rootParameters[RootParameters::MatrixCB].InitAsConstants( sizeof( DirectX::XMMATRIX ) / 4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_VERTEX );
    rootParameters[RootParameters::FontTexture].InitAsDescriptorTable( 1, &descriptorRage, D3D12_SHADER_VISIBILITY_PIXEL );

    CD3DX12_STATIC_SAMPLER_DESC linearRepeatSampler( 0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR );
    linearRepeatSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    linearRepeatSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1( RootParameters::NumRootParameters, rootParameters, 1, &linearRepeatSampler, rootSignatureFlags );

    m_RootSignature = std::make_unique<RootSignature>( rootSignatureDescription.Desc_1_1, featureData.HighestVersion );

    const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, offsetof( ImDrawVert, pos ), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, offsetof( ImDrawVert, uv ),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof( ImDrawVert, col ), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_RT_FORMAT_ARRAY rtvFormats = {};
    rtvFormats.NumRenderTargets = 1;
    rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = false;
    depthStencilDesc.StencilEnable = false;

    // Setup the pipeline state.
    struct PipelineStateStream
    {
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS VS;
        CD3DX12_PIPELINE_STATE_STREAM_PS PS;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC SampleDesc;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC BlendDesc;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL DepthStencilState;
    } pipelineStateStream;

    pipelineStateStream.pRootSignature = m_RootSignature->GetRootSignature().Get();
    pipelineStateStream.InputLayout = { inputLayout, 3 };
    pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateStream.VS = { g_ImGUI_VS, sizeof( g_ImGUI_VS ) };
    pipelineStateStream.PS = { g_ImGUI_PS, sizeof( g_ImGUI_PS ) };
    pipelineStateStream.RTVFormats = rtvFormats;
    pipelineStateStream.SampleDesc = { 1, 0 };
    pipelineStateStream.BlendDesc = CD3DX12_BLEND_DESC( blendDesc );
    pipelineStateStream.RasterizerState = CD3DX12_RASTERIZER_DESC( rasterizerDesc );
    pipelineStateStream.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC( depthStencilDesc );
    
    D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
        sizeof( PipelineStateStream ), &pipelineStateStream
    };
    ThrowIfFailed( device->CreatePipelineState( &pipelineStateStreamDesc, IID_PPV_ARGS( &m_PipelineState ) ) );

    return true;
}

void GUI::NewFrame()
{
    ImGui::SetCurrentContext( m_pImGuiCtx );
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void GUI::Render( std::shared_ptr<CommandList> commandList, const RenderTarget& renderTarget )
{
    ImGui::SetCurrentContext( m_pImGuiCtx );
    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    ImDrawData* drawData = ImGui::GetDrawData();

    // Check if there is anything to render.
    if ( !drawData || drawData->CmdListsCount == 0 )
        return;

    ImVec2 displayPos = drawData->DisplayPos;

    commandList->SetGraphicsRootSignature( *m_RootSignature );
    commandList->SetPipelineState( m_PipelineState );
    commandList->SetRenderTarget( renderTarget );

    // Set root arguments.
//    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixOrthographicRH( drawData->DisplaySize.x, drawData->DisplaySize.y, 0.0f, 1.0f );
    float L = drawData->DisplayPos.x;
    float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
    float T = drawData->DisplayPos.y;
    float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
    float mvp[4][4] =
    {
        { 2.0f / ( R - L ),   0.0f,           0.0f,       0.0f },
        { 0.0f,         2.0f / ( T - B ),     0.0f,       0.0f },
        { 0.0f,         0.0f,           0.5f,       0.0f },
        { ( R + L ) / ( L - R ),  ( T + B ) / ( B - T ),    0.5f,       1.0f },
    };

    commandList->SetGraphics32BitConstants( RootParameters::MatrixCB, mvp );
    commandList->SetShaderResourceView( RootParameters::FontTexture, 0, *m_FontTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    D3D12_VIEWPORT viewport = {};
    viewport.Width = drawData->DisplaySize.x;
    viewport.Height = drawData->DisplaySize.y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    commandList->SetViewport( viewport );
    commandList->SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    const DXGI_FORMAT indexFormat = sizeof( ImDrawIdx ) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    // It may happen that ImGui doesn't actually render anything. In this case,
    // any pending resource barriers in the commandList will not be flushed (since 
    // resource barriers are only flushed when a draw command is executed).
    // In that case, manually flushing the resource barriers will ensure that
    // they are properly flushed before exiting this function.
    commandList->FlushResourceBarriers();

    for ( int i = 0; i < drawData->CmdListsCount; ++i )
    {
        const ImDrawList* drawList = drawData->CmdLists[i];

        commandList->SetDynamicVertexBuffer( 0, drawList->VtxBuffer.size(), sizeof( ImDrawVert ), drawList->VtxBuffer.Data );
        commandList->SetDynamicIndexBuffer( drawList->IdxBuffer.size(), indexFormat, drawList->IdxBuffer.Data );

        int indexOffset = 0;
        for ( int j = 0; j < drawList->CmdBuffer.size(); ++j )
        {
            const ImDrawCmd& drawCmd = drawList->CmdBuffer[j];
            if ( drawCmd.UserCallback )
            {
                drawCmd.UserCallback( drawList, &drawCmd );
            }
            else
            {
                ImVec4 clipRect = drawCmd.ClipRect;
                D3D12_RECT scissorRect;
                scissorRect.left = static_cast<LONG>( clipRect.x - displayPos.x);
                scissorRect.top = static_cast<LONG>( clipRect.y - displayPos.y );
                scissorRect.right = static_cast<LONG>( clipRect.z - displayPos.x );
                scissorRect.bottom = static_cast<LONG>( clipRect.w - displayPos.y );

                if ( scissorRect.right - scissorRect.left > 0.0f && 
                     scissorRect.bottom - scissorRect.top > 0.0 )
                {
                    commandList->SetScissorRect( scissorRect );
                    commandList->DrawIndexed( drawCmd.ElemCount, 1, indexOffset );
                }
            }
            indexOffset += drawCmd.ElemCount;
        }
    }
}

void GUI::Destroy()
{
    if ( m_Window )
    {
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext( m_pImGuiCtx );
        m_pImGuiCtx = nullptr;
        m_Window.reset();
    }
}

//--------------------------------------------------------------------------------------
// Get surface information for a particular format
//--------------------------------------------------------------------------------------
void GetSurfaceInfo(
    _In_ size_t width,
    _In_ size_t height,
    _In_ DXGI_FORMAT fmt,
    size_t* outNumBytes,
    _Out_opt_ size_t* outRowBytes,
    _Out_opt_ size_t* outNumRows )
{
    size_t numBytes = 0;
    size_t rowBytes = 0;
    size_t numRows = 0;

    bool bc = false;
    bool packed = false;
    bool planar = false;
    size_t bpe = 0;
    switch ( fmt )
    {
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            bc = true;
            bpe = 8;
            break;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            bc = true;
            bpe = 16;
            break;

        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_YUY2:
            packed = true;
            bpe = 4;
            break;

        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            packed = true;
            bpe = 8;
            break;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
            planar = true;
            bpe = 2;
            break;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            planar = true;
            bpe = 4;
            break;
    }

    if ( bc )
    {
        size_t numBlocksWide = 0;
        if ( width > 0 )
        {
            numBlocksWide = std::max<size_t>( 1, ( width + 3 ) / 4 );
        }
        size_t numBlocksHigh = 0;
        if ( height > 0 )
        {
            numBlocksHigh = std::max<size_t>( 1, ( height + 3 ) / 4 );
        }
        rowBytes = numBlocksWide * bpe;
        numRows = numBlocksHigh;
        numBytes = rowBytes * numBlocksHigh;
    }
    else if ( packed )
    {
        rowBytes = ( ( width + 1 ) >> 1 ) * bpe;
        numRows = height;
        numBytes = rowBytes * height;
    }
    else if ( fmt == DXGI_FORMAT_NV11 )
    {
        rowBytes = ( ( width + 3 ) >> 2 ) * 4;
        numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
        numBytes = rowBytes * numRows;
    }
    else if ( planar )
    {
        rowBytes = ( ( width + 1 ) >> 1 ) * bpe;
        numBytes = ( rowBytes * height ) + ( ( rowBytes * height + 1 ) >> 1 );
        numRows = height + ( ( height + 1 ) >> 1 );
    }
    else
    {
        size_t bpp = BitsPerPixel( fmt );
        rowBytes = ( width * bpp + 7 ) / 8; // round up to nearest byte
        numRows = height;
        numBytes = rowBytes * height;
    }

    if ( outNumBytes )
    {
        *outNumBytes = numBytes;
    }
    if ( outRowBytes )
    {
        *outRowBytes = rowBytes;
    }
    if ( outNumRows )
    {
        *outNumRows = numRows;
    }
}

