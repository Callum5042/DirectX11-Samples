#pragma once

#include <memory>
#include <SDL_video.h>
#include "Timer.h"
#include "DxRenderer.h"
#include "DxModel.h"
#include "DxShader.h"

class Applicataion
{
public:
	Applicataion() = default;
	virtual ~Applicataion();

	int Execute();

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
};