#include <DX12LibPCH.h>

#include <RenderTarget.h>

RenderTarget::RenderTarget()
    : m_Textures(AttachmentPoint::NumAttachmentPoints)
{}

// Attach a texture to the render target.
// The texture will be copied into the texture array.
void RenderTarget::AttachTexture( AttachmentPoint attachmentPoint, const Texture& texture )
{
    m_Textures[attachmentPoint] = texture;
}

const Texture& RenderTarget::GetTexture( AttachmentPoint attachmentPoint ) const
{
    return m_Textures[attachmentPoint];
}

// Resize all of the textures associated with the render target.
void RenderTarget::Resize( uint32_t width, uint32_t height )
{
    for ( auto& texture : m_Textures )
    {
        texture.Resize( width, height );
    }
}

// Get a list of the textures attached to the render target.
// This method is primarily used by the CommandList when binding the
// render target to the output merger stage of the rendering pipeline.
const std::vector<Texture>& RenderTarget::GetTextures() const
{
    return m_Textures;
}
