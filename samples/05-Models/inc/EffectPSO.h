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
 *  @file EffectPSO.h
 *  @date November 12, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief EffectPSO is the base class for effect pipeline state objects (PSO).
 * The Effect class defines both the PSO and the root signature and allows for binding on a command list.
 */

#include <memory>

namespace dx12lib
{
class Device;
class CommandList;
class RootSignature;
class PipelineStateObject;
}

class EffectPSO
{
public:
    virtual void Apply( dx12lib::CommandList& commandList ) = 0;

protected:
    explicit EffectPSO(std::shared_ptr<dx12lib::Device> device);
    virtual ~EffectPSO() = default;

    std::shared_ptr<dx12lib::Device> m_Device;
    std::shared_ptr<dx12lib::RootSignature> m_RootSignature;
    std::shared_ptr<dx12lib::PipelineStateObject> m_PipelineStateObject;
};