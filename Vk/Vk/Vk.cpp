#include "stdafx.h"
#include "Vk.h"
#include "VGraphics.h"

/*#include "App.h"
#include "Wnd.h"

int Main(App &app)
{
  app.m_name = "VGraphics";
  app.m_ver = 1;

  Window *wndMain = app.NewWindow("Vulkan Graphics");
  if (!wndMain->Init())
    return -1;

  VGraphics vg(true);

  if (!vg.Init(app.m_name, app.m_ver, app.GetID(), wndMain->GetID()))
    return -1;

  while (app.ProcessMessages()) {

  }

  vg.Done();

  return 0;
}

*/

#ifdef _WIN32
  #define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include "../glfw/glfw3.h"
#include "../glfw/glfw3native.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window = glfwCreateWindow(1024, 768, "Vulkan Graphics", nullptr, nullptr);

  try {
    VGraphics vg(true, "VGraphics", 1, (uintptr_t)hInstance, (uintptr_t)glfwGetWin32Window(window));

    VImage *img = (*vg.m_device).LoadVImage("../../Terrain/Data/Textures/Earth.bmp");
    delete img;

    std::vector<char> trash(1024);
    VBuffer *buf = (*vg.m_device).LoadVBuffer(trash.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, trash.data());
    delete buf;

    VShader *shader = (*vg.m_device).LoadVShader("/src/VulkanSDK/1.0.42.1/Bin/cube-vert.spv");
    delete shader;

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
    }
  }
  catch (VGraphicsException &vgEx) {
    int i = 0;
  }


  glfwTerminate();
  return 0;
}