#pragma once

#include "glad/glad.h"

class Backbuffer {
public:
  ~Backbuffer();

  void Resize(uint32_t width, uint32_t height);
  void UpdateTexture();

protected:
  void CreateTexture(uint32_t width, uint32_t height);
  void DeleteTexture();

public:
  typedef Eigen::Array<uint32_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::DontAlign | Eigen::RowMajor> ImageArray;
  ImageArray m_Image;

  GLuint m_Texture = 0;
  uint32_t m_TextureSize;
  Eigen::Matrix4f m_Projection;
};