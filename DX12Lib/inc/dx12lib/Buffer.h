#pragma once

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
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
 *  @file Buffer.h
 *  @date October 22, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Abstract base class for buffer resources.
 */

#include "Resource.h"

class Buffer : public Resource
{
public:
    explicit Buffer(const std::wstring& name = L"");
    explicit Buffer( const D3D12_RESOURCE_DESC& resDesc,
        size_t numElements, size_t elementSize,
        const std::wstring& name = L"" );
    
    /**
     * Create the views for the buffer resource.
     * Used by the CommandList when setting the buffer contents.
     */
    virtual void CreateViews(size_t numElements, size_t elementSize) = 0;

protected:

private:

};