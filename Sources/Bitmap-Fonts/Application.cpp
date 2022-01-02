#include "Application.h"

#include <string>
#include <SDL.h>
#include <SpriteFont.h>
#include <SimpleMath.h>

// Tutorial
// https://github.com/microsoft/DirectXTK/wiki/Drawing-text

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

    // Sprite font
    std::unique_ptr<DirectX::SpriteFont> spriteFont = std::make_unique<DirectX::SpriteFont>(m_DxRenderer->GetDevice(), L"myfile.spritefont");

    auto context = m_DxRenderer->GetDeviceContext();
    DirectX::SimpleMath::Vector2 fontPos;
    std::unique_ptr<DirectX::SpriteBatch> spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

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

                    // Resize text position to be centre of the window
                    int width, height;
                    SDL_GetWindowSize(m_SdlWindow, &width, &height);

                    fontPos.x = float(width) / 2.f;
                    fontPos.y = float(height) / 2.f;
                }
            }
        }
        else
        {
            m_Timer.Tick();
            CalculateFramesPerSecond();

            // Clear the buffers
            m_DxRenderer->Clear();

            // Render the model
            spriteBatch->Begin();

            std::wstring output = L"Hello, DirectX11!";
            DirectX::SimpleMath::Vector2 origin = spriteFont->MeasureString(output.c_str());
            origin /= 2.0f;

            spriteFont->DrawString(spriteBatch.get(), output.c_str(), fontPos, DirectX::Colors::White, 0.f, origin);

            spriteBatch->End();

            // Display the rendered scene
            m_DxRenderer->Present();
        }
    }

    return 0;
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
    m_SdlWindow = SDL_CreateWindow("DirectX - Bitmap-Fonts", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
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

        auto title = "DirectX - Bitmap-Fonts - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
        SDL_SetWindowTitle(m_SdlWindow, title.c_str());
    }
}
