#include "stdafx.h"

#include "Backbuffer.h"
#include "Util.h"

#undef max

Backbuffer::~Backbuffer()
{
  DeleteTexture();
}

void Backbuffer::Resize(uint32_t width, uint32_t height)
{
  DeleteTexture();
  CreateTexture(width, height);

  m_Projection = Orthographic(-1.0f, 2 * width / (float)m_TextureSize - 1,
    -1.0f, 2 * height / (float)m_TextureSize - 1,
    1.0f, -1.0f);

  m_Image.resize(height, width);
}

void Backbuffer::UpdateTexture()
{
  glBindTexture(GL_TEXTURE_2D, m_Texture);

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Image.cols(), m_Image.rows(), GL_RGBA, GL_UNSIGNED_BYTE, m_Image.data());

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Backbuffer::CreateTexture(uint32_t width, uint32_t height)
{
  m_TextureSize = NextPowerOf2(std::max(width, height));

  glGenTextures(1, &m_Texture);

  glBindTexture(GL_TEXTURE_2D, m_Texture);

  glTexImage2D(GL_TEXTURE_2D,
    0,
    GL_RGBA,
    m_TextureSize,
    m_TextureSize,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);
}

void Backbuffer::DeleteTexture()
{
    glDeleteTextures(1, &m_Texture);
}
