#pragma once

#include <DirectXMath.h>

namespace DX
{
	struct WorldBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};
}