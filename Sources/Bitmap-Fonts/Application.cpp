#include "Application.h"

#include <string>
#include <SDL.h>
#include <SpriteFont.h>
#include <SimpleMath.h>

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
    //m_DxModel = std::make_unique<DX::Model>(m_DxRenderer.get());
    //m_DxModel->Create();

    // Sprite font
    std::unique_ptr<DirectX::SpriteFont> m_font;
    m_font = std::make_unique<DirectX::SpriteFont>(m_DxRenderer->GetDevice(), L"myfile.spritefont");
    //m_font.reset();

    DirectX::SimpleMath::Vector2 m_fontPos;
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

    auto context = m_DxRenderer->GetDeviceContext();
    m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);

    int width, height;
    SDL_GetWindowSize(m_SdlWindow, &width, &height);

    m_fontPos.x = float(width) / 2.f;
    m_fontPos.y = float(height) / 2.f;
    //m_spriteBatch.reset();

    // Initialise and create the DirectX 11 shader
    m_DxShader = std::make_unique<DX::Shader>(m_DxRenderer.get());
    m_DxShader->LoadVertexShader("Shaders/VertexShader.cso");
    m_DxShader->LoadPixelShader("Shaders/PixelShader.cso");

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
                }
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
            //m_DxModel->Render();

            m_spriteBatch->Begin();

            const wchar_t* output = L"Hello World";

            DirectX::SimpleMath::Vector2 origin = m_font->MeasureString(output);
            origin /= 2.0f;

            m_font->DrawString(m_spriteBatch.get(), output, m_fontPos, DirectX::Colors::White, 0.f, origin);

            m_spriteBatch->End();

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
