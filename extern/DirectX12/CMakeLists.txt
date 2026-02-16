cmake_minimum_required(VERSION 3.16.1)

set( D3D12_LIB "d3d12.lib" ) # This is required to get the Agility SDK to find the D3D12.lib file?

# Find the DirectX 12 Agility SDK package from vcpkg
find_package(directx-headers CONFIG REQUIRED GLOBAL)
find_package(directx12-agility CONFIG REQUIRED GLOBAL)

# Create an interface library that other targets can link against
add_library(DirectX12 INTERFACE)
add_library(DirectX12::DirectX12 ALIAS DirectX12)

# Link both the Windows SDK D3D12 and the Agility SDK
target_link_libraries(DirectX12
    INTERFACE
        Microsoft::DirectX-Headers
        Microsoft::DirectX-Guids
        Microsoft::DirectX12-Agility
        d3d12.lib
        dxgi.lib
        dxguid.lib
)

# The DirectX 12 and Agility SDK are now available to any target
# that links against DirectX12::DirectX12
