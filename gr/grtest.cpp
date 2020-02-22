#include <iostream>
#include <chrono>
#include <filesystem>
#include "glm/glm.hpp"
#include "stb/stb_image.h"
#include "tinygltf/tiny_gltf.h"
#include "util/rect.h"
#include "util/time.h"
#include "util/file.h"
#include "platform/platform.h"
#include "gr1/host.h"
#include "gr1/device.h"
#include "gr1/execution_queue.h"
#include "gr1/image.h"
#include "gr1/shader.h"
#include "gr1/buffer.h"
#include "gr1/sampler.h"
#include "gr1/render_pass.h"
#include "gr1/render_state.h"
#include "gr1/utl/gltf.h"
#include "gr1/utl/font.h"
#include <string>

#if defined(_MSC_VER) && defined(_DEBUG)

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

static struct DbgInit {
  DbgInit() 
  {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

		//_CrtSetBreakAlloc(19282);
  }
} dbgInit;

#endif

#if defined(_WIN32)
#include "gr1/win32/presentation_surface_create_data_win32.h"
#include "platform/win32/window_win32.h"
#endif

using namespace std;
using namespace glm;
using namespace platform;
using namespace gr1;

shared_ptr<Shader> LoadShader(Device *device, std::string name)
{
	vector<uint8_t> contents = util::ReadFile(string("../data/") + name);
	string ext = name.substr(name.find_last_of('.'));
	ShaderKind kind = ext == ".vert" ? ShaderKind::Vertex : ShaderKind::Fragment;
	shared_ptr<Shader> shader = device->CreateResource<Shader>();
	shader->Init(name, kind, contents);
	return shader;
}

shared_ptr<Image> LoadImage(Device *device, std::string name)
{
	std::string path = string("../data/") + name;
	int width, height, channels;
	uint8_t *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	ASSERT(channels == 4);

	auto image = device->CreateResource<Image>();
	image->Init(Image::Usage::Texture, ColorFormat::R8G8B8A8, glm::uvec4(width, height, 0, 0), 1);

	auto stagingFormat = util::CreateLayoutArray(rttr::type::get<uint8_t>(), height, width, 4);
	auto staging = device->CreateResource<Buffer>();
	staging->Init(Buffer::Usage::Staging, stagingFormat);

	void *mapped = staging->Map();
	memcpy(mapped, data, stagingFormat->GetSize());
	staging->Unmap();

	stbi_image_free(data);

	auto copyPass = device->CreateResource<ImageBufferCopyPass>();
	copyPass->Init(staging, image);

	device->GetExecutionQueue().EnqueuePass(copyPass);

	return image;
}

std::shared_ptr<Model> LoadModel(Device *device, std::string name)
{
	std::string path = string("../data/models/") + name;
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string err, warn;

	if (!loader.LoadASCIIFromFile(&model, &err, &warn, path)) {
		cout << "Gltf loading failed for " << path << ", error: " << err << " warning: " << warn << endl;
		return nullptr;
	}
	
	cout << "Gltf loading succeeded for " << path << ", error: " << err << " warning: " << warn << endl;

	static std::unordered_map<std::string, std::string> remapAttr{ {
		{ "POSITION", "inPosition" },
		{ "NORMAL", "inNormal" },
		{ "TANGENT", "inTangent" },
		{ "TEXCOORD_0", "inTexCoord" },
	} };

	std::shared_ptr<Model> result = LoadGltfModel(*device, model, remapAttr);

	return result;
}

std::shared_ptr<Font> LoadFont(Device *device, std::string name)
{
	string path = string("../data/fonts/") + name;
	shared_ptr<vector<uint8_t>> fontData = make_shared<vector<uint8_t>>();
	*fontData = util::ReadFile(path);
	shared_ptr<Font> font = make_shared<Font>(*device);
	font->Init(fontData, 0, 32, { {32, 128} });
	return font;
}

std::shared_ptr<RenderDrawCommand> InitFontDraw(Device *device, std::shared_ptr<Font> const &font, std::string shaderName)
{
	auto shaderVert = LoadShader(device, shaderName + ".vert");
	auto shaderFrag = LoadShader(device, shaderName + ".frag");

	auto sampler = device->CreateResource<Sampler>();
	sampler->Init();

	font->SetRenderingData(1024, "inPosition", "inTexCoord", "inColor");

	auto renderState = device->CreateResource<RenderState>();
	renderState->Init();
	renderState->SetAttachmentBlendState(0, true,
		RenderState::BlendFactor::SrcAlpha, RenderState::BlendFactor::OneMinusSrcAlpha, RenderState::BlendFunc::Add,
		RenderState::BlendFactor::SrcAlpha, RenderState::BlendFactor::OneMinusSrcAlpha, RenderState::BlendFunc::Add, 
		RenderState::ColorComponentMask::RGBA);

	auto &uniformInfo = shaderVert->GetUniformBuffers().front();
	auto samplerInfo = shaderFrag->GetSamplers().front();

	auto uniforms = device->CreateResource<Buffer>();
	uniforms->Init(Buffer::Usage::Uniform, uniformInfo._layout);

	auto drawFontCmd = device->CreateResource<RenderDrawCommand>();
	drawFontCmd->SetShader(shaderVert);
	drawFontCmd->SetShader(shaderFrag);
	drawFontCmd->SetRenderState(renderState);
	drawFontCmd->AddBuffer(uniforms, uniformInfo._binding);
	drawFontCmd->AddSampler(sampler, nullptr, samplerInfo._binding);

	return drawFontCmd;
}

void UpdateFontTransform(std::shared_ptr<RenderDrawCommand> const &fontDraw, glm::uvec2 screenSize, glm::vec4 fontColor)
{
	shared_ptr<Buffer> uniforms;
	for (int i = 0; i < fontDraw->GetBufferCount(); ++i) {
		auto &bufData = fontDraw->GetBufferData(i);
		if (bufData.IsUniform()) {
			uniforms = bufData._buffer;
			break;
		}
	}

	Device *device = fontDraw->GetDevice();
	auto staging = device->CreateResource<Buffer>();
	staging->Init(Buffer::Usage::Staging, uniforms->GetBufferLayout());

	glm::mat4 transform(1);
	transform[0].x = 2.0f / screenSize.x;
	transform[3].x = -1;

	transform[1].y = 2.0f / screenSize.y;
	transform[3].y = -1;

	void *mapped = staging->Map();

	*staging->GetBufferLayout()->GetMemberPtr<glm::mat4>(mapped, "transform") = transform;
	*staging->GetBufferLayout()->GetMemberPtr<glm::vec4>(mapped, "fontColor") = fontColor;

	staging->Unmap();

	auto copyPass = device->CreateResource<BufferCopyPass>();
	copyPass->Init(staging, uniforms);

	device->GetExecutionQueue().EnqueuePass(copyPass);
}

void InitTriangleVertices(Device *device, std::shared_ptr<Buffer> const &vertexBuffer)
{
	auto vertexStaging = device->CreateResource<Buffer>();
	vertexStaging->Init(Buffer::Usage::Staging, vertexBuffer->GetBufferLayout());

	util::LayoutElement *layout = vertexStaging->GetBufferLayout().get();
	void *mapped = vertexStaging->Map();

	*layout->GetMemberPtr<glm::vec3>(mapped, 0, "inPosition") = glm::vec3(0.0f, -0.5f, 0.0f);
	*layout->GetMemberPtr<glm::vec3>(mapped, 0, "inColor") = glm::vec3(1.0f, 0.0f, 0.0f);
	*layout->GetMemberPtr<glm::vec2>(mapped, 0, "inTexCoord") = glm::vec2(1.0f, 1.0f);

	*layout->GetMemberPtr<glm::vec3>(mapped, 1, "inPosition") = glm::vec3(0.5f, 0.5f, 0.0f);
	*layout->GetMemberPtr<glm::vec3>(mapped, 1, "inColor") = glm::vec3(0.0f, 1.0f, 0.0f);
	*layout->GetMemberPtr<glm::vec2>(mapped, 1, "inTexCoord") = glm::vec2(1.0f, 0.0f);

	*layout->GetMemberPtr<glm::vec3>(mapped, 2, "inPosition") = glm::vec3(-0.5f, 0.5f, 0.0f);
	*layout->GetMemberPtr<glm::vec3>(mapped, 2, "inColor") = glm::vec3(0.0f, 0.0f, 1.0f);
	*layout->GetMemberPtr<glm::vec2>(mapped, 2, "inTexCoord") = glm::vec2(0.0f, 1.0f);

	vertexStaging->Unmap();

	auto copyPass = device->CreateResource<BufferCopyPass>();
	copyPass->Init(vertexStaging, vertexBuffer);

	device->GetExecutionQueue().EnqueuePass(copyPass);
}

void InitTriangleStreams(Device *device, std::shared_ptr<Buffer> const &posBuffer, std::shared_ptr<Buffer> const &colorBuffer, std::shared_ptr<Buffer> const &texCoordBuffer)
{
	auto posStaging = device->CreateResource<Buffer>();
	posStaging->Init(Buffer::Usage::Staging, posBuffer->GetBufferLayout());

	auto colorStaging = device->CreateResource<Buffer>();
	colorStaging->Init(Buffer::Usage::Staging, colorBuffer->GetBufferLayout());

	auto texCoordStaging = device->CreateResource<Buffer>();
	texCoordStaging->Init(Buffer::Usage::Staging, texCoordBuffer->GetBufferLayout());

	util::LayoutElement *posLayout = posStaging->GetBufferLayout().get();
	util::LayoutElement *colorLayout = colorStaging->GetBufferLayout().get();
	util::LayoutElement *tcLayout = texCoordStaging->GetBufferLayout().get();
	void *positions = posStaging->Map();
	void *colors = colorStaging->Map();
	void *tc = texCoordStaging->Map();

	*posLayout->GetMemberPtr<glm::vec3>(positions, 0, "inPosition") = glm::vec3(0.0f, -0.5f, 0.0f);
	*colorLayout->GetMemberPtr<glm::vec3>(colors, 0, "inColor") = glm::vec3(1.0f, 0.0f, 0.0f);
	*tcLayout->GetMemberPtr<glm::vec2>(tc, 0, "inTexCoord") = glm::vec2(1.0f, 1.0f);

	*posLayout->GetMemberPtr<glm::vec3>(positions, 1, "inPosition") = glm::vec3(0.5f, 0.5f, 0.0f);
	*colorLayout->GetMemberPtr<glm::vec3>(colors, 1, "inColor") = glm::vec3(0.0f, 1.0f, 0.0f);
	*tcLayout->GetMemberPtr<glm::vec2>(tc, 1, "inTexCoord") = glm::vec2(1.0f, 0.0f);

	*posLayout->GetMemberPtr<glm::vec3>(positions, 2, "inPosition") = glm::vec3(-0.5f, 0.5f, 0.0f);
	*colorLayout->GetMemberPtr<glm::vec3>(colors, 2, "inColor") = glm::vec3(0.0f, 0.0f, 1.0f);
	*tcLayout->GetMemberPtr<glm::vec2>(tc, 2, "inTexCoord") = glm::vec2(0.0f, 1.0f);

	texCoordStaging->Unmap();
	colorStaging->Unmap();
	posStaging->Unmap();

	auto copyPosPass = device->CreateResource<BufferCopyPass>();
	copyPosPass->Init(posStaging, posBuffer);

	auto copyColorPass = device->CreateResource<BufferCopyPass>();
	copyColorPass->Init(colorStaging, colorBuffer);

	auto copyTcPass = device->CreateResource<BufferCopyPass>();
	copyTcPass->Init(texCoordStaging, texCoordBuffer);

	device->GetExecutionQueue().EnqueuePass(copyPosPass);
	device->GetExecutionQueue().EnqueuePass(copyColorPass);
	device->GetExecutionQueue().EnqueuePass(copyTcPass);
}

void InitTriangleIndices(Device *device, std::shared_ptr<Buffer> const &indexBuffer)
{
	auto indexStaging = device->CreateResource<Buffer>();
	indexStaging->Init(Buffer::Usage::Staging, indexBuffer->GetBufferLayout());

	util::LayoutElement *layout = indexStaging->GetBufferLayout().get();
	void *mapped = indexStaging->Map();

	*layout->GetMemberPtr<uint16_t>(mapped, 0) = 0;
	*layout->GetMemberPtr<uint16_t>(mapped, 1) = 2;
	*layout->GetMemberPtr<uint16_t>(mapped, 2) = 1;

	*layout->GetMemberPtr<uint16_t>(mapped, 3) = 0;
	*layout->GetMemberPtr<uint16_t>(mapped, 4) = 1;
	*layout->GetMemberPtr<uint16_t>(mapped, 5) = 2;

	indexStaging->Unmap();

	auto copyPass = device->CreateResource<BufferCopyPass>();
	copyPass->Init(indexStaging, indexBuffer);

	device->GetExecutionQueue().EnqueuePass(copyPass);
}

void InitPerInstanceColor(Device *device, std::shared_ptr<Buffer> const &vertexBuffer)
{
	auto vertexStaging = device->CreateResource<Buffer>();
	vertexStaging->Init(Buffer::Usage::Staging, vertexBuffer->GetBufferLayout());

	util::LayoutElement *layout = vertexStaging->GetBufferLayout().get();
	void *mapped = vertexStaging->Map();

	*layout->GetMemberPtr<glm::vec3>(mapped, 0, "inColor") = glm::vec3(1.0f, 1.0f, 1.0f);

	vertexStaging->Unmap();

	auto copyPass = device->CreateResource<BufferCopyPass>();
	copyPass->Init(vertexStaging, vertexBuffer);

	device->GetExecutionQueue().EnqueuePass(copyPass);
}
void UpdateTransforms(std::shared_ptr<Buffer> const &buffer, glm::mat4 model, glm::mat4 view, glm::mat4 proj)
{
	util::LayoutElement *layout = buffer->GetBufferLayout().get();
	void *mapped = buffer->Map();
	*layout->GetMemberPtr<glm::mat4>(mapped, "model") = model;
	*layout->GetMemberPtr<glm::mat4>(mapped, "view") = view;
	*layout->GetMemberPtr<glm::mat4>(mapped, "proj") = proj;
	buffer->Unmap();
}

int main()
{
  auto platform = unique_ptr<Platform>(Platform::Create());

  cout << "Current execution directory: " << platform->CurrentDirectory() << endl;

  Window *window = platform->CreateWindow();

  window->SetName("gr test");

  auto ri = window->GetRect();
  ri.SetSize(ivec2(300, 300));
  window->SetRect(ri);

  //window->SetStyle(Window::Style::Borderless);

  window->SetShown(true);

  ri = window->GetRect();

	Host host;
	shared_ptr<Device> device = host.CreateDevice(0);


#if defined(_WIN32)
  auto windowWin32 = dynamic_cast<WindowWin32*>(window);
  PresentationSurfaceCreateDataWin32 surfaceData;
  surfaceData._hInstance = windowWin32->GetPlatformWin32()->_hInstance;
  surfaceData._hWnd = windowWin32->_hWnd;
#endif

	auto vertShader = LoadShader(device.get(), "simple.vert");
	auto fragShader = LoadShader(device.get(), "simple.frag");

	auto texture = LoadImage(device.get(), "grid2.png");

	auto mesh = LoadModel(device.get(), "Cube.gltf");
	
	auto font = LoadFont(device.get(), "Lato-Regular.ttf");
	auto fontDraw = InitFontDraw(device.get(), font, "font");
	UpdateFontTransform(fontDraw, ri.GetSize(), glm::vec4(1));
	glm::vec2 textPos(100, 100);
	font->AddText(textPos, "The quick brown fox");
	font->SetDataToDrawCommand(fontDraw.get());

	auto vbLayout = std::make_shared<util::LayoutArray>(vertShader->GetVertexLayout(), 3);
	auto vertexBuffer = device->CreateResource<Buffer>();
	vertexBuffer->Init(Buffer::Usage::Vertex, vbLayout);

	InitTriangleVertices(device.get(), vertexBuffer);

	auto vbPositionsLayout = util::CreateLayoutArray(util::CreateLayoutStruct("inPosition", rttr::type::get<glm::vec3>()), 3);
	auto vbColorsLayout = util::CreateLayoutArray(util::CreateLayoutStruct("inColor", rttr::type::get<glm::vec3>()), 3);
	auto vbTexCoordsLayout = util::CreateLayoutArray(util::CreateLayoutStruct("inTexCoord", rttr::type::get<glm::vec2>()), 3);

	auto posStream = device->CreateResource<Buffer>();
	posStream->Init(Buffer::Usage::Vertex, vbPositionsLayout);
	auto colorStream = device->CreateResource<Buffer>();
	colorStream->Init(Buffer::Usage::Vertex, vbColorsLayout);
	auto texCoordStream = device->CreateResource<Buffer>();
	texCoordStream->Init(Buffer::Usage::Vertex, vbTexCoordsLayout);

	InitTriangleStreams(device.get(), posStream, colorStream, texCoordStream);

	auto vbPerInstanceColor = util::CreateLayoutArray(vbColorsLayout->GetArrayElement()->AsShared(), 1);
	auto perInstanceColor = device->CreateResource<Buffer>();
	perInstanceColor->Init(Buffer::Usage::Vertex, vbPerInstanceColor);
	InitPerInstanceColor(device.get(), perInstanceColor);
	mesh->_buffers.push_back({ perInstanceColor, true });

	auto ibLayout = std::make_shared<util::LayoutArray>(std::make_shared<util::LayoutValue>(rttr::type::get<uint16_t>()), 6);
	auto indexBuffer = device->CreateResource<Buffer>();
	indexBuffer->Init(Buffer::Usage::Index, ibLayout);

	InitTriangleIndices(device.get(), indexBuffer);

	auto renderState = device->CreateResource<RenderState>();
	renderState->Init();
	renderState->SetCullState(RenderState::FrontFaceMode::CCW, RenderState::CullMask::Back);
	renderState->SetDepthState(true, true, CompareFunc::Less);
	//renderState->SetScissor(util::RectI{ { 0, 0 }, { 1024, 1024 } });

	auto sampler = device->CreateResource<Sampler>();
	sampler->Init();

	mat4 model(1.0f), view(1.0f), proj(1.0f);
	view = glm::translate(view, vec3(0, 0, 5.5f));

	auto uboShader = device->CreateResource<Buffer>();
	uboShader->Init(Buffer::Usage::Uniform, vertShader->GetUniformBuffers()[0]._layout);

	auto uboStaging = device->CreateResource<Buffer>();
	uboStaging->Init(Buffer::Usage::Staging, uboShader->GetBufferLayout());

	auto uboUpdatePass = device->CreateResource<BufferCopyPass>();
	uboUpdatePass->Init(uboStaging, uboShader);

	auto renderTriangle = device->CreateResource<RenderDrawCommand>();
	renderTriangle->SetShader(vertShader);
	renderTriangle->SetShader(fragShader);
	renderTriangle->SetRenderState(renderState);
	//renderTriangle->AddBuffer(vertexBuffer);
	//renderTriangle->AddBuffer(posStream, 0);
	//renderTriangle->AddBuffer(colorStream, 1);
	//renderTriangle->AddBuffer(perInstanceColor, 1, 0, true);
	//renderTriangle->AddBuffer(texCoordStream, 2);
	//renderTriangle->AddBuffer(indexBuffer);
	renderTriangle->AddBuffer(uboShader);
	//renderTriangle->AddSampler(sampler, texture, 1);
	renderTriangle->AddSampler(sampler, font->_texture, 1);
	mesh->SetToDrawCommand(renderTriangle);
	//renderTriangle->SetDrawCounts(static_cast<uint32_t>(vertexBuffer->GetBufferLayout()->GetArrayCount()));

	auto surface = device->CreateResource<PresentationSurface>();
	surface->Init(surfaceData, PresentMode::Immediate);
	surface->Update(ri.GetSize().x, ri.GetSize().y);
	proj = glm::perspectiveLH<float>(glm::pi<float>() / 3, static_cast<float>(ri.GetSize().x) / ri.GetSize().y, 0.1f, 100.0f);


	auto depthBuffer = device->CreateResource<Image>();
	depthBuffer->Init(Image::Usage::DepthBuffer, ColorFormat::D24S8, uvec4(ri.GetSize().x, ri.GetSize().y, 0, 0), 1);

	auto renderPass = device->CreateResource<RenderPass>();
	renderPass->AddAttachment(ContentTreatment::Clear, ContentTreatment::Keep, vec4(0, 0, 1, 1));
	renderPass->AddAttachment(ContentTreatment::Clear, ContentTreatment::Keep, vec4(1, 0, 0, 0));
	renderPass->SetAttachmentImage(1, depthBuffer);

	renderPass->AddCommand(renderTriangle);
	renderPass->AddCommand(fontDraw);

	auto presentPass = device->CreateResource<PresentPass>();
	presentPass->Init(surface);


	window->SetRectUpdatedFunc([&](Window *window, Window::Rect rect) {
		LOG("Window rect changed ", util::ToString(rect._min), util::ToString(rect._max));
		auto size = surface->GetSize();
		if (size.x > 0 && size.y > 0) {
			surface->Update(size.x, size.y);
			proj = glm::perspectiveLH<float>(glm::pi<float>() / 3, static_cast<float>(size.x) / size.y, 0.1f, 100.0f);
			depthBuffer = device->CreateResource<Image>();
			depthBuffer->Init(Image::Usage::DepthBuffer, ColorFormat::D24S8, uvec4(size.x, size.y, 0, 0), 1);
			renderPass->SetAttachmentImage(1, depthBuffer);
			UpdateFontTransform(fontDraw, size, glm::vec4(1));
		}
	});

  {
    auto start = chrono::system_clock::now();
		uint32_t frameNumber = 0;

    while (!window->ShouldClose()) {
      if (window->GetInput().IsJustPressed(Key::Enter)) {
        auto newStyle = window->GetStyle() == Window::Style::CaptionedResizeable
          ? Window::Style::BorderlessFullscreen
          : Window::Style::CaptionedResizeable;

        window->SetStyle(newStyle);
      }

      auto text = window->GetInput()._input;
      if (text.size() > 0) {
        LOG("Input ", text);
      }

			auto size = surface->GetSize();
			if (size.x > 0 && size.y > 0) {
				auto backBuffer = surface->AcquireNextImage();
				renderPass->SetAttachmentImage(0, backBuffer);

				presentPass->SetImageToPresent(backBuffer);

				auto time = util::ToSeconds(chrono::system_clock::now() - start);
				model = glm::rotate(glm::identity<glm::mat4>(), time, glm::vec3(0, 1, 0));
				UpdateTransforms(uboStaging, model, view, proj);

				device->GetExecutionQueue().EnqueuePass(uboUpdatePass);
				device->GetExecutionQueue().EnqueuePass(renderPass);
				device->GetExecutionQueue().EnqueuePass(presentPass);
				device->GetExecutionQueue().ExecutePasses();
			}

      platform->Update();
			++frameNumber;
    }

		float seconds = util::ToSeconds(chrono::system_clock::now() - start);
		cout << "Quitting after " << seconds << " seconds and " << frameNumber << " frames for " << frameNumber / seconds << " FPS" << endl;
  }

  return 0;
}