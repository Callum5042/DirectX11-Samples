#pragma once

#include <memory>
#include <SDL_video.h>
#include "Timer.h"
#include "DxRenderer.h"

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
};