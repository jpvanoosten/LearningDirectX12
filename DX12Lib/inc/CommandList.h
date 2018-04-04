/**
 * CommandList class encapsulates a ID3D12GraphicsCommandList2 interface.
 * The CommandList class provides additional functionality that makes working with 
 * DirectX 12 applications easier.
 */

#include <wrl.h>

#include <d3d12.h>

#include <vector>

class CommandList
{
public:
    
protected:

private:
    using D3D12ObjectList = std::vector < Microsoft::WRL::ComPtr<ID3D12Object> >;

    D3D12_COMMAND_LIST_TYPE m_CommandListType;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> m_d3d12GraphicsCommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_d3d12CommandAllocator;

    // Objects that are being referenced by a command list that is "in-flight" on 
    // the command-queue cannot be deleted. To ensure objects are not deleted 
    // until the command list is finished executing, a reference to the object
    // is stored. The referenced objects are released when the command list is 
    // reset.
    D3D12ObjectList m_ReferencedObjects;
};