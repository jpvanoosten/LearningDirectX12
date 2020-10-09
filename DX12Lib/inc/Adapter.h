#pragma once

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
 *  @file Adapter.h
 *  @date October 9, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief Wrapper for IDXGIAdapter
 */

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <memory>

class Adapter
{
public:
    /**
     * Create a GPU adapter.
     *
     * @param gpuPreference The GPU preference. By default, a high-performance
     * GPU is preferred.
     * @param useWarp If true, create a WARP adapter.
     *
     * @returns A shared pointer to a GPU adapter or nullptr if the adapter
     * could not be created.
     */
    static std::shared_ptr<Adapter>
        Create( DXGI_GPU_PREFERENCE gpuPreference =
                    DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                bool useWarp = false );

    /**
     * Get the IDXGIAdapter
     */
    Microsoft::WRL::ComPtr<IDXGIAdapter> GetDXGIAdapter() const
    {
        return m_dxgiAdapter;
    }

    /**
     * Get the description of the adapter.
     */
    const std::wstring GetDescription() const;

protected:
    Adapter( Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter );
    virtual ~Adapter() = default;

private:
    Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgiAdapter;
    DXGI_ADAPTER_DESC3                    m_Desc;
};
