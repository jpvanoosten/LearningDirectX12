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
 *  @file PipelineStateObject.h
 *  @date October 18, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief Wrapper for a ID3D12PipelineState object.
 */

#include <directx/d3d12.h>       // For D3D12_PIPELINE_STATE_STREAM_DESC, and ID3D12PipelineState
#include <wrl/client.h>  // For Microsoft::WRL::ComPtr

namespace dx12lib
{

class Device;

class PipelineStateObject
{
public:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> GetD3D12PipelineState() const
    {
        return m_d3d12PipelineState;
    }

protected:
    PipelineStateObject( Device& device, const D3D12_PIPELINE_STATE_STREAM_DESC& desc );
    virtual ~PipelineStateObject() = default;

private:
    Device&                                     m_Device;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_d3d12PipelineState;
};
}  // namespace dx12lib
