#include <iostream>
#include <chrono>
#include <filesystem>
#include "glm/glm.hpp"
#include "stb/stb_image.h"
#include "tinygltf/tiny_gltf.h"
#include "util/geom.h"
#include "util/time.h"
#include "util/file.h"
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
#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include <string>

#if defined(_MSC_VER) && !defined(NDEBUG)

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
#elif defined(__linux__)
#include "gr1/x11/presentation_surface_create_data_xlib.h"
#endif

using namespace std;
using namespace glm;
using namespace gr1;

static const string s_DataRoot("data/");

shared_ptr<Shader> LoadShader(Device *device, std::string name)
{
	string path = s_DataRoot + name;
	string ext = name.substr(name.find_last_of('.'));
	ShaderKind::Enum kind = ext == ".vert" ? ShaderKind::Vertex : ShaderKind::Fragment;
	shared_ptr<Shader> shader = device->CreateResource<Shader>();
	shader->Init(path, kind, nullptr);
	return shader;
}

shared_ptr<Image> LoadImage(Device *device, std::string name)
{
	std::string path = s_DataRoot + name;
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

static util::StrId 
	s_inPositionId("inPosition", util::StrId::AddToRepository), 
	s_inColorId("inColor", util::StrId::AddToRepository), 
	s_inTexCoordId("inTexCoord", util::StrId::AddToRepository),
	s_texSamplerId("texSampler", util::StrId::AddToRepository);


std::shared_ptr<Model> LoadModel(Device *device, std::string name)
{
	std::string path = s_DataRoot + string("models/") + name;
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string err, warn;

	if (!loader.LoadASCIIFromFile(&model, &err, &warn, path)) {
		cout << "Gltf loading failed for " << path << ", error: " << err << " warning: " << warn << endl;
		return nullptr;
	}
	
	cout << "Gltf loading succeeded for " << path << ", error: " << err << " warning: " << warn << endl;

	static std::unordered_map<std::string, std::string> modelToShader{{
		{ "POSITION"        , s_inPositionId.GetString() },
		{ "TEXCOORD_0"      , s_inTexCoordId.GetString() },
		{ "baseColorTexture", s_texSamplerId.GetString() },
	}};

	std::shared_ptr<Model> loadedModel = LoadGltfModel(*device, model, modelToShader);

	return loadedModel;
}

std::shared_ptr<gr1::Font> LoadFont(Device *device, std::string name)
{
	string path = s_DataRoot + string("fonts/") + name;
	shared_ptr<vector<uint8_t>> fontData = make_shared<vector<uint8_t>>();
	*fontData = util::ReadFile(path);
	shared_ptr<gr1::Font> font = make_shared<gr1::Font>(*device);
	font->Init(fontData, 0, 32, { {32, 128} });
	return font;
}

std::shared_ptr<RenderDrawCommand> InitFontDraw(Device *device, std::shared_ptr<gr1::Font> const &font, std::string shaderName)
{
	auto shaderVert = LoadShader(device, shaderName + ".vert");
	auto shaderFrag = LoadShader(device, shaderName + ".frag");

	auto sampler = device->CreateResource<Sampler>();
	sampler->Init();

	font->SetRenderingData(1024, s_inPositionId.GetString(), s_inTexCoordId.GetString(), s_inColorId.GetString());

	auto renderState = device->CreateResource<RenderState>();
	renderState->Init();
	renderState->SetAttachmentBlendState(0, true,
		BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFunc::Add,
		BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendFunc::Add, 
		ColorComponentMask::RGBA);

	auto &uniformInfo = shaderVert->GetParameters(Shader::Parameter::UniformBuffer).front();
	auto samplerInfo = shaderFrag->GetParameters(Shader::Parameter::Sampler).front();

	auto uniforms = device->CreateResource<Buffer>();
	uniforms->Init(Buffer::Usage::Uniform, uniformInfo._layout);

	auto drawFontCmd = device->CreateResource<RenderDrawCommand>();
	drawFontCmd->SetShader(shaderVert);
	drawFontCmd->SetShader(shaderFrag);
	drawFontCmd->SetRenderState(renderState);
	drawFontCmd->AddBuffer(uniforms, uniformInfo._id);
	drawFontCmd->AddSampler(sampler, nullptr, samplerInfo._id);

	return drawFontCmd;
}

static util::StrId s_transformId("transform"), s_fontColorId("fontColor");

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

	*staging->GetBufferLayout()->GetMemberPtr<glm::mat4>(mapped, s_transformId) = transform;
	*staging->GetBufferLayout()->GetMemberPtr<glm::vec4>(mapped, s_fontColorId) = fontColor;

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

	*layout->GetMemberPtr<glm::vec3>(mapped, 0, s_inPositionId) = glm::vec3(0.0f, -0.5f, 0.0f);
	*layout->GetMemberPtr<glm::vec3>(mapped, 0, s_inColorId) = glm::vec3(1.0f, 0.0f, 0.0f);
	*layout->GetMemberPtr<glm::vec2>(mapped, 0, s_inTexCoordId) = glm::vec2(1.0f, 1.0f);

	*layout->GetMemberPtr<glm::vec3>(mapped, 1, s_inPositionId) = glm::vec3(0.5f, 0.5f, 0.0f);
	*layout->GetMemberPtr<glm::vec3>(mapped, 1, s_inColorId) = glm::vec3(0.0f, 1.0f, 0.0f);
	*layout->GetMemberPtr<glm::vec2>(mapped, 1, s_inTexCoordId) = glm::vec2(1.0f, 0.0f);

	*layout->GetMemberPtr<glm::vec3>(mapped, 2, s_inPositionId) = glm::vec3(-0.5f, 0.5f, 0.0f);
	*layout->GetMemberPtr<glm::vec3>(mapped, 2, s_inColorId) = glm::vec3(0.0f, 0.0f, 1.0f);
	*layout->GetMemberPtr<glm::vec2>(mapped, 2, s_inTexCoordId) = glm::vec2(0.0f, 1.0f);

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

	*posLayout->GetMemberPtr<glm::vec3>(positions, 0, s_inPositionId) = glm::vec3(0.0f, -0.5f, 0.0f);
	*colorLayout->GetMemberPtr<glm::vec3>(colors, 0, s_inColorId) = glm::vec3(1.0f, 0.0f, 0.0f);
	*tcLayout->GetMemberPtr<glm::vec2>(tc, 0, s_inTexCoordId) = glm::vec2(1.0f, 1.0f);

	*posLayout->GetMemberPtr<glm::vec3>(positions, 1, s_inPositionId) = glm::vec3(0.5f, 0.5f, 0.0f);
	*colorLayout->GetMemberPtr<glm::vec3>(colors, 1, s_inColorId) = glm::vec3(0.0f, 1.0f, 0.0f);
	*tcLayout->GetMemberPtr<glm::vec2>(tc, 1, s_inTexCoordId) = glm::vec2(1.0f, 0.0f);

	*posLayout->GetMemberPtr<glm::vec3>(positions, 2, s_inPositionId) = glm::vec3(-0.5f, 0.5f, 0.0f);
	*colorLayout->GetMemberPtr<glm::vec3>(colors, 2, s_inColorId) = glm::vec3(0.0f, 0.0f, 1.0f);
	*tcLayout->GetMemberPtr<glm::vec2>(tc, 2, s_inTexCoordId) = glm::vec2(0.0f, 1.0f);

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

	*layout->GetMemberPtr<glm::vec3>(mapped, 0, s_inColorId) = glm::vec3(1.0f, 1.0f, 1.0f);

	vertexStaging->Unmap();

	auto copyPass = device->CreateResource<BufferCopyPass>();
	copyPass->Init(vertexStaging, vertexBuffer);

	device->GetExecutionQueue().EnqueuePass(copyPass);
}

util::StrId s_modelId("model"), s_viewId("view"), s_projId("proj");

void UpdateTransforms(std::shared_ptr<Buffer> const &buffer, glm::mat4 model, glm::mat4 view, glm::mat4 proj)
{
	util::LayoutElement *layout = buffer->GetBufferLayout().get();
	void *mapped = buffer->Map();
	*layout->GetMemberPtr<glm::mat4>(mapped, s_modelId) = model;
	*layout->GetMemberPtr<glm::mat4>(mapped, s_viewId) = view;
	*layout->GetMemberPtr<glm::mat4>(mapped, s_projId) = proj;
	buffer->Unmap();
}

int main(int argc, char *argv[])
{
	util::AutoFree<int> sdl(
		SDL_Init(SDL_INIT_EVERYTHING), 
		[](int) {
			SDL_Quit(); 
		});

	util::AutoFree<SDL_Window*> sdlWindow(
		SDL_CreateWindow("gr test SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 300, 300, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN), 
		[](SDL_Window *window) {
			SDL_DestroyWindow(window);
		});

	uint32_t sdlWindowId = SDL_GetWindowID(sdlWindow._data);
	glm::ivec2 winSize;
	SDL_GetWindowSize(sdlWindow._data, &winSize.x, &winSize.y);

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(sdlWindow._data, &wmInfo);
#if defined(_WIN32)
	PresentationSurfaceCreateDataWin32 surfaceData{wmInfo.info.win.hinstance, wmInfo.info.win.window};
#elif defined(__linux__)	
	PresentationSurfaceCreateDataXlib surfaceData{wmInfo.info.x11.display, wmInfo.info.x11.window};
#else
#	error Unsupported platform!
#endif

	Host host;
	shared_ptr<Device> device = host.CreateDevice(0, &surfaceData);

	auto vertShader = LoadShader(device.get(), "simple.vert");
	auto fragShader = LoadShader(device.get(), "simple.frag");

	auto texture = LoadImage(device.get(), "grid2.png");

	auto mesh = LoadModel(device.get(), "Cube.gltf");
	
	auto font = LoadFont(device.get(), "Lato-Regular.ttf");
	int fonts = gr1::Font::GetFontIndices(font->GetFontData());
	auto fontDraw = InitFontDraw(device.get(), font, "font");
	UpdateFontTransform(fontDraw, winSize, glm::vec4(1));
	glm::vec2 textPos(font->GetLineSpacing());
	glm::vec2 measurePos = textPos;
	std::string text(u8"The quick brown fox");
	auto rect = font->MeasureText(measurePos, text);
	font->AddText(textPos, text);
	ASSERT(glm::all(glm::equal(measurePos, textPos)));
	font->SetDataToDrawCommand(fontDraw.get());

	auto vbLayout = std::make_shared<util::LayoutArray>(vertShader->GetVertexLayout(), 3);
	auto vertexBuffer = device->CreateResource<Buffer>();
	vertexBuffer->Init(Buffer::Usage::Vertex, vbLayout);

	InitTriangleVertices(device.get(), vertexBuffer);

	auto vbPositionsLayout = util::CreateLayoutArray(util::CreateLayoutStruct(s_inPositionId.GetString(), rttr::type::get<glm::vec3>()), 3);
	auto vbColorsLayout = util::CreateLayoutArray(util::CreateLayoutStruct(s_inColorId.GetString(), rttr::type::get<glm::vec3>()), 3);
	auto vbTexCoordsLayout = util::CreateLayoutArray(util::CreateLayoutStruct(s_inTexCoordId.GetString(), rttr::type::get<glm::vec2>()), 3);

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
	renderState->SetCullState(FrontFaceMode::CCW, CullMask::Back);
	renderState->SetDepthState(true, true, CompareFunc::Less);
	//renderState->SetScissor(util::RectI{ { 0, 0 }, { 1024, 1024 } });

	auto sampler = device->CreateResource<Sampler>();
	sampler->Init();

	mat4 model(1.0f), view(1.0f), proj(1.0f);
	view = glm::translate(view, vec3(0, 0, 5.5f));

	auto uboShader = device->CreateResource<Buffer>();
	uboShader->Init(Buffer::Usage::Uniform, vertShader->GetParameters(Shader::Parameter::UniformBuffer)[0]._layout);

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
	renderTriangle->AddBuffer(uboShader, vertShader->GetParameters(Shader::Parameter::UniformBuffer).front()._id);
	renderTriangle->AddSampler(sampler, texture, fragShader->GetParameters(Shader::Parameter::Sampler).front()._id);
	//renderTriangle->AddSampler(sampler, font->GetTexture(), 1);
	mesh->SetToDrawCommand(renderTriangle);
	//renderTriangle->SetDrawCounts(static_cast<uint32_t>(vertexBuffer->GetBufferLayout()->GetArrayCount()));

	auto surface = device->CreateResource<PresentationSurface>();
	surface->Init(surfaceData);
	std::vector<PresentMode> desiredModes = { PresentMode::Immediate, PresentMode::Mailbox };
	surface->SetPresentMode(surface->GetFirstAvailablePresentMode(desiredModes));
	surface->Update(winSize.x, winSize.y);
	proj = glm::perspectiveLH<float>(glm::pi<float>() / 3, static_cast<float>(winSize.x) / winSize.y, 0.1f, 100.0f);


	auto depthBuffer = device->CreateResource<Image>();
	depthBuffer->Init(Image::Usage::DepthBuffer, ColorFormat::D24S8, uvec4(winSize.x, winSize.y, 0, 0), 1);

	auto renderPass = device->CreateResource<RenderPass>();
	renderPass->AddAttachment(ContentTreatment::Clear, ContentTreatment::Keep, vec4(0, 0, 1, 1));
	renderPass->AddAttachment(ContentTreatment::Clear, ContentTreatment::Keep, vec4(1, 0, 0, 0));
	renderPass->SetAttachmentImage(1, depthBuffer);

	renderPass->AddCommand(renderTriangle);
	renderPass->AddCommand(fontDraw);

	auto presentPass = device->CreateResource<PresentPass>();
	presentPass->Init(surface);

	{
		auto start = chrono::system_clock::now();
		uint32_t frameNumber = 0;

		while (!SDL_QuitRequested()) {
			SDL_Event event;

			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_WINDOWEVENT: {
						if (event.window.windowID == sdlWindowId) {
							switch (event.window.event) {
								case SDL_WINDOWEVENT_SIZE_CHANGED: {
									LOG("Window rect changed (", std::to_string(event.window.data1), ", ", std::to_string(event.window.data2), ")");
									auto size = surface->GetSize();
									if (size.x > 0 && size.y > 0) {
										surface->Update(size.x, size.y);
										proj = glm::perspectiveLH<float>(glm::pi<float>() / 3, static_cast<float>(size.x) / size.y, 0.1f, 100.0f);
										depthBuffer = device->CreateResource<Image>();
										depthBuffer->Init(Image::Usage::DepthBuffer, ColorFormat::D24S8, uvec4(size.x, size.y, 0, 0), 1);
										renderPass->SetAttachmentImage(1, depthBuffer);
										UpdateFontTransform(fontDraw, size, glm::vec4(1));
									}
									break;
								}
								case SDL_WINDOWEVENT_CLOSE: {
									event.type = SDL_QUIT;
									SDL_PushEvent(&event);
									break;
								}
							}
						}
						break;
					}
					case SDL_KEYDOWN: {
						if (event.key.keysym.sym == SDLK_RETURN && (event.key.keysym.mod & (KMOD_LALT | KMOD_RALT))) {
							if (SDL_GetWindowFlags(sdlWindow._data) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
								SDL_SetWindowBordered(sdlWindow._data, SDL_TRUE);
								SDL_SetWindowFullscreen(sdlWindow._data, 0);
							} else {
								SDL_SetWindowFullscreen(sdlWindow._data, SDL_WINDOW_FULLSCREEN_DESKTOP);
								SDL_SetWindowBordered(sdlWindow._data, SDL_FALSE);
							}
						}
						break;
					}
				}
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

			++frameNumber;
		}

		float seconds = util::ToSeconds(chrono::system_clock::now() - start);
		cout << "Quitting after " << seconds << " seconds and " << frameNumber << " frames for " << frameNumber / seconds << " FPS" << endl;
	}

	device->WaitIdle();

	return 0;
}