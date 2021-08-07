#pragma once

#include <memory>
#include <SDL_video.h>
#include "Timer.h"
#include "DxRenderer.h"
#include "DxShader.h"
#include "DxCamera.h"

#include "DxModel.h"
#include "DxFloor.h"
#include "DxPointLight.h"

#include "DxSky.h"
#include "DxSkyShader.h"

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
	std::unique_ptr<DX::Floor> m_DxFloor = nullptr;

	// Point Light
	std::unique_ptr<DX::PointLight> m_DxPointLight = nullptr;

	// Direct3D 11 shader
	std::unique_ptr<DX::Shader> m_DxShader = nullptr;
	std::unique_ptr<DX::Shader> m_DxShadowMapShader = nullptr;

	// Direct3D 11 sky
	std::unique_ptr<DX::Sky> m_DxSky = nullptr;

	// Direct3D 11 sky shader
	std::unique_ptr<DX::SkyShader> m_DxSkyShader = nullptr;

	// Direct3D 11 perspective camera
	std::unique_ptr<DX::Camera> m_DxCamera = nullptr;

	// Update buffers
	void SetCameraBuffer();

	// Move point light
	void MovePointLight();

	ComPtr<ID3D11RenderTargetView> m_RenderTargetView = nullptr;
};