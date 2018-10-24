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
 *  @file TextureUsage.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief The TextureUsage enumeration describes how a texture is used.
 *  Albedo (diffuse) textures should be loaded and stored using sRGB formats
 *  so that the texture sampler will automatically linearize the color when 
 *  sampled.
 *  Height maps and normals must must not be linearized during load and thus 
 *  must ignore any sRGB settings that they may contain in the metadata of the 
 *  image file.
 */


enum class TextureUsage
{
    Albedo,
    Diffuse = Albedo,       // Treat Diffuse and Albedo textures the same.
    Heightmap,
    Depth = Heightmap,      // Treat height and depth textures the same.
    Normalmap,
    RenderTarget,           // Texture is used as a render target.
};