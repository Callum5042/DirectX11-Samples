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
        m_Timer.Tick();
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
                    DX::WorldBuffer world_buffer = {};
                    world_buffer.world = DirectX::XMMatrixTranspose(m_DxModel->World);
                    world_buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
                    world_buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());
                    m_DxShader->UpdateWorldConstantBuffer(world_buffer);
                }
            }
            else if (e.type == SDL_MOUSEMOTION)
            {
                if (e.motion.state == SDL_BUTTON_LEFT)
                {
                    // Rotate the world 
                    auto pitch = e.motion.yrel * 0.01f;
                    auto yaw = e.motion.xrel * 0.01f;
                    m_DxCamera->Rotate(pitch, yaw);

                    // Update world constant buffer with new camera view and perspective
                    DX::WorldBuffer world_buffer = {};
                    world_buffer.world = DirectX::XMMatrixTranspose(m_DxModel->World);
                    world_buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
                    world_buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());
                    m_DxShader->UpdateWorldConstantBuffer(world_buffer);
                }
            }
            else if (e.type == SDL_MOUSEWHEEL)
            {
                auto direction = static_cast<float>(e.wheel.y);
                m_DxCamera->UpdateFov(-direction);

                // Update world constant buffer with new camera view and perspective
                DX::WorldBuffer world_buffer = {};
                world_buffer.world = DirectX::XMMatrixTranspose(m_DxModel->World);
                world_buffer.view = DirectX::XMMatrixTranspose(m_DxCamera->GetView());
                world_buffer.projection = DirectX::XMMatrixTranspose(m_DxCamera->GetProjection());
                m_DxShader->UpdateWorldConstantBuffer(world_buffer);
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.repeat == 0)
                {
                    static bool wireframe = false;
                    wireframe = !wireframe;
                    m_DxRenderer->ToggleWireframe(wireframe);
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                if (e.button.button == SDL_BUTTON_RIGHT)
                {
                    Pick(e.button.x, e.button.y);
                }
            }
        }
        else
        {
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
    m_SdlWindow = SDL_CreateWindow("DirectX - Drawing a Triangle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, window_flags);
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

        auto title = "DirectX - Drawing a Triangle - FPS: " + std::to_string(fps) + " (" + std::to_string(1000.0f / fps) + " ms)";
        SDL_SetWindowTitle(m_SdlWindow, title.c_str());
    }
}

void Applicataion::Pick(int sx, int sy)
{
    auto projection = m_DxCamera->GetProjection();

    // Get window size
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(m_SdlWindow, &width, &height);

    // Compute picking ray in view space.
    // define _XM_NO_INTRINSICS_
    float vx = (+2.0f * sx / width - 1.0f) / projection(0, 0);
    float vy = (-2.0f * sy / height + 1.0f) / projection(1, 1);

    // Ray definition in view space.
    auto rayOrigin = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    auto rayDir = DirectX::XMVectorSet(vx, vy, 1.0f, 0.0f);

    // Tranform ray to local space of Mesh.
    auto V = m_DxCamera->GetView();
    auto determinant_v = DirectX::XMMatrixDeterminant(V);
    auto invView = DirectX::XMMatrixInverse(&determinant_v, V);

    auto W = m_DxModel->World;
    auto determinant_w = DirectX::XMMatrixDeterminant(W);
    auto invWorld = DirectX::XMMatrixInverse(&determinant_w, W);

    auto toLocal = DirectX::XMMatrixMultiply(invView, invWorld);

    rayOrigin = DirectX::XMVector3TransformCoord(rayOrigin, toLocal);
    rayDir = DirectX::XMVector3TransformNormal(rayDir, toLocal);

    // Make the ray direction unit length for the intersection tests.
    rayDir = XMVector3Normalize(rayDir);

    // If we hit the bounding box of the Mesh, then we might have picked a Mesh triangle,
    // so do the ray/triangle tests.
    //
    // If we did not hit the bounding box, then it is impossible that we hit 
    // the Mesh, so do not waste effort doing ray/triangle tests.

    // Assume we have not picked anything yet, so init to -1.
    m_PickedTriangle = -1;
    float tmin = 0.0f;
    //if (XNA::IntersectRayAxisAlignedBox(rayOrigin, rayDir, &mMeshBox, &tmin))

    if (m_Meshbox.Intersects(rayOrigin, rayDir, tmin))
    {
        // Find the nearest ray/triangle intersection.
        tmin = FLT_MAX;// MathHelper::Infinity;
        for (UINT i = 0; i < m_DxModel->Indices.size() / 3; ++i)
        {
            // Indices for this triangle.
            UINT i0 = m_DxModel->Indices[i * 3 + 0];
            UINT i1 = m_DxModel->Indices[i * 3 + 1];
            UINT i2 = m_DxModel->Indices[i * 3 + 2];

            // Vertices for this triangle.
            auto& vertex0 = m_DxModel->Vertices[i0];
            auto& vertex1 = m_DxModel->Vertices[i1];
            auto& vertex2 = m_DxModel->Vertices[i2];

            DirectX::FXMVECTOR v0 = DirectX::XMVectorSet(vertex0.x, vertex0.y, vertex0.z, 1.0f);
            DirectX::GXMVECTOR v1 = DirectX::XMVectorSet(vertex1.x, vertex1.y, vertex1.z, 1.0f);
            DirectX::HXMVECTOR v2 = DirectX::XMVectorSet(vertex2.x, vertex2.y, vertex2.z, 1.0f);

            // We have to iterate over all the triangles in order to find the nearest intersection.
            float t = 0.0f;
            if (DirectX::TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t))
            {
                if (t < tmin)
                {
                    // This is the new nearest picked triangle.
                    tmin = t;
                    m_PickedTriangle = i;

                    //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Test", "Hitbox", nullptr);

                    vertex0.colour.r = 0.0f;
                    vertex1.colour.r = 0.0f;
                    vertex2.colour.r = 0.0f;

                    vertex0.colour.g = 1.0f;
                    vertex1.colour.g = 1.0f;
                    vertex2.colour.g = 1.0f;

                    m_DxModel->CreateVertexBufferAgain();
                }
            }
        }
    }
}
