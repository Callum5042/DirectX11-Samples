#include "Application.h"

#include <string>
#include <SDL.h>
#include <iostream>
#include <algorithm>

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
    m_DxModel = std::make_unique<DX::Model>(m_DxRenderer.get());
    m_DxModel->Create();
    m_DxRenderer->ToggleWireframe(true);

    // Initialise and create the DirectX 11 shader
    m_DxShader = std::make_unique<DX::Shader>(m_DxRenderer.get());
    m_DxShader->LoadVertexShader("Shaders/VertexShader.cso");
    m_DxShader->LoadPixelShader("Shaders/PixelShader.cso");
    m_DxShader->LoadHullShader("Shaders/HullShader.cso");
    m_DxShader->LoadDomainShader("Shaders/DomainShader.cso");

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
                    SetWorldBuffer();
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
                    SetWorldBuffer();
                }
            }
            else if (e.type == SDL_MOUSEWHEEL)
            {
                auto direction = static_cast<float>(e.wheel.y);
                m_DxCamera->UpdateRadius(direction);

                // Update world constant buffer with new camera view and perspective
                SetWorldBuffer();
            }
        }
        else
        {
            m_Timer.Tick();
            CalculateFramesPerSecond();

            // Clear the buffers
            m_DxRenderer->Clear();

            // Bind the shader to the pipeline
            m_DxShader->Use();

            // Render the model
            m_DxModel->Render();

            // Display the rendered scene
            m_DxRenderer->Present();
        }
    }

    return 0;
}

void Application::SetWorldBuffer()
{
    DX::WorldBuffer world_buffer = {};
    world_buffer.world = DirectX::XMMatrixTranspose(m_DxModel->World);
    world_buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
    world_buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());

    auto position = m_DxCamera->GetPosition();
    world_buffer.cameraPosition = DirectX::XMFLOAT4(position.x, position.y, position.z, 1.0f);

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
    m_SdlWindow = SDL_CreateWindow("DirectX - Heightmap", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
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

        auto title = "DirectX - Heightmap - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
        SDL_SetWindowTitle(m_SdlWindow, title.c_str());
    }
}
