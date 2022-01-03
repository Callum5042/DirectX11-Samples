#include "Application.h"

#include <string>
#include <SDL.h>
#include <iostream>

#include <dwrite.h>
#include <d2d1_1.h>
#pragma comment(lib, "dwrite")

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

	// Make Direct2D device from Direct3D Texture
	auto texture = m_DxRenderer->GetTexture();

	ComPtr<IDXGISurface> dxgiSurface = nullptr;
	DX::Check(texture->QueryInterface(dxgiSurface.GetAddressOf()));

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), 96, 96);

	ComPtr<ID2D1Factory> d2dFactory = nullptr;
	DX::Check(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.GetAddressOf()));

	ComPtr<ID2D1RenderTarget> d2dRenderTarget = nullptr;
	DX::Check(d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface.Get(), &props, d2dRenderTarget.GetAddressOf()));

	// Setup DirectWrite
	ComPtr<IDWriteFactory> directWriteFactory = nullptr;
	ComPtr<IDWriteTextFormat> directWriteTextFormat = nullptr;

	DX::Check(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(directWriteFactory), reinterpret_cast<IUnknown**>(directWriteFactory.GetAddressOf())));

	static const WCHAR msc_fontName[] = L"Verdana";
	static const FLOAT msc_fontSize = 100;

	DX::Check(directWriteFactory->CreateTextFormat(msc_fontName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, msc_fontSize, L"", directWriteTextFormat.GetAddressOf()));

	directWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	directWriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	ComPtr<ID2D1SolidColorBrush> textBrushColour = nullptr;
	const D2D1_COLOR_F color = D2D1::ColorF(0.0f, 0.0f, 0.0f);
	DX::Check(d2dRenderTarget->CreateSolidColorBrush(color, textBrushColour.GetAddressOf()));

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

			//
			// Direct2D
			//

			d2dRenderTarget->BeginDraw();
			d2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::YellowGreen));

			static const WCHAR sc_helloWorld[] = L"Hello, DirectWrite!";
			D2D1_SIZE_F renderTargetSize = d2dRenderTarget->GetSize();
			d2dRenderTarget->DrawText(sc_helloWorld, ARRAYSIZE(sc_helloWorld) - 1, directWriteTextFormat.Get(), D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height), textBrushColour.Get());

			d2dRenderTarget->EndDraw();


			//
			// Direct3D
			//

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
	m_SdlWindow = SDL_CreateWindow("DirectX - DirectWrite", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
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

		auto title = "DirectX - DirectWrite - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
		SDL_SetWindowTitle(m_SdlWindow, title.c_str());
	}
}
