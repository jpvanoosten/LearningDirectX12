cmake_minimum_required( VERSION 3.10.1 ) # Latest version of CMake when this file was created.

set( HEADER_FILES 
    inc/DX12LibPCH.h
    inc/Application.h
    inc/Buffer.h
    inc/ByteAddressBuffer.h
    inc/CommandList.h
    inc/CommandQueue.h
    inc/ConstantBuffer.h
    inc/DescriptorAllocator.h
    inc/DynamicDescriptorHeap.h
    inc/Game.h
	inc/GenerateMipsPSO.h
    inc/Helpers.h
    inc/HighResolutionClock.h
    inc/IndexBuffer.h
    inc/Mesh.h
    inc/Resource.h
    inc/ResourceStateTracker.h
    inc/RootSignature.h
    inc/StructuredBuffer.h
    inc/Texture.h
    inc/UploadBuffer.h
    inc/VertexBuffer.h
    inc/Window.h
    inc/d3dx12.h
)

set( SOURCE_FILES
    src/DX12LibPCH.cpp
    src/Application.cpp
    src/Buffer.cpp
    src/ByteAddressBuffer.cpp
    src/CommandQueue.cpp
    src/CommandList.cpp
    src/ConstantBuffer.cpp
    src/DescriptorAllocator.cpp
    src/DynamicDescriptorHeap.cpp
    src/Game.cpp
	src/GenerateMipsPSO.cpp
    src/HighResolutionClock.cpp
    src/IndexBuffer.cpp
    src/Mesh.cpp
    src/Resource.cpp
    src/ResourceStateTracker.cpp
    src/RootSignature.cpp
    src/StructuredBuffer.cpp
    src/Texture.cpp
    src/UploadBuffer.cpp
    src/VertexBuffer.cpp
    src/Window.cpp
)

set( DIRECTXTEX_HEADERS
    inc/DirectXTex/DDSTextureLoader12.h
    inc/DirectXTex/WICTextureLoader12.h
)

source_group( "Header Files\\DirectXTex" FILES ${DIRECTXTEX_HEADERS} )

set( DIRECTXTEX_SOURCE 
    src/DirectXTex/DDSTextureLoader12.cpp
    src/DirectXTex/WICTextureLoader12.cpp
)

source_group( "Source Files\\DirectXTex" FILES ${DIRECTXTEX_SOURCE} )

set( RESOURCE_FILES 
    DX12Lib.rc
    Resources/Icon/app_icon.ico
	Resources/Shaders/GenerateMips_CS.hlsl
)

source_group( "Resources" FILES DX12Lib.rc resource.h )
source_group( "Resources\\Icon" FILES Resources/Icon/app_icon.ico )
source_group( "Resources\\Shaders" FILES Resources/Shaders/GenerateMips_CS.hlsl )

add_library( DX12Lib STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${RESOURCE_FILES}
    ${DIRECTXTEX_HEADERS} ${DIRECTXTEX_SOURCE}
)

target_include_directories( DX12Lib
    PUBLIC inc
)

target_link_libraries( DX12Lib 
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC dxguid.lib
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/DX12Lib.dir/${CMAKE_CFG_INTDIR}/DX12Lib.res # This is the only way I could figure out how to link the compiled resource file.
)

# Enable precompiled header files.
set_target_properties( DX12Lib 
    PROPERTIES 
        COMPILE_FLAGS /Yu"DX12LibPCH.h"
)

set_source_files_properties( src/DX12LibPCH.cpp
    PROPERTIES
        COMPILE_FLAGS /Yc"DX12LibPCH.h"
)

set_source_files_properties( Resources/Shaders/GenerateMips_CS.hlsl
	PROPERTIES
		VS_SHADER_TYPE Compute
		VS_SHADER_MODEL 5.1
        VS_SHADER_OBJECT_FILE_NAME "${CMAKE_SOURCE_DIR}/Data/Shaders/%(Filename).cso"
		VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/inc/%(Filename).inc"
		VS_SHADER_VARIABLE_NAME g_GenerateMips_CS
	    VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:ON>
        VS_SHADER_ENABLE_DEBUG $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ON>
)