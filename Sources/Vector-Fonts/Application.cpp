#include "Application.h"

#include <string>
#include <SDL.h>
#include <iostream>

#include <dwrite.h>
#include <d2d1_1.h>

Application::~Application()
{
	SDLCleanup();
}

int Application::Execute()
{
	// Initialise SDL subsystems and creates the window
	if (!SDLInit())
		return -1;

	// Initialise and create the DirectX 11 renderer
	m_DxRenderer = std::make_unique<DX::Renderer>(m_SdlWindow);
	m_DxRenderer->Create();

	m_DxPlane = std::make_unique<DX::Plane>(m_DxRenderer.get());
	m_DxPlane->Create();

	// Initialise and create the DirectX 11 shader
	m_DxShader = std::make_unique<DX::Shader>(m_DxRenderer.get());
	m_DxShader->LoadVertexShader("Shaders/VertexShader.cso");
	m_DxShader->LoadPixelShader("Shaders/PixelShader.cso");

	// Initialise and setup the perspective camera
	auto window_width = 0;
	auto window_height = 0;
	SDL_GetWindowSize(m_SdlWindow, &window_width, &window_height);

	m_DxCamera = std::make_unique<DX::Camera>(window_width, window_height);

	// Something DirectWrite
	ComPtr<ID3D11Device> device = m_DxRenderer->GetDevice();

	ComPtr<IDXGIDevice> dxgiDevice = nullptr;
	DX::Check(device.As(&dxgiDevice));

	// Create a Direct2D factory.
	ComPtr<ID2D1Factory1> d2dFactory = nullptr;
	DX::Check(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.GetAddressOf()));

	ComPtr<ID2D1Device> d2dDevice = nullptr;
	DX::Check(d2dFactory->CreateDevice(dxgiDevice.Get(), d2dDevice.GetAddressOf()));

	ComPtr<ID2D1DeviceContext> d2dDeviceContext = nullptr;
	DX::Check(d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2dDeviceContext.GetAddressOf()));

	ComPtr<ID2D1SolidColorBrush> pBlackBrush;
	DX::Check(d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black),&pBlackBrush));



	// Starts the timer
	m_Timer.Start();

	// Main application event loop
	SDL_Event e = {};
	while (e.type != SDL_QUIT)
	{
		if (SDL_PollEvent(&e))
		{
			if (e.type == SDL_WINDOWEVENT)
			{
				// On resize event, resize the DxRender device
				if (e.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					m_DxRenderer->Resize(e.window.data1, e.window.data2);
					m_DxCamera->UpdateAspectRatio(e.window.data1, e.window.data2);
				}
			}
			else if (e.type == SDL_MOUSEMOTION)
			{
				if (e.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))
				{
					// Rotate the world 
					auto pitch = e.motion.yrel * 0.01f;
					auto yaw = e.motion.xrel * 0.01f;
					m_DxCamera->Rotate(pitch, yaw);
				}
			}
		}
		else
		{
			m_Timer.Tick();
			CalculateFramesPerSecond();

			// Clear the buffers
			m_DxRenderer->SetRenderTargetBackBuffer();

			// Bind the shader to the pipeline
			m_DxShader->Use();

			// Update camera
			UpdateWorldBufferCamera();

			// Render the plane
			auto rendered_texture = m_DxRenderer->GetRenderedTexture();
			m_DxPlane->SetTexture(rendered_texture);
			m_DxPlane->Render();

			// Display the rendered scene
			m_DxRenderer->Present();
		}
	}

	return 0;
}

void Application::UpdateWorldBufferCamera()
{
	DX::WorldBuffer world_buffer = {};
	world_buffer.world = DirectX::XMMatrixTranspose(m_DxPlane->World);
	world_buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
	world_buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());
	m_DxShader->UpdateWorldConstantBuffer(world_buffer);
}

bool Application::SDLInit()
{
	// Initialise SDL subsystems
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		auto error = SDL_GetError();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error, nullptr);
		return false;
	}

	// Create SDL Window
	auto window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED;
	m_SdlWindow = SDL_CreateWindow("DirectX - Vector-Fonts", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
	if (m_SdlWindow == nullptr)
	{
		std::string error = "SDL_CreateWindow failed: ";
		error += SDL_GetError();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error.c_str(), nullptr);
		return false;
	}

	return true;
}

void Application::SDLCleanup()
{
	SDL_DestroyWindow(m_SdlWindow);
	SDL_Quit();
}

void Application::CalculateFramesPerSecond()
{
	// Changes the window title to show the frames per second and average frame time every second

	static double time = 0;
	static int frameCount = 0;

	frameCount++;
	time += m_Timer.DeltaTime();
	if (time > 1.0f)
	{
		auto fps = frameCount;
		time = 0.0f;
		frameCount = 0;

		auto title = "DirectX - Vector-Fonts - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
		SDL_SetWindowTitle(m_SdlWindow, title.c_str());
	}
}
