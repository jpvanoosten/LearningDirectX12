/**
 * The TextureUsage enumeration describes how a texture is used.
 * Albedo (diffuse) textures should be loaded and stored using sRGB formats
 * so that the texture sampler will automatically linearize the color when sampled.
 * Height maps and normals must must not be linearized during load and thus must
 * ignore any sRGB settings that they may contain in the metadata of the image file.
 */
#pragma once

enum class TextureUsage
{
    Albedo,
    Diffuse = Albedo,       // Treat Diffuse and Albedo textures the same.
    Heightmap,
    Depth = Heightmap,      // Treat height and depth textures the same.
    Normalmap,
    RenderTarget,           // Texture is used as a render target.
};