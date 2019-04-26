#include "graphics.h"

#ifdef GR_VK
#include "vk/graphics_vk.h"
#endif

namespace gr {

Graphics *Graphics::Create()
{
#ifdef GR_VK
  return new GraphicsVk();
#endif
}

Graphics::Graphics()
{
#ifdef _DEBUG
  _validationLevel = ValidationLevel::Full;
#endif
}

Graphics::~Graphics()
{
}

}