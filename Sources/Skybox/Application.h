#pragma once

#include <memory>
#include <SDL_video.h>
#include "Timer.h"
#include "DxRenderer.h"
#include "DxModel.h"
#include "DxShader.h"
#include "DxCamera.h"
#include "DxSky.h"

class Applicataion
{
public:
	Applicataion() = default;
	virtual ~Applicataion();

	int Execute();

	void UpdateWorldBuffer();

private:
	// SDL window
	bool SDLInit();
	void SDLCleanup();
	SDL_Window* m_SdlWindow = nullptr;

	// Timer
	Timer m_Timer;
	void CalculateFramesPerSecond();

	// Direct3D 11 renderer
	std::unique_ptr<DX::Renderer> m_DxRenderer = nullptr;
	
	// Direct3D 11 model
	std::unique_ptr<DX::Model> m_DxModel = nullptr;

	// Direct3D 11 shader
	std::unique_ptr<DX::Shader> m_DxShader = nullptr;

	// Direct3D 11 perspective camera
	std::unique_ptr<DX::Camera> m_DxCamera = nullptr;

	// Direct3D 11 sky
	std::unique_ptr<DX::Sky> m_DxSky = nullptr;
};