#include "DxRenderer.h"
#include <SDL.h>
#include <DirectXColors.h>

// Required for using SDL_SysWMinfo
#include <SDL_syswm.h>

HWND DX::GetHwnd(SDL_Window* window)
{
	SDL_SysWMinfo wmInfo = {};
	SDL_GetVersion(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	return wmInfo.info.win.window;
}

DX::Renderer::Renderer(SDL_Window* window) : m_SdlWindow(window)
{
}

void DX::Renderer::Create()
{
	// Query window size
	auto window_width = 0;
	auto window_height = 0;
	SDL_GetWindowSize(m_SdlWindow, &window_width, &window_height);

	// Setup Direct3D 11
	CreateDeviceAndContext();
	CreateSwapChain(window_width, window_height);
	CreateRenderTargetAndDepthStencilView(window_width, window_height);
	SetViewport(window_width, window_height);

	// Create render to texture depth stencil
	/*const int MAX_CASCADES = 3;
	m_ShadowMapDepthStencilViews.resize(MAX_CASCADES);
	m_ShadowMapTextures.resize(MAX_CASCADES);

	for (int cascade_level = 0; cascade_level < MAX_CASCADES; ++cascade_level)
	{
	}*/
	CreateTextureDepthStencilView(0);

	CreateRasterModeBackCull();
	CreateRasterModeBackCullShadow();
}

void DX::Renderer::Resize(int width, int height)
{
	// Releases the current render target and depth stencil view
	m_d3dDepthStencilView.ReleaseAndGetAddressOf();
	m_d3dRenderTargetView.ReleaseAndGetAddressOf();

	// Resize the swapchain
	DX::Check(m_d3dSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING));

	// Creates a new render target and depth stencil view with the new window size
	CreateRenderTargetAndDepthStencilView(width, height);

	// Sets a new viewport with the new window size
	SetViewport(width, height);
}

void DX::Renderer::SetRenderTargetBackBuffer()
{
	// Clear the render target view to the chosen colour
	m_d3dDeviceContext->ClearRenderTargetView(m_d3dRenderTargetView.Get(), reinterpret_cast<const float*>(&DirectX::Colors::SteelBlue));
	m_d3dDeviceContext->ClearDepthStencilView(m_d3dDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Bind the render target view to the pipeline's output merger stage
	m_d3dDeviceContext->OMSetRenderTargets(1, m_d3dRenderTargetView.GetAddressOf(), m_d3dDepthStencilView.Get());
}

void DX::Renderer::SetRenderTargetShadowMap(int cascade_level)
{
	// Clear the render target view to the chosen colour
	m_d3dDeviceContext->ClearDepthStencilView(m_ShadowMapDepthStencilViews[cascade_level].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Bind the render target view to the pipeline's output merger stage
	ID3D11RenderTargetView* render_target_views[1] = { nullptr };
	m_d3dDeviceContext->OMSetRenderTargets(1, render_target_views, m_ShadowMapDepthStencilViews[cascade_level].Get());

	// Describe the viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(m_ShadowMapTextureSize);
	viewport.Height = static_cast<float>(m_ShadowMapTextureSize);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	// Bind viewport to the pipline's rasterization stage
	m_d3dDeviceContext->RSSetViewports(1, &viewport);
}

void DX::Renderer::Present()
{
	// Check if we support IDXGISwapChain1
	if (m_d3dSwapChain1 != nullptr)
	{
		// Use IDXGISwapChain1::Present1 for presenting instead
		// This is a requirement for using variable refresh rate displays
		DXGI_PRESENT_PARAMETERS presentParameters = {};
		DX::Check(m_d3dSwapChain1->Present1(0, 0, &presentParameters));
	}
	else
	{
		DX::Check(m_d3dSwapChain->Present(0, 0));
	}
}

void DX::Renderer::CreateDeviceAndContext()
{
	// Look for Direct3D 11 feature
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	// Add debug flag if in debug mode
	D3D11_CREATE_DEVICE_FLAG deviceFlag = (D3D11_CREATE_DEVICE_FLAG)0;
#ifdef _DEBUG
	deviceFlag = D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create device and device context
	D3D_FEATURE_LEVEL featureLevel;
	DX::Check(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlag, featureLevels, numFeatureLevels,
			  D3D11_SDK_VERSION, m_d3dDevice.ReleaseAndGetAddressOf(), &featureLevel, m_d3dDeviceContext.ReleaseAndGetAddressOf()));

	// Check if Direct3D 11.1 is supported, if not fall back to Direct3D 11
	if (featureLevel != D3D_FEATURE_LEVEL_11_1)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "D3D_FEATURE_LEVEL_11_1 is not supported! Falling back to D3D_FEATURE_LEVEL_11_0", nullptr);
	}

	// Check if Direct3D 11 is supported
	if (featureLevel != D3D_FEATURE_LEVEL_11_1)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "D3D_FEATURE_LEVEL_11_0 is not supported", nullptr);
		throw std::exception();
	}
}

void DX::Renderer::CreateSwapChain(int width, int height)
{
	// Get the Win32 window from SDL_Window
	auto hwnd = DX::GetHwnd(m_SdlWindow);

	// Query the device until we get the DXGIFactory
	ComPtr<IDXGIDevice> dxgiDevice = nullptr;
	DX::Check(m_d3dDevice.As(&dxgiDevice));

	ComPtr<IDXGIAdapter> adapter = nullptr;
	DX::Check(dxgiDevice->GetAdapter(adapter.GetAddressOf()));

	ComPtr<IDXGIFactory> dxgiFactory = nullptr;
	DX::Check(adapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(dxgiFactory.GetAddressOf())));

	// Query IDXGIFactory to try to get IDXGIFactory2
	ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;
	DX::Check(dxgiFactory.As(&dxgiFactory2));

	// If we can support IDXGIFactory2 then use it to create the swap chain, otherwise fallback to IDXIFactory
	if (dxgiFactory2 != nullptr)
	{
		// DirectX 11.1
		DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
		swapchain_desc.Width = width;
		swapchain_desc.Height = height;
		swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchain_desc.SampleDesc.Count = 1;
		swapchain_desc.SampleDesc.Quality = 0;
		swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchain_desc.BufferCount = 2;
		swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		// CreateSwapChainForHwnd is the prefered way of creating the swap chain
		DX::Check(dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hwnd, &swapchain_desc, nullptr, nullptr, &m_d3dSwapChain1));
		DX::Check(m_d3dSwapChain1.As(&m_d3dSwapChain));
	}
	else
	{
		// Describe the swapchain
		DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
		swapchain_desc.BufferDesc.Width = width;
		swapchain_desc.BufferDesc.Height = height;
		swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchain_desc.SampleDesc.Count = 1;
		swapchain_desc.SampleDesc.Quality = 0;
		swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchain_desc.BufferCount = 2;
		swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		swapchain_desc.BufferDesc.RefreshRate.Numerator = 60;
		swapchain_desc.BufferDesc.RefreshRate.Denominator = 1;
		swapchain_desc.OutputWindow = hwnd;
		swapchain_desc.Windowed = TRUE;

		// Creates the swapchain
		DX::Check(dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &swapchain_desc, &m_d3dSwapChain));
	}
}

void DX::Renderer::CreateRenderTargetAndDepthStencilView(int width, int height)
{
	// Create the render target view
	ComPtr<ID3D11Texture2D> back_buffer = nullptr;
	DX::Check(m_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(back_buffer.GetAddressOf())));
	DX::Check(m_d3dDevice->CreateRenderTargetView(back_buffer.Get(), nullptr, m_d3dRenderTargetView.GetAddressOf()));

	// Describe the depth stencil view
	D3D11_TEXTURE2D_DESC depth_desc = {};
	depth_desc.Width = width;
	depth_desc.Height = height;
	depth_desc.MipLevels = 1;
	depth_desc.ArraySize = 1;
	depth_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_desc.SampleDesc.Count = 1;
	depth_desc.SampleDesc.Quality = 0;
	depth_desc.Usage = D3D11_USAGE_DEFAULT;
	depth_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	// Create the depth stencil view
	ComPtr<ID3D11Texture2D> depth_stencil = nullptr;
	DX::Check(m_d3dDevice->CreateTexture2D(&depth_desc, nullptr, &depth_stencil));
	DX::Check(m_d3dDevice->CreateDepthStencilView(depth_stencil.Get(), nullptr, m_d3dDepthStencilView.GetAddressOf()));
}

void DX::Renderer::SetViewport(int width, int height)
{
	// Describe the viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	// Bind viewport to the pipline's rasterization stage
	m_d3dDeviceContext->RSSetViewports(1, &viewport);
}

void DX::Renderer::SetRasterBackCull()
{
	m_d3dDeviceContext->RSSetState(m_RasterModelBack.Get());
}

void DX::Renderer::SetRasterBackCullShadow()
{
	m_d3dDeviceContext->RSSetState(m_RasterModelBackShadow.Get());
}

void DX::Renderer::CreateRasterModeBackCull()
{
	D3D11_RASTERIZER_DESC rasterizerState = {};
	rasterizerState.CullMode = D3D11_CULL_BACK;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.DepthClipEnable = true;

	DX::Check(m_d3dDevice->CreateRasterizerState(&rasterizerState, m_RasterModelBack.ReleaseAndGetAddressOf()));
}

void DX::Renderer::CreateRasterModeBackCullShadow()
{
	D3D11_RASTERIZER_DESC rasterizerState = {};
	rasterizerState.CullMode = D3D11_CULL_BACK;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.DepthClipEnable = true;

	rasterizerState.DepthBias = 10000;
	rasterizerState.DepthBiasClamp = 0.0f;
	rasterizerState.SlopeScaledDepthBias = 1.0f;

	DX::Check(m_d3dDevice->CreateRasterizerState(&rasterizerState, m_RasterModelBackShadow.ReleaseAndGetAddressOf()));
}

void DX::Renderer::CreateTextureDepthStencilView(int cascade_level)
{
	// Create texture
	D3D11_TEXTURE2D_DESC texture_desc = {};
	texture_desc.Width = static_cast<UINT>(m_ShadowMapTextureSize);
	texture_desc.Height = static_cast<UINT>(m_ShadowMapTextureSize);
	texture_desc.ArraySize = 3;
	texture_desc.SampleDesc.Count = 1;
	texture_desc.SampleDesc.Quality = 0;
	texture_desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texture_desc.Usage = D3D11_USAGE_DEFAULT;
	texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> texture = nullptr;
	DX::Check(m_d3dDevice->CreateTexture2D(&texture_desc, 0, texture.GetAddressOf()));

	const int MAX_CASCADES = 3;
	m_ShadowMapDepthStencilViews.resize(MAX_CASCADES);
	m_ShadowMapTextures.resize(MAX_CASCADES);

	for (int cascade_level = 0; cascade_level < MAX_CASCADES; ++cascade_level)
	{
		// Create depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_desc = {};
		depth_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		depth_view_desc.Texture2DArray.ArraySize = 1;
		depth_view_desc.Texture2DArray.FirstArraySlice = cascade_level;
		depth_view_desc.Texture2DArray.MipSlice = 0;

		DX::Check(m_d3dDevice->CreateDepthStencilView(texture.Get(), &depth_view_desc, m_ShadowMapDepthStencilViews[cascade_level].GetAddressOf()));
	}

	// Create shadow resource
	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
	shader_resource_view_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	shader_resource_view_desc.Texture2DArray.MostDetailedMip = 0;
	shader_resource_view_desc.Texture2DArray.MipLevels = 1;
	shader_resource_view_desc.Texture2DArray.ArraySize = 3;
	shader_resource_view_desc.Texture2DArray.FirstArraySlice = 0;

	DX::Check(m_d3dDevice->CreateShaderResourceView(texture.Get(), &shader_resource_view_desc, m_ShadowMapTextures[cascade_level].GetAddressOf()));
}
