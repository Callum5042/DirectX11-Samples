#pragma once

#include "DxRenderer.h"
#include <DirectXMath.h>
#include "Vertex.h"

namespace DX
{
	class Overlay
	{
	public:
		Overlay(DX::Renderer* renderer);
		virtual ~Overlay() = default;

		// Create device
		void Create();

		// Render the model
		void Render();

	private:
		DX::Renderer* m_DxRenderer = nullptr;

		// Vertex buffer
		ComPtr<ID3D11Buffer> m_d3dVertexBuffer = nullptr;
		void CreateVertexBuffer();

		// Index buffer
		ComPtr<ID3D11Buffer> m_d3dIndexBuffer = nullptr;
		void CreateIndexBuffer();

		// Mesh data
		std::vector<DX::OverlayVertex> m_Vertices;
		std::vector<UINT> m_Indices;

		// Texture sampler
		ComPtr<ID3D11SamplerState> m_AnisotropicSampler = nullptr;
		void CreateAnisotropicFiltering();
	};
}