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
        }
        else
        {
            m_Timer.Tick();
            CalculateFramesPerSecond();

            MoveDirectionalLight();

            m_DxRenderer->SetRasterBackCullShadow();

            // 
            // Render to shadow map
            //

            // Set render target to the shadow map
            m_DxRenderer->SetRenderTargetShadowMap();
            SetOrthoCameraBuffer();

            // Bind the shader to the pipeline
            m_DxShader->Use();

            // Render the model
            m_DxShader->UpdateWorldBuffer(m_DxModel->World);
            m_DxModel->Render();

            // Render the floor
            m_DxShader->UpdateWorldBuffer(m_DxFloor->World);
            m_DxFloor->Render();


            //
            // Render to back buffer
            //
            m_DxRenderer->SetRasterBackCull();
            // Set render target to the back buffer
            m_DxRenderer->SetRenderTargetBackBuffer();
            m_DxRenderer->SetViewport(window_width, window_height);
            SetCameraBuffer();

            // Bind the shader to the pipeline
            m_DxShader->Use();

            // Apply shadow map texture
            auto context = m_DxRenderer->GetDeviceContext();
            context->PSSetShaderResources(0, 1, m_DxRenderer->GetShadowMapTexture());

            // Render the model
            m_DxShader->UpdateWorldBuffer(m_DxModel->World);
            m_DxModel->Render();

            // Render the floor
            m_DxShader->UpdateWorldBuffer(m_DxFloor->World);
            m_DxFloor->Render();

            // Render the light as a model for visualisation
            m_DxShader->UpdateWorldBuffer(m_DxDirectionalLight->World);
            m_DxDirectionalLight->Render();

            // Display the rendered scene
            m_DxRenderer->Present();
        }
    }

    return 0;
}

void Applicataion::MoveDirectionalLight()
{
    auto inputs = SDL_GetKeyboardState(nullptr);
    float delta_time = static_cast<float>(m_Timer.DeltaTime());

    // Move forward/backward along Z-axis
    if (inputs[SDL_SCANCODE_W])
    {
        m_DxDirectionalLight->World *= DirectX::XMMatrixTranslation(0.0f, 0.0f, 1.0f * delta_time);
    }
    else if (inputs[SDL_SCANCODE_S])
    {
        m_DxDirectionalLight->World *= DirectX::XMMatrixTranslation(0.0f, 0.0f, -1.0f * delta_time);
    }

    // Move left/right along X-axis
    if (inputs[SDL_SCANCODE_A])
    {
        m_DxDirectionalLight->World *= DirectX::XMMatrixTranslation(-1.0f * delta_time, 0.0f, 0.0f);
    }
    else if (inputs[SDL_SCANCODE_D])
    {
        m_DxDirectionalLight->World *= DirectX::XMMatrixTranslation(1.0f * delta_time, 0.0f, 0.0f);
    }

    // Move up/down along Y-axis v
    if (inputs[SDL_SCANCODE_E])
    {
        m_DxDirectionalLight->World *= DirectX::XMMatrixTranslation(0.0f, 1.0f * delta_time, 0.0f);
    }
    else if (inputs[SDL_SCANCODE_Q])
    {
        m_DxDirectionalLight->World *= DirectX::XMMatrixTranslation(0.0f, -1.0f * delta_time, 0.0f);
    }

    // Decompose matrix for position
    DirectX::XMVECTOR scale;
    DirectX::XMVECTOR rotation;
    DirectX::XMVECTOR position;
    DirectX::XMMatrixDecompose(&scale, &rotation, &position, m_DxDirectionalLight->World);

    // Calculate direction (direction looking at the center of the scene)
    DirectX::XMVECTOR center = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    auto direction = DirectX::XMVectorSubtract(center, position);
    direction = DirectX::XMVector4Normalize(direction);

    // Update buffer
    DX::DirectionalLightBuffer buffer = {};
    buffer.view = DirectX::XMMatrixTranspose(m_ShadowCameraView);
    buffer.projection = DirectX::XMMatrixTranspose(m_ShadowCameraProjection);
    DirectX::XMStoreFloat4(&buffer.direction, direction);

    m_DxShader->UpdateDirectionalLightBuffer(buffer);
}

void Applicataion::SetCameraBuffer()
{
    DX::CameraBuffer buffer = {};
    buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
    buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());
    buffer.cameraPosition = m_DxCamera->GetPosition();

    m_DxShader->UpdateCameraBuffer(buffer);
}

void Applicataion::SetOrthoCameraBuffer()
{
    // Decompose matrix for position
    DirectX::XMVECTOR scale;
    DirectX::XMVECTOR rotation;
    DirectX::XMVECTOR position;
    DirectX::XMMatrixDecompose(&scale, &rotation, &position, m_DxDirectionalLight->World);

    // Calculate view
    auto eye = position;
    auto at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    auto up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_ShadowCameraView = DirectX::XMMatrixLookAtLH(eye, at, up);

    // Calculate projection
    /*float fieldOfView = DirectX::XMConvertToRadians(m_FOV);
    float screenAspect = (float)m_WindowWidth / (float)m_WindowHeight;

    float depht_s = std::tan(m_FOV / 2.0 * DirectX::XM_PI / 180.0) * 2.0;

    float z = std::abs(position.z);
    float size_y = depht_s * z;
    float size_x = depht_s * z * screenAspect;*/

    m_ShadowCameraProjection = DirectX::XMMatrixOrthographicLH(40.0f, 40.0f, 1.0f, 100.0f);

    // Set buffer
    DX::CameraBuffer buffer = {};
    buffer.view = DirectX::XMMatrixTranspose(m_ShadowCameraView);
    buffer.projection = DirectX::XMMatrixTranspose(m_ShadowCameraProjection);
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
    m_SdlWindow = SDL_CreateWindow("DirectX - Directional Shadow Mapping", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
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

        auto title = "DirectX - Directional Shadow Mapping - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
        SDL_SetWindowTitle(m_SdlWindow, title.c_str());
    }
}
