/**
 * A mesh class encapsulates the index and vertex buffers for a geometric primitive.
 */

#include <memory> // For std::unique_ptr
#include <vector>

#include <DirectXMath.h>
#include <d3d12.h>

#include <wrl.h>

 // Vertex struct holding position, normal vector, and texture mapping information.
struct VertexPositionNormalTexture
{
    VertexPositionNormalTexture()
    { }

    VertexPositionNormalTexture(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& normal, const DirectX::XMFLOAT2& textureCoordinate)
        : position(position),
        normal(normal),
        textureCoordinate(textureCoordinate)
    { }

    VertexPositionNormalTexture(DirectX::FXMVECTOR position, DirectX::FXMVECTOR normal, DirectX::FXMVECTOR textureCoordinate)
    {
        XMStoreFloat3(&this->position, position);
        XMStoreFloat3(&this->normal, normal);
        XMStoreFloat2(&this->textureCoordinate, textureCoordinate);
    }

    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 textureCoordinate;

    static const int InputElementCount = 3;
    static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

using VertexCollection = std::vector<VertexPositionNormalTexture>;
using IndexCollection = std::vector<uint16_t>;

class Mesh
{
public:

    void Draw(ID3D12GraphicsCommandList* pDeviceContext);

    static std::unique_ptr<Mesh> CreateCube(ID3D12GraphicsCommandList* commandList, float size = 1, bool rhcoords = true);
    static std::unique_ptr<Mesh> CreateSphere(ID3D12GraphicsCommandList* commandList, float diameter = 1, size_t tessellation = 16, bool rhcoords = true);
    static std::unique_ptr<Mesh> CreateCone(ID3D12GraphicsCommandList* commandList, float diameter = 1, float height = 1, size_t tessellation = 32, bool rhcoords = true);
    static std::unique_ptr<Mesh> CreateTorus(ID3D12GraphicsCommandList* commandList, float diameter = 1, float thickness = 0.333f, size_t tessellation = 32, bool rhcoords = true);

protected:

private:
    friend struct std::default_delete<Mesh>;

    Mesh();
    Mesh(const Mesh& copy);
    virtual ~Mesh();

    void Initialize(ID3D12GraphicsCommandList* deviceContext, VertexCollection& vertices, IndexCollection& indices, bool rhcoords);

    Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBufferUpload;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBufferUpload;

    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

    UINT m_IndexCount;
};