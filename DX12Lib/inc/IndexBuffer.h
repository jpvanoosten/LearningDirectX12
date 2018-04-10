/**
 * Index buffer resource.
 */

#include "Buffer.h"

class IndexBuffer : public Buffer
{
public:
    IndexBuffer( const std::wstring& name = L"");
    IndexBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initialResourceState);
    virtual ~IndexBuffer();

    // Inherited from Buffer
    virtual void CreateViews(size_t numElements, size_t elementSize) override;

    size_t GetNumIndicies() const
    {
        return m_NumIndicies;
    }

    DXGI_FORMAT GetIndexFormat() const
    {
        return m_IndexFormat;
    }

    /**
     * Get the index buffer view for biding to the Input Assembler stage.
     */
    D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
    {
        return m_IndexBufferView;
    }

protected:

private:
    size_t m_NumIndicies;
    DXGI_FORMAT m_IndexFormat;

    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};