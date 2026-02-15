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
 *  @file DX12LibPCH.h
 *  @date October 22, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Precompiled Header File for DX12Lib.
 */

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>  // For CommandLineToArgvW

// In order to define a function called CreateWindow, the Windows macro needs to
// be undefined.
#if defined( CreateWindow )
    #undef CreateWindow
#endif

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl/client.h>
using namespace Microsoft::WRL;

// DirectX 12 specific headers.
#include <directx/d3dx12.h>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>  // For ReportLiveObjects.

using namespace DirectX;

// STL Headers
#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#if defined( __cpp_lib_filesystem )
namespace fs = std::filesystem;
#else
namespace fs = std::experimental::filesystem;
#endif

// Assimp header files.
#include <assimp/Exporter.hpp>
#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>
#include <assimp/anim.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// Helper functions
#include <dx12lib/Helpers.h>
