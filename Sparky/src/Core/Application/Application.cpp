#include <Windows.h>

#include "Sparky.h"

Sparky::Application::Application()
{
	PlaySound(L"Assets/Audio/Music", SP_NULL, SND_ASYNC);

	WindowCreateInfo windowInfo{};
	windowInfo.pApplicationName = "Sparky Editor";
	windowInfo.glContextVersion = Version(4, 6, 0);
	windowInfo.fullscreen = SP_FALSE;
	windowInfo.coreProfile = SP_TRUE;
	windowInfo.listGPUExtensions = SP_FALSE;
	windowInfo.debugMode = SP_TRUE;
	windowInfo.VSYNC = SP_TRUE;
	windowInfo.windowSize = windowInfo.fullscreen ? Window::MAX_WINDOW_SIZE : vec2(1280, 720);

	m_Window = Window::CreateInstance(&windowInfo);

	if (!m_Window->Init())
	{
		SP_FATAL("Failed to Initialize Window!");
		throw SparkyException(__LINE__, __FILE__);
	}

	m_ActiveScene = new Scene();
}

Sparky::Application::~Application()
{
	if (m_Window != SP_NULL_HANDLE)
		delete m_Window;
	if (m_ActiveScene != SP_NULL_HANDLE)
		delete m_ActiveScene;
}

Sparky::Application* Sparky::Application::MakeInstance()
{
	static Application* app = new Application();

	if (app != SP_NULL_HANDLE)
		return app;
	else
	{
		SP_FATAL("Failed to allocate memory for new Application!");
		throw SparkyException(__LINE__, __FILE__);
	}
}

void Sparky::Application::Run() const noexcept
{
	stl::Array<Vertex, 4> vertices{};

	              //positions                 colors                  texture coords
	vertices[0] = { vec3(-0.5f,  0.5f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec2(0.0f, 1.0f) }; // top left
	vertices[1] = { vec3( 0.5f,  0.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec2(1.0f, 1.0f) }; // top right
	vertices[2] = { vec3(-0.5f, -0.5f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec2(0.0f, 0.0f) }; // bottom left
	vertices[3] = { vec3( 0.5f, -0.5f, 0.0f), vec3(1.0f, 1.0f, 0.0f), vec2(1.0f, 0.0f) }; // bottom right

	stl::Array<u8, 6> indices{};

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	indices[3] = 1;
	indices[4] = 2;
	indices[5] = 3;

	ShaderProgramCreateInfo shaderInfo{
		"Assets/Shaders/default.vert.glsl",
		"Assets/Shaders/default.frag.glsl"
	};

	Shader shader(&shaderInfo);

	shader.SetUniform("u_Proj", mat4::orthographic(-2.75f, 2.75f));
	shader.SetUniform("u_TexImage", 0);

	mat4 model = mat4::identity();

	TextureCreateInfo texInfo{};
	texInfo.pFilename = "Assets/Textures/DarkSky.jpg";
	texInfo.format = TextureFormatType::RGBA;
	texInfo.pixelType = TexturePixelType::Smooth;
	texInfo.flipY = SP_TRUE;

	Texture texture(&texInfo);

	FrameBufferCreateInfo fbInfo{};
	fbInfo.size = Window::MAX_WINDOW_SIZE;

	FrameBuffer framebuffer(&fbInfo);

	VertexBufferCreateInfo vbInfo{};
	vbInfo.dataType = VertexBufferDataType::Float;
	vbInfo.storageType = VertexBufferStorageType::Dynamic;
	vbInfo.stride = static_cast<u32>(sizeof(Vertex));
	vbInfo.offset = SP_NULL;
	vbInfo.vertexCount = 6;
	vbInfo.vertices = vertices;

	IndexBufferCreateInfo ibInfo{};
	ibInfo.dataType = IndexBufferDataType::UByte;
	ibInfo.storageType = IndexBufferStorageType::Dynamic;
	ibInfo.indexCount = 6;
	ibInfo.indices = indices;

	VertexArray va;
	va.LinkVBO(VertexBuffer::Create(&vbInfo));

	va.PushAttrib(0, 3, SP_FALSE, SP_NULL);
	va.PushAttrib(1, 3, SP_FALSE, sizeof(vec3));
	va.PushAttrib(2, 2, SP_FALSE, sizeof(vec3) * 2);

	va.LinkIBO(IndexBuffer::Create(&ibInfo));
	
	RendererStatistics stats{};
	stats.drawCalls = 1;
	stats.triangleCount = va.GetLinkedVBOs()[0]->GetVertexCount() / SP_VERTICES_PER_TRIANGLE;
	stats.vertices = va.GetLinkedVBOs()[0]->GetVertexCount();

	Renderer renderer;
	renderer.SetClearColor({ .1, .1, .11 });
	renderer.SubmitStats(stats);
	
	u32 frameCount{};

	while (!m_Window->Closed())
	{
		framebuffer.Bind();
		renderer.RenderClear();

		shader.SetUniform("u_Model", model);
		m_Window->ProcessInput(model, 0.05f, shader);

		texture.Bind(0);
		va.Bind();

		renderer.Render(PrimitiveType::Triangles, va.GetLinkedVBOs()[0]->GetVertexCount(), va.GetIBODataType());

		renderer.Update();
		framebuffer.Unbind();

		m_Window->CreateEditorGUIFrame(framebuffer, frameCount, renderer.GetStats());

		frameCount++;
	}

	va.Destroy();
	shader.Destroy();
}