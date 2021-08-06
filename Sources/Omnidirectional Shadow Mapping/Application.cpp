#include "Application.h"

#include <string>
#include <SDL.h>
#include <iostream>

Applicataion::~Applicataion()
{
	SDLCleanup();
}

int Applicataion::Execute()
{
	// Initialise SDL subsystems and creates the window
	if (!SDLInit())
		return -1;

	// Initialise and create the DirectX 11 renderer
	m_DxRenderer = std::make_unique<DX::Renderer>(m_SdlWindow);
	m_DxRenderer->Create();

	// Initialise and create the DirectX 11 model
	m_DxModel = std::make_unique<DX::Model>(m_DxRenderer.get());
	m_DxModel->Create();

	m_DxFloor = std::make_unique<DX::Floor>(m_DxRenderer.get());
	m_DxFloor->Create();

	m_DxPointLight = std::make_unique<DX::PointLight>(m_DxRenderer.get());
	m_DxPointLight->Create();

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

					// Update world constant buffer with new camera view and perspective
					//SetCameraBuffer();
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

					// Update world constant buffer with new camera view and perspective
					//SetCameraBuffer();
				}
			}
			else if (e.type == SDL_MOUSEWHEEL)
			{
				auto direction = static_cast<float>(e.wheel.y);
				m_DxCamera->UpdateFov(-direction);

				// Update world constant buffer with new camera view and perspective
				//SetCameraBuffer();
			}
		}
		else
		{
			m_Timer.Tick();
			CalculateFramesPerSecond();

			// Update light source
			MovePointLight();

			// Update light source
			DX::PointLightBuffer light_buffer = {};

			DirectX::XMVECTOR scale_vector;
			DirectX::XMVECTOR rotation_vector;
			DirectX::XMVECTOR position_vector;
			DirectX::XMMatrixDecompose(&scale_vector, &rotation_vector, &position_vector, m_DxPointLight->World);

			DirectX::XMStoreFloat3(&light_buffer.position, position_vector);
			m_DxShader->UpdatePointLightBuffer(light_buffer);

			// Clear the buffers
			//m_DxRenderer->Clear();


			//
			// Render to depth buffer for shadows
			// 

			m_DxShader->Use();

			auto projection = DirectX::XMMatrixPerspectiveFovLH(0.5f * DirectX::XM_PI, static_cast<float>(window_width) / window_height, 1.0f, 100.0f);

			DirectX::XMFLOAT3 center = light_buffer.position;
			DirectX::XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

			DirectX::XMFLOAT3 targets[6] =
			{
				DirectX::XMFLOAT3(center.x + 1.0f, center.y, center.z), // +X
				DirectX::XMFLOAT3(center.x - 1.0f, center.y, center.z), // -X
				DirectX::XMFLOAT3(center.x, center.y + 1.0f, center.z), // +Y
				DirectX::XMFLOAT3(center.x, center.y - 1.0f, center.z), // -Y
				DirectX::XMFLOAT3(center.x, center.y, center.z + 1.0f), // +Z
				DirectX::XMFLOAT3(center.x, center.y, center.z - 1.0f)  // -Z
			};

			// Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
			// are looking down +Y or -Y, so we need a different "up" vector.
			DirectX::XMFLOAT3 ups[6] =
			{
				DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),  // +X
				DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),  // -X
				DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f), // +Y
				DirectX::XMFLOAT3(0.0f, 0.0f, +1.0f), // -Y
				DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),  // +Z
				DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)	  // -Z
			};

			for (int i = 0; i < 6; i++)
			{
				m_DxRenderer->SetRenderTargetTexture(i);

				// Configure camera
				
				DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&center);
				DirectX::XMVECTOR at = DirectX::XMLoadFloat3(&targets[i]);
				DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&ups[i]);
				auto view = DirectX::XMMatrixLookAtLH(eye, at, up);

				DX::CameraBuffer buffer = {};
				buffer.view = DirectX::XMMatrixTranspose(view);
				buffer.projection = DirectX::XMMatrixTranspose(projection);
				DirectX::XMStoreFloat3(&buffer.cameraPosition, position_vector);

				m_DxShader->UpdateCameraBuffer(buffer);

				// Render scene
				m_DxShader->UpdateWorldBuffer(m_DxModel->World);
				m_DxModel->Render();

				m_DxShader->UpdateWorldBuffer(m_DxFloor->World);
				m_DxFloor->Render();
			}

			//
			// Render to back buffer
			//


			// Bind the shader to the pipeline
			m_DxRenderer->SetViewport(window_width, window_height);
			m_DxRenderer->SetRenderTargetBackBuffer();
			SetCameraBuffer();

			auto shadowmap = m_DxRenderer->GetShadowCubeMap();
			auto deviceContext = m_DxRenderer->GetDeviceContext();
			deviceContext->PSSetShaderResources(0, 1, shadowmap);

			// Render the model
			m_DxShader->UpdateWorldBuffer(m_DxModel->World);
			m_DxModel->Render();

			// Render the floor
			m_DxShader->UpdateWorldBuffer(m_DxFloor->World);
			m_DxFloor->Render();

			// Render the light as a model for visualisation
			m_DxShader->UpdateWorldBuffer(m_DxPointLight->World);
			m_DxPointLight->Render();

			// Display the rendered scene
			m_DxRenderer->Present();
		}
	}

	return 0;
}

void Applicataion::MovePointLight()
{
	auto inputs = SDL_GetKeyboardState(nullptr);
	float delta_time = static_cast<float>(m_Timer.DeltaTime());

	// Move forward/backward along Z-axis
	if (inputs[SDL_SCANCODE_W])
	{
		m_DxPointLight->World *= DirectX::XMMatrixTranslation(0.0f, 0.0f, 1.0f * delta_time);
	}
	else if (inputs[SDL_SCANCODE_S])
	{
		m_DxPointLight->World *= DirectX::XMMatrixTranslation(0.0f, 0.0f, -1.0f * delta_time);
	}

	// Move left/right along X-axis
	if (inputs[SDL_SCANCODE_A])
	{
		m_DxPointLight->World *= DirectX::XMMatrixTranslation(-1.0f * delta_time, 0.0f, 0.0f);
	}
	else if (inputs[SDL_SCANCODE_D])
	{
		m_DxPointLight->World *= DirectX::XMMatrixTranslation(1.0f * delta_time, 0.0f, 0.0f);
	}

	// Move up/down along Y-axis
	if (inputs[SDL_SCANCODE_E])
	{
		m_DxPointLight->World *= DirectX::XMMatrixTranslation(0.0f, 1.0f * delta_time, 0.0f);
	}
	else if (inputs[SDL_SCANCODE_Q])
	{
		m_DxPointLight->World *= DirectX::XMMatrixTranslation(0.0f, -1.0f * delta_time, 0.0f);
	}
}

void Applicataion::SetCameraBuffer()
{
	DX::CameraBuffer buffer = {};
	buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
	buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());
	buffer.cameraPosition = m_DxCamera->GetPosition();

	m_DxShader->UpdateCameraBuffer(buffer);
}

bool Applicataion::SDLInit()
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
	m_SdlWindow = SDL_CreateWindow("DirectX - Omnidirectional Shadow Mapping", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
	if (m_SdlWindow == nullptr)
	{
		std::string error = "SDL_CreateWindow failed: ";
		error += SDL_GetError();
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error.c_str(), nullptr);
		return false;
	}

	return true;
}

void Applicataion::SDLCleanup()
{
	SDL_DestroyWindow(m_SdlWindow);
	SDL_Quit();
}

void Applicataion::CalculateFramesPerSecond()
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

		auto title = "DirectX - Omnidirectional Shadow Mapping - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
		SDL_SetWindowTitle(m_SdlWindow, title.c_str());
	}
}