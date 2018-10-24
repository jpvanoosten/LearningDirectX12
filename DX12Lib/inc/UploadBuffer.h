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
 *  @file UploadBuffer.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief An UploadBuffer provides a convenient method to upload resources to the GPU.
 */

#include <Defines.h>

#include <wrl.h>
#include <d3d12.h>

#include <memory>
#include <deque>

class UploadBuffer
{
public:
    // Use to upload data to the GPU
    struct Allocation
    {
        void* CPU;
        D3D12_GPU_VIRTUAL_ADDRESS GPU;
    };

    /**
     * @param pageSize The size to use to allocate new pages in GPU memory.
     */
    explicit UploadBuffer(size_t pageSize = _2MB);

    virtual ~UploadBuffer();

    /**
     * The maximum size of an allocation is the size of a single page.
     */
    size_t GetPageSize() const { return m_PageSize;  }

    /**
     * Allocate memory in an Upload heap.
     * An allocation must not exceed the size of a page.
     * Use a memcpy or similar method to copy the 
     * buffer data to CPU pointer in the Allocation structure returned from 
     * this function.
     */
    Allocation Allocate(size_t sizeInBytes, size_t alignment);

    /**
     * Release all allocated pages. This should only be done when the command list
     * is finished executing on the CommandQueue.
     */
    void Reset();

private:
    // A single page for the allocator.
    struct Page
    {
        Page(size_t sizeInBytes);
        ~Page();

        // Check to see if the page has room to satisfy the requested
        // allocation.
        bool HasSpace(size_t sizeInBytes, size_t alignment ) const;
        
        // Allocate memory from the page.
        // Throws std::bad_alloc if the the allocation size is larger
        // that the page size or the size of the allocation exceeds the 
        // remaining space in the page.
        Allocation Allocate(size_t sizeInBytes, size_t alignment);

        // Reset the page for reuse.
        void Reset();

    private:

        Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;

        // Base pointer.
        void* m_CPUPtr;
        D3D12_GPU_VIRTUAL_ADDRESS m_GPUPtr;

        // Allocated page size.
        size_t m_PageSize;
        // Current allocation offset in bytes.
        size_t m_Offset;
    };

    // A pool of memory pages.
    using PagePool = std::deque< std::shared_ptr<Page> >;

    // Request a page from the pool of available pages
    // or create a new page if there are no available pages.
    std::shared_ptr<Page> RequestPage();

    PagePool m_PagePool;
    PagePool m_AvailablePages;

    std::shared_ptr<Page> m_CurrentPage;

    // The size of each page of memory.
    size_t m_PageSize;

};