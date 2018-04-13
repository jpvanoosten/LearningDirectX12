/**
 * Constant buffer resource.
 */
#pragma once

#include "Buffer.h"

class ConstantBuffer : public Buffer
{
public:
    ConstantBuffer(const std::wstring & name = L"");
    virtual ~ConstantBuffer();

    // Inherited from Buffer
    virtual void CreateViews(size_t numElements, size_t elementSize) override;

    size_t GetSizeInBytes() const
    {
        return m_SizeInBytes;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetConstantBufferView() const
    {
        return m_ConstantBufferView;
    }

protected:

private:
    size_t m_SizeInBytes;

    D3D12_CPU_DESCRIPTOR_HANDLE m_ConstantBufferView;
};