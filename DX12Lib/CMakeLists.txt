cmake_minimum_required( VERSION 3.18.3 ) # Latest version of CMake when this file was created.

set( HEADER_FILES
    inc/dx12lib/Adapter.h
    inc/dx12lib/Buffer.h
    inc/dx12lib/ByteAddressBuffer.h
    inc/dx12lib/CommandList.h
    inc/dx12lib/CommandQueue.h
    inc/dx12lib/ConstantBuffer.h
    inc/dx12lib/ConstantBufferView.h
    inc/dx12lib/d3dx12.h
    inc/dx12lib/Defines.h
    inc/dx12lib/DepthStencilView.h
    inc/dx12lib/DescriptorAllocation.h
    inc/dx12lib/DescriptorAllocator.h
    inc/dx12lib/DescriptorAllocatorPage.h
    inc/dx12lib/Device.h
    inc/dx12lib/DynamicDescriptorHeap.h
    inc/dx12lib/GenerateMipsPSO.h
    inc/dx12lib/GUI.h
    inc/dx12lib/Helpers.h
    inc/dx12lib/IndexBuffer.h
    inc/dx12lib/IndexBufferView.h
    inc/dx12lib/Mesh.h
    inc/dx12lib/PanoToCubemapPSO.h
    inc/dx12lib/PipelineStateObject.h
    inc/dx12lib/RenderTarget.h
    inc/dx12lib/RenderTargetView.h
    inc/dx12lib/Resource.h
    inc/dx12lib/ResourceStateTracker.h
    inc/dx12lib/RootSignature.h
    inc/dx12lib/Scene.h
    inc/dx12lib/SceneNode.h
    inc/dx12lib/ShaderResourceView.h
    inc/dx12lib/StructuredBuffer.h
    inc/dx12lib/SwapChain.h
    inc/dx12lib/Texture.h
    inc/dx12lib/TextureUsage.h
    inc/dx12lib/ThreadSafeQueue.h
    inc/dx12lib/UnorderedAccessView.h
    inc/dx12lib/UploadBuffer.h
    inc/dx12lib/VertexBuffer.h
    inc/dx12lib/VertexBufferView.h

)

set( SOURCE_FILES
    src/DX12LibPCH.h
    src/DX12LibPCH.cpp
    src/Adapter.cpp
    src/Buffer.cpp
    src/ByteAddressBuffer.cpp
    src/CommandQueue.cpp
    src/CommandList.cpp
    src/ConstantBuffer.cpp
    src/ConstantBufferView.cpp
    src/DepthStencilView.cpp
    src/DescriptorAllocation.cpp
    src/DescriptorAllocator.cpp
    src/DescriptorAllocatorPage.cpp
    src/Device.cpp
    src/DynamicDescriptorHeap.cpp
    src/GenerateMipsPSO.cpp
    src/GUI.cpp
    src/IndexBuffer.cpp
    src/IndexBufferView.cpp
    src/Mesh.cpp
    src/PanoToCubemapPSO.cpp
    src/PipelineStateObject.cpp
    src/RenderTarget.cpp
    src/RenderTargetView.cpp
    src/Resource.cpp
    src/ResourceStateTracker.cpp
    src/RootSignature.cpp
    src/SceneNode.cpp
    src/ShaderResourceView.cpp
    src/StructuredBuffer.cpp
    src/SwapChain.cpp
    src/Texture.cpp
    src/UnorderedAccessView.cpp
    src/UploadBuffer.cpp
    src/VertexBuffer.cpp
    src/VertexBufferView.cpp
)

set( IMGUI_HEADERS
    inc/imgui.h
    inc/imconfig.h
    inc/imgui_impl_win32.h
)

source_group( "Header Files\\ImGUI" FILES ${IMGUI_HEADERS} )

set( IMGUI_SOURCE
    src/imgui.cpp
    src/imgui_demo.cpp
    src/imgui_draw.cpp
    src/imgui_widgets.cpp
    src/imgui_impl_win32.cpp
)

source_group( "Source Files\\ImGUI" FILES ${IMGUI_SOURCE} )

set( SHADER_FILES
    Resources/Shaders/GenerateMips_CS.hlsl
    Resources/Shaders/PanoToCubemap_CS.hlsl
    Resources/Shaders/ImGUI_VS.hlsl
    Resources/Shaders/ImGUI_PS.hlsl
)

set( RESOURCE_FILES 
    DX12Lib.rc
    Resources/Icon/app_icon.ico
	${SHADER_FILES}
)

source_group( "Resources" FILES DX12Lib.rc resource.h )
source_group( "Resources\\Icon" FILES Resources/Icon/app_icon.ico )
source_group( "Resources\\Shaders" FILES ${SHADER_FILES} )

add_library( DX12Lib STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${RESOURCE_FILES}
    ${IMGUI_HEADERS} ${IMGUI_SOURCE}
    ../.clang-format
)

# Enable C++17 compiler features.
target_compile_features( DX12Lib 
	PUBLIC cxx_std_17
)

target_compile_definitions( DX12Lib
    PUBLIC NOMINMAX
)

target_include_directories( DX12Lib
    PUBLIC inc
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/$(Platform)/$(Configuration)/Shaders"
)

target_link_libraries( DX12Lib 
	PUBLIC DirectXTex
    PUBLIC assimp
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
#    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/DX12Lib.dir/${CMAKE_CFG_INTDIR}/DX12Lib.res # This is the only way I could figure out how to link the compiled resource file.
)

# Enable precompiled header files.
set_source_files_properties( ${SOURCE_FILES} 
    PROPERTIES 
        COMPILE_FLAGS /Yu"DX12LibPCH.h"
)

set_source_files_properties( src/DX12LibPCH.cpp
    PROPERTIES
        COMPILE_FLAGS /Yc"DX12LibPCH.h"
)

set_source_files_properties( ${SHADER_FILES}
    PROPERTIES
        VS_SHADER_MODEL 5.1
        VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(Platform)/$(Configuration)/Shaders/%(Filename).h"
        VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
)

set_source_files_properties( Resources/Shaders/GenerateMips_CS.hlsl
    PROPERTIES
        VS_SHADER_TYPE Compute
        VS_SHADER_VARIABLE_NAME g_GenerateMips_CS
)

set_source_files_properties( Resources/Shaders/PanoToCubemap_CS.hlsl
    PROPERTIES
        VS_SHADER_TYPE Compute
        VS_SHADER_VARIABLE_NAME g_PanoToCubemap_CS
)

set_source_files_properties( Resources/Shaders/ImGUI_VS.hlsl
    PROPERTIES
        VS_SHADER_TYPE Vertex
        VS_SHADER_VARIABLE_NAME g_ImGUI_VS
)

set_source_files_properties( Resources/Shaders/ImGUI_PS.hlsl
    PROPERTIES
        VS_SHADER_TYPE Pixel
        VS_SHADER_VARIABLE_NAME g_ImGUI_PS
)
