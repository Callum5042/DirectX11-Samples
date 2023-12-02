#include "Application.h"

#include <string>
#include <SDL.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <future>

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

	// Initialise and create the DirectX 11 model
	std::future result1 = std::async(std::launch::async, [&]
	{
		std::unique_ptr<DX::Model> model = std::make_unique<DX::Model>(m_DxRenderer.get());
		model->Create(-3.0f, 0.0f, 0.0f);
		m_DxModels.push_back(std::move(model));
	});

	std::future result2 = std::async(std::launch::async, [&]
	{
		std::unique_ptr<DX::Model> model = std::make_unique<DX::Model>(m_DxRenderer.get());
		model->Create(0.0f, 0.0f, 0.0f);
		m_DxModels.push_back(std::move(model));
	});

	std::future result3 = std::async(std::launch::async, [&]
	{
		std::unique_ptr<DX::Model> model = std::make_unique<DX::Model>(m_DxRenderer.get());
		model->Create(3.0f, 0.0f, 0.0f);
		m_DxModels.push_back(std::move(model));
	});

	result1.wait();
	result2.wait();
	result3.wait();



	// Initialise and create the DirectX 11 shader
	m_DxShader = std::make_unique<DX::Shader>(m_DxRenderer.get());
	m_DxShader->LoadVertexShader("Shaders/VertexShader.cso");
	m_DxShader->LoadPixelShader("Shaders/PixelShader.cso");

	// Initialise and setup the perspective camera
	auto window_width = 0;
	auto window_height = 0;
	SDL_GetWindowSize(m_SdlWindow, &window_width, &window_height);

	m_DxCamera = std::make_unique<DX::Camera>(window_width, window_height);

	// Starts the timer
	m_Timer.Start();

	// Mutex
	std::mutex mrlock;

	// Render thread
	bool running = true;
	std::thread render_thread([&]
	{
		while (running)
		{
			if (mrlock.try_lock())
			{
				m_Timer.Tick();
				CalculateFramesPerSecond();

				// Clear the buffers
				m_DxRenderer->Clear();

				// Bind the shader to the pipeline
				m_DxShader->Use();

				// Render the model
				for (auto& model : m_DxModels)
				{
					model->Update(m_Timer.DeltaTime());
					UpdateWorldBuffer(model.get());
					model->Render();
				}

				mrlock.unlock();

				// Display the rendered scene
				m_DxRenderer->Present();
			}
		}
	});

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
					std::lock_guard<std::mutex> lock(mrlock);
					m_DxRenderer->Resize(e.window.data1, e.window.data2);
					m_DxCamera->UpdateAspectRatio(e.window.data1, e.window.data2);
				}
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				auto direction = static_cast<float>(e.wheel.y);
				m_DxCamera->UpdateFov(-direction);
			}
		}
	}

	running = false;
	if (render_thread.joinable())
	{
		render_thread.join();
	}

	return 0;
}

void Application::UpdateWorldBuffer(DX::Model* model)
{
	DX::WorldBuffer world_buffer = {};
	world_buffer.world = DirectX::XMMatrixTranspose(model->World);
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
	m_SdlWindow = SDL_CreateWindow("DirectX - Multithreading", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
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

		auto title = "DirectX - Multithreading - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
		SDL_SetWindowTitle(m_SdlWindow, title.c_str());
	}
}
