#include "stdafx.h"
#include <array>
#include <iostream>
#include <chrono>
#include "Vk.h"
#include "VGraphics.h"

#ifdef _WIN32
  #include <iostream>
  #include "OutputDebugBuf.h"
#endif

#ifdef _WIN32
  #define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include "../glfw/glfw3.h"
#include "../glfw/glfw3native.h"
#include "../glm/matrix.hpp"

struct AppData {
  std::unique_ptr<VGraphics>      vg;

  std::shared_ptr<VMaterial>      material;
  std::shared_ptr<VModel>         model;
  std::shared_ptr<VModelInstance> modelInstance;
};

struct VertexFormat {
  glm::vec3 position;
  glm::vec3 color;
};

struct UniformBuffer {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

void InitMaterial(AppData &data)
{
  std::vector<std::shared_ptr<VShader>> shaders;
  shaders.push_back(std::shared_ptr<VShader>(data.vg->m_device->LoadVShader("../data/simple.vert.spv")));
  shaders.push_back(std::shared_ptr<VShader>(data.vg->m_device->LoadVShader("../data/simple.frag.spv")));
  data.material = std::make_shared<VMaterial>(shaders);
  UniformBuffer uniform;
  data.material->m_uniformContent.resize(1);
  data.material->m_uniformContent[0].resize(sizeof(uniform));
  memcpy(data.material->m_uniformContent[0].data(), &uniform, sizeof(uniform));
}

void InitModel(AppData &data)
{
  std::array<VertexFormat, 3> vertices{
    VertexFormat{ glm::vec3(  0.0f, -0.5f, 0.5f ), glm::vec3( 1.0f, 0.0f, 0.0f ) },
    VertexFormat{ glm::vec3(  0.5f,  0.5f, 0.5f ), glm::vec3( 0.0f, 1.0f, 0.0f ) },
    VertexFormat{ glm::vec3( -0.5f,  0.5f, 0.5f ), glm::vec3( 0.0f, 0.0f, 1.0f ) }
  };

  std::array<uint16_t, 6> indices{ 0, 2, 1 };

  std::shared_ptr<VVertexBuffer> vb = std::make_shared<VVertexBuffer>(*data.vg->m_device, vertices.size() * sizeof(VertexFormat), vertices.data());
  vb->m_vertexInfo->m_stride = sizeof(VertexFormat);
  vb->m_vertexInfo->AddAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexFormat, position));
  vb->m_vertexInfo->AddAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexFormat, color));
  
  std::shared_ptr<VIndexBuffer> ib = std::make_shared<VIndexBuffer>(*data.vg->m_device, indices.size(), indices.data());

  std::shared_ptr<VGeometry> geom = std::make_shared<VGeometry>(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, ib, std::vector<std::shared_ptr<VVertexBuffer>>{vb});

  data.model = std::make_shared<VModel>(geom, data.material);
}

void InitModelInstance(AppData &data)
{
  data.modelInstance = std::make_shared<VModelInstance>(data.model);
}

void FramebufferSizeChanged(GLFWwindow *window, int width, int height)
{
  AppData *data = static_cast<AppData*>(glfwGetWindowUserPointer(window));
  if (data) 
    data->vg->m_device->InitSwapchain(width, height);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
#ifdef _WIN32
  static OutputDebugStringBuf<char> charDebugOutput;
  std::cerr.rdbuf(&charDebugOutput);
  std::clog.rdbuf(&charDebugOutput);
#endif

  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window = glfwCreateWindow(1024, 768, "Vulkan Graphics", nullptr, nullptr);

  try {
    auto data = std::make_unique<AppData>();
    data->vg.reset(new VGraphics(true, "VGraphics", 1, (uintptr_t)hInstance, (uintptr_t)glfwGetWin32Window(window)));

    glfwSetWindowUserPointer(window, data.get());
    glfwSetFramebufferSizeCallback(window, FramebufferSizeChanged);

    InitMaterial(*data);
    InitModel(*data);
    InitModelInstance(*data);

/*    VImage *img = (*data->vg->m_device).LoadVImage("../../Terrain/Data/Textures/Earth.bmp");
    delete img;

    std::vector<char> trash(1024);
    VBuffer *buf = (*data->vg->m_device).LoadVBuffer(trash.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, trash.data());
    delete buf;

    auto ib = std::make_shared<VIndexBuffer>(*data->vg->m_device, 128, (uint16_t*)nullptr);
    auto vb = std::make_shared<VVertexBuffer>(*data->vg->m_device, 1024, nullptr);
    auto geom = std::make_shared<VGeometry>(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, ib, std::vector<std::shared_ptr<VVertexBuffer>>{vb});
*/
    size_t frames = 0;
    auto start = std::chrono::system_clock::now();

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      data->vg->m_device->Add(data->modelInstance);
      data->vg->m_device->RenderFrame();
      ++frames;
    }

    auto end = std::chrono::system_clock::now();
    double fps = (double)(frames / std::chrono::duration_cast<std::chrono::seconds>(end - start).count());
    std::clog << "FPS: " << fps << std::endl;

    data->vg->m_device->WaitIdle();
  } catch (VGraphicsException &vgEx) {
    int i = 0;
  }

  glfwTerminate();
  return 0;
}