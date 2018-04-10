/**
 * Abstract base class for buffer resource.
 */

#include "Resource.h"

class Buffer : public Resource
{
public:
    Buffer(const std::wstring& name = L"");
    Buffer(Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource, D3D12_RESOURCE_STATES initialResourceState);
    
    /**
     * Create the views for the buffer resource.
     * Used by the CommandList when setting the buffer contents.
     */
    virtual void CreateViews(size_t numElements, size_t elementSize) = 0;

protected:

private:

};