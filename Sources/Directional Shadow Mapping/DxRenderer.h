#pragma once

#include <exception>
#include <SDL_video.h>
#include <d3d11_1.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// This include is requires for using DirectX smart pointers (ComPtr)
#include <wrl\client.h>
using Microsoft::WRL::ComPtr;

namespace DX
{
	// Throw if the function result has failed. For ease of error handling
	inline void Check(HRESULT hr)
	{
#ifdef _DEBUG
		if (FAILED(hr))
		{
			throw std::exception();
		}
#endif
	}

	// DirectX requires the Win32 window (HWND)
	HWND GetHwnd(SDL_Window* window);

	// DirectX rendering class
	class Renderer
	{
	public:
		Renderer(SDL_Window* window);
		virtual ~Renderer() = default;

		// Setup functions
		void Create();

		// Resizing
		void Resize(int width, int height);

		// Display the rendered scene
		void Present();

		// Get the ID3D11 Device
		ID3D11Device* GetDevice() const { return m_d3dDevice.Get(); }

		// Get the ID3D11 Device Context
		ID3D11DeviceContext* GetDeviceContext() const { return m_d3dDeviceContext.Get(); }

		// Set render target back buffer
		void SetRenderTargetBackBuffer();

		// Void render target texture
		void SetRenderTargetShadowMap();

		// Viewport
		void SetViewport(int width, int height);

		// Get texture
		ID3D11ShaderResourceView** GetShadowMapTexture() { return m_ShadowMapTexture.GetAddressOf(); }

		// Set cull mode
		void SetRasterBackCull();
		void SetRasterBackCullShadow();

	private:
		SDL_Window* m_SdlWindow = nullptr;

		// Device and device context
		ComPtr<ID3D11Device> m_d3dDevice = nullptr;
		ComPtr<ID3D11DeviceContext> m_d3dDeviceContext = nullptr;
		void CreateDeviceAndContext();

		// Swapchain
		ComPtr<IDXGISwapChain> m_d3dSwapChain = nullptr;
		ComPtr<IDXGISwapChain1> m_d3dSwapChain1 = nullptr;
		void CreateSwapChain(int width, int height);

		// Render target and depth stencil view
		ComPtr<ID3D11RenderTargetView> m_d3dRenderTargetView = nullptr;
		ComPtr<ID3D11DepthStencilView> m_d3dDepthStencilView = nullptr;
		void CreateRenderTargetAndDepthStencilView(int width, int height);

		// Shadow sampler
		ComPtr<ID3D11SamplerState> m_ShadowSampler = nullptr;
		void CreateShadowFiltering();

		// Raster modes
		void CreateRasterModeBackCull();
		ComPtr<ID3D11RasterizerState> m_RasterModelBack = nullptr;

		void CreateRasterModeBackCullShadow();
		ComPtr<ID3D11RasterizerState> m_RasterModelBackShadow = nullptr;

		// Render to texture
		ComPtr<ID3D11DepthStencilView> m_ShadowMapDepthStencilView = nullptr;
		ComPtr<ID3D11ShaderResourceView> m_ShadowMapTexture = nullptr;
		void CreateTextureDepthStencilView();
		float m_ShadowMapTextureSize = 4096.0f;
	};
}