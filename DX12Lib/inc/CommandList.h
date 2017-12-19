/**
 * Class that wraps a ID3D12GraphicsCommandList.
 */
#pragma once

#include <d3d12.h>
#include <wrl.h>

class CommandQueue;

class CommandList
{
public:
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList() const;

protected:
    friend class CommandQueue;

    CommandList(Microsoft::WRL::ComPtr<ID3D12Device2> device, 
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator, 
        D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandList();

    void Close();
    void Reset(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const;

private:

    Microsoft::WRL::ComPtr<ID3D12Device2>               m_d3d12Device;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>  m_d3d12CommandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>      m_d3d12CommandAllocator;
    D3D12_COMMAND_LIST_TYPE                             m_Type;
    bool                                                m_IsClosed;
};