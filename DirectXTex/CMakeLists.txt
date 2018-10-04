cmake_minimum_required( VERSION 3.12.3 ) # Latest version of CMake when this file was created.

set( PUBLIC_HEADER_FILES 
	DirectXTex/DirectXTex.h
	DirectXTex/DirectXTex.inl
)

source_group( "Header Files" FILES ${PUBLIC_HEADER_FILES} )

set( PRIVATE_HEADER_FILES
	DirectXTex/BC.h
	DirectXTex/BCDirectCompute.h
	DirectXTex/d3dx12.h
	DirectXTex/DDS.h
	DirectXTex/DirectXTexP.h
	DirectXTex/Filters.h
	DirectXTex/scoped.h
)

set( SOURCE_FILES
	DirectXTex/DirectXTexWIC.cpp
	DirectXTex/BC.cpp
	DirectXTex/BC4BC5.cpp
	DirectXTex/BC6HBC7.cpp
	DirectXTex/BCDirectCompute.cpp
	DirectXTex/DirectXTexCompress.cpp
	DirectXTex/DirectXTexCompressGPU.cpp
	DirectXTex/DirectXTexConvert.cpp
	DirectXTex/DirectXTexD3D11.cpp
	DirectXTex/DirectXTexD3D12.cpp
	DirectXTex/DirectXTexDDS.cpp
	DirectXTex/DirectXTexFlipRotate.cpp
	DirectXTex/DirectXTexHDR.cpp
	DirectXTex/DirectXTexImage.cpp
	DirectXTex/DirectXTexMipmaps.cpp
	DirectXTex/DirectXTexMisc.cpp
	DirectXTex/DirectXTexNormalMaps.cpp
	DirectXTex/DirectXTexPMAlpha.cpp
	DirectXTex/DirectXTexResize.cpp
	DirectXTex/DirectXTexTGA.cpp
	DirectXTex/DirectXTexUtil.cpp
)

source_group( "Source Files" FILES ${PRIVATE_HEADER_FILES} ${SOURCE_FILES} )

set( SHADER_FILES
	DirectXTex/Shaders/BC6HEncode.hlsl
	DirectXTex/Shaders/BC7Encode.hlsl
)

source_group( "Source Files\\Shaders" FILES ${SHADER_FILES} )
set_source_files_properties( ${SHADER_FILES}
	PROPERTIES 
		HEADER_FILE_ONLY ON
)

add_library( DirectXTex STATIC
	${PUBLIC_HEADER_FILES}
	${PRIVATE_HEADER_FILES}
	${SOURCE_FILES}
	${SHADER_FILES}
)

target_include_directories( DirectXTex
	PUBLIC DirectXTex
)

add_custom_command( TARGET DirectXTex PRE_BUILD
	COMMAND DirectXTex/Shaders/CompileShaders.cmd
	WORKING_DIRECTORY DirectXTex/Shaders
)