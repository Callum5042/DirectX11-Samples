#include "Application.h"

#include <string>
#include <SDL.h>
#include <iostream>
using namespace DirectX;

Application::~Application()
{
	SDLCleanup();
}

int Application::Execute()
{
	// Set size
	m_ShadowCameraViews.resize(m_CascadeLevels.size());
	m_ShadowCameraProjections.resize(m_CascadeLevels.size());

	// Initialise SDL subsystems and creates the window
	if (!SDLInit())
		return -1;

	// Initialise and create the DirectX 11 renderer
	m_DxRenderer = std::make_unique<DX::Renderer>(m_SdlWindow);
	m_DxRenderer->Create();

	// Initialise and create the DirectX 11 model
	for (int i = 0; i < 100; ++i)
	{
		auto model = std::make_unique<DX::Model>(m_DxRenderer.get());
		model->Create();
		model->Position = DirectX::XMFLOAT3(0, 1.0f, i * 5.0f);

		model->World = DirectX::XMMatrixTranslation(model->Position.x, model->Position.y, model->Position.z);

		m_DxModels.push_back(std::move(model));
	}

	m_DxFloor = std::make_unique<DX::Floor>(m_DxRenderer.get());
	m_DxFloor->Create();

	// Overlay shadow map visualise
	m_DxOverlay = std::make_unique<DX::Overlay>(m_DxRenderer.get());
	m_DxOverlay->Create();

	m_DxOverlayShader = std::make_unique<DX::OverlayShader>(m_DxRenderer.get());
	m_DxOverlayShader->LoadVertexShader("Shaders/OverlayVertexShader.cso");
	m_DxOverlayShader->LoadPixelShader("Shaders/OverlayPixelShader.cso");

	// Direct Light visualise
	m_DxDirectionalLight = std::make_unique<DX::DirectionalLight>(m_DxRenderer.get());
	m_DxDirectionalLight->Create();

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
			else if (e.type == SDL_MOUSEWHEEL)
			{
				auto direction = static_cast<float>(e.wheel.y);
				m_DxCamera->UpdateFov(-direction);
			}
			else if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.scancode == SDL_SCANCODE_1)
				{
					m_CameraIndex = 0;
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_2)
				{
					m_CameraIndex = 1;
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_3)
				{
					m_CameraIndex = 2;
				}
				else if (e.key.keysym.scancode == SDL_SCANCODE_4)
				{
					m_CameraIndex = 3;
				}
			}
		}
		else
		{
			m_Timer.Tick();
			CalculateFramesPerSecond();

			// Move first-person camera
			m_DxCamera->Update(static_cast<float>(m_Timer.DeltaTime()));

			// Move light with arrow keys and PageUp/PageDown
			m_DxDirectionalLight->Update(static_cast<float>(m_Timer.DeltaTime()));
			UpdateDirectionalLightBuffer();

			// Render to shadow map
			SetRenderToShadowMap();
			RenderScene();

			// Render to back buffer
			SetRenderToBackBuffer();
			RenderScene();

			// Render the light as a model for visualisation
			m_DxShader->UpdateWorldBuffer(m_DxDirectionalLight->World);
			m_DxDirectionalLight->Render();

			// Render overlay to visualize shadow map texture
			m_DxOverlayShader->Use();
			m_DxOverlay->Render();

			// Display the rendered scene
			m_DxRenderer->Present();
		}
	}

	return 0;
}

void Application::RenderScene()
{
	// Render the model
	for (auto& model : m_DxModels)
	{
		m_DxShader->UpdateWorldBuffer(model->World);
		model->Render();
	}

	// Render the floor
	m_DxShader->UpdateWorldBuffer(m_DxFloor->World);
	m_DxFloor->Render();
}

void Application::SetRenderToBackBuffer()
{
	m_DxRenderer->SetRasterBackCull();

	// Set render target to the back buffer
	m_DxRenderer->SetRenderTargetBackBuffer();

	// Reset the viewport back to ours
	auto window_width = 0;
	auto window_height = 0;
	SDL_GetWindowSize(m_SdlWindow, &window_width, &window_height);

	m_DxRenderer->SetViewport(window_width, window_height);

	// Update camera buffer to use main camera
	SetCameraBuffer();

	// Bind the shader to the pipeline
	m_DxShader->Use();

	// Apply shadow map texture
	auto context = m_DxRenderer->GetDeviceContext();
	context->PSSetShaderResources(0, 1, m_DxRenderer->GetShadowMapTexture());
}

void Application::SetRenderToShadowMap()
{
	// Set rasterizer for shadows
	m_DxRenderer->SetRasterBackCullShadow();

	// Set render target to the shadow map
	m_DxRenderer->SetRenderTargetShadowMap();

	// Set shadow camera from the light point of view
	SetShadowCameraBuffer();

	// Bind the shader to the pipeline
	m_DxShader->Use();
}

void Application::UpdateDirectionalLightBuffer()
{
	auto direction = DirectX::XMVectorNegate(m_DxDirectionalLight->GetDirection());

	// Update buffer
	DX::DirectionalLightBuffer buffer = {};
	buffer.view = DirectX::XMMatrixTranspose(m_ShadowCameraViews[0]);
	buffer.projection = DirectX::XMMatrixTranspose(m_ShadowCameraProjections[0]);
	DirectX::XMStoreFloat4(&buffer.direction, direction);

	m_DxShader->UpdateDirectionalLightBuffer(buffer);
}

void Application::SetCameraBuffer()
{
	DX::CameraBuffer buffer = {};
	buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
	buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());
	DirectX::XMStoreFloat3(&buffer.cameraPosition, m_DxCamera->GetPosition());

	// Shadow camera
	if (m_CameraIndex != 0)
	{
		buffer.view = DirectX::XMMatrixTranspose(m_ShadowCameraViews[m_CameraIndex - 1]);
		buffer.projection = DirectX::XMMatrixTranspose(m_ShadowCameraProjections[m_CameraIndex - 1]);
	}

	m_DxShader->UpdateCameraBuffer(buffer);
}

void Application::SetShadowCameraBuffer()
{
	for (int cascade_level = 0; cascade_level < m_CascadeLevels.size(); ++cascade_level)
	{
		// Calculate bounding box of view frustum
		float nearZ = m_CascadeLevels[cascade_level].first;
		float farZ = m_CascadeLevels[cascade_level].second;

		XMMATRIX projection = XMMatrixPerspectiveFovLH(m_DxCamera->GetFieldOfViewRadians(), m_DxCamera->GetAspectRatio(), nearZ, farZ);
		XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, (m_DxCamera->GetView() * projection));

		// Get the 8 points of the view frustum in world space
		XMFLOAT3 frustumCornersWS[8] =
		{
			XMFLOAT3(-1.0f,  1.0f, 0.0f),
			XMFLOAT3(1.0f,  1.0f, 0.0f),
			XMFLOAT3(1.0f, -1.0f, 0.0f),
			XMFLOAT3(-1.0f, -1.0f, 0.0f),
			XMFLOAT3(-1.0f,  1.0f, 1.0f),
			XMFLOAT3(1.0f,  1.0f, 1.0f),
			XMFLOAT3(1.0f, -1.0f, 1.0f),
			XMFLOAT3(-1.0f, -1.0f, 1.0f),
		};

		for (int i = 0; i < 8; ++i)
		{
			DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&frustumCornersWS[i]);
			DirectX::XMStoreFloat3(&frustumCornersWS[i], DirectX::XMVector3TransformCoord(v, invViewProj));
		}

		// Calculate the centroid of the view frustum slice
		XMVECTOR frustumCenter = DirectX::XMVectorZero();
		for (int i = 0; i < 8; ++i)
		{
			frustumCenter = frustumCenter + DirectX::XMLoadFloat3(&frustumCornersWS[i]);
		}

		frustumCenter /= 8.0f;

		// Distance
		DirectX::XMVECTOR difference = DirectX::XMVectorSubtract(frustumCenter, m_DxCamera->GetPosition());
		float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(difference));

		// std::cout << "distance: " << distance << '\n';

		// Calculate view
		auto eye = frustumCenter + DirectX::XMVectorScale(m_DxDirectionalLight->GetDirection(), distance);
		auto at = frustumCenter;
		auto up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		m_ShadowCameraViews[cascade_level] = DirectX::XMMatrixLookAtLH(eye, at, up);

		// Calculate projection
		m_ShadowCameraProjections[cascade_level] = DirectX::XMMatrixOrthographicLH(distance * 2, distance * 2, 1.0f, distance * 2);
	}

	// Set buffer
	DX::CameraBuffer buffer = {};
	buffer.view = DirectX::XMMatrixTranspose(m_ShadowCameraViews[0]);
	buffer.projection = DirectX::XMMatrixTranspose(m_ShadowCameraProjections[0]);
	DirectX::XMStoreFloat3(&buffer.cameraPosition, m_DxCamera->GetPosition());

	m_DxShader->UpdateCameraBuffer(buffer);
}

//void Application::SetShadowCameraBuffer()
//{
//	// Calculate view
//	auto eye = DirectX::XMVectorScale(m_DxDirectionalLight->GetDirection(), 20.0f);
//	auto at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
//	auto up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//	m_ShadowCameraView = DirectX::XMMatrixLookAtLH(eye, at, up);
//
//	// Calculate projection
//	m_ShadowCameraProjection = DirectX::XMMatrixOrthographicLH(30.0f, 40.0f, 1.0f, 50.0f);
//
//	// Set buffer
//	DX::CameraBuffer buffer = {};
//	buffer.view = DirectX::XMMatrixTranspose(m_ShadowCameraView);
//	buffer.projection = DirectX::XMMatrixTranspose(m_ShadowCameraProjection);
//	DirectX::XMStoreFloat3(&buffer.cameraPosition, m_DxCamera->GetPosition());
//
//	m_DxShader->UpdateCameraBuffer(buffer);
//}

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
	m_SdlWindow = SDL_CreateWindow("DirectX - Casaded Shadow Maps", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
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

		auto title = "DirectX - Casaded Shadow Maps - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
		SDL_SetWindowTitle(m_SdlWindow, title.c_str());
	}
}
