#pragma once

/*
 *  Copyright(c) 2020 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file VertexBufferView.h
 *  @date October 17, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief Wrapper for a Vertex Buffer View.
 */

#include <d3d12.h>  // For D3D12_VERTEX_BUFFER_VIEW
#include <memory>   // For std::shared_ptr

namespace dx12lib
{

class Device;
class VertexBuffer;

class VertexBufferView
{
public:
    std::shared_ptr<VertexBuffer> GetVertexBuffer() const
    {
        return m_VertexBuffer;
    }

    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
    {
        return m_VertexBufferView;
    }

protected:
    VertexBufferView( Device& device, std::shared_ptr<VertexBuffer> vertexBuffer );
    virtual ~VertexBufferView() = default;

private:
    Device&                       m_Device;
    std::shared_ptr<VertexBuffer> m_VertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW      m_VertexBufferView;
};
}  // namespace dx12lib
