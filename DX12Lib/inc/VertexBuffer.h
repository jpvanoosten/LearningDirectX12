/**
 * Vertex buffer resource.
 */

#include "Buffer.h"

class VertexBuffer : public Buffer
{
public:
    VertexBuffer(const std::wstring& name = L"");
    VertexBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES initialResourceState);
    virtual ~VertexBuffer();

    // Inherited from Buffer
    virtual void CreateViews(size_t numElements, size_t elementSize) override;

    /**
     * Get the vertex buffer view for binding to the Input Assembler stage.
     */
    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()
    {
        return m_VertexBufferView;
    }

    size_t GetNumVertices() const
    {
        return m_NumVertices;
    }

    size_t GetVertexStride() const
    {
        return m_VertexStride;
    }

protected:

private:
    size_t m_NumVertices;
    size_t m_VertexStride;

    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
};