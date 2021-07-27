#include "GeometryGenerator.h"

void GeometryGenerator::CreateBox(float width, float height, float depth, DX::Model* model)
{
	// Vertices
	DX::Vertex vertices[] =
	{
		{ -width, -height, -depth, +0.0f, +0.0f, -1.0f, +0.0f, +1.0f, +1.0f, +0.0f, 0.0f  },
		{ -width, +height, -depth, +0.0f, +0.0f, -1.0f, +0.0f, +0.0f, +1.0f, +0.0f, 0.0f  },
		{ +width, +height, -depth, +0.0f, +0.0f, -1.0f, +1.0f, +0.0f, +1.0f, +0.0f, 0.0f  },
		{ +width, -height, -depth, +0.0f, +0.0f, -1.0f, +1.0f, +1.0f, +1.0f, +0.0f, 0.0f  },

		{ -width, -height, +depth, +0.0f, +0.0f, +1.0f, +1.0f, +1.0f, -1.0f, +0.0f, 0.0f },
		{ +width, -height, +depth, +0.0f, +0.0f, +1.0f, +0.0f, +1.0f, -1.0f, +0.0f, 0.0f },
		{ +width, +height, +depth, +0.0f, +0.0f, +1.0f, +0.0f, +0.0f, -1.0f, +0.0f, 0.0f },
		{ -width, +height, +depth, +0.0f, +0.0f, +1.0f, +1.0f, +0.0f, -1.0f, +0.0f, 0.0f },

		{ -width, +height, -depth, +0.0f, +1.0f, +0.0f, +0.0f, +1.0f, +1.0f, +0.0f, +0.0f },
		{ -width, +height, +depth, +0.0f, +1.0f, +0.0f, +0.0f, +0.0f, +1.0f, +0.0f, +0.0f },
		{ +width, +height, +depth, +0.0f, +1.0f, +0.0f, +1.0f, +0.0f, +1.0f, +0.0f, +0.0f },
		{ +width, +height, -depth, +0.0f, +1.0f, +0.0f, +1.0f, +1.0f, +1.0f, +0.0f, +0.0f },

		{ -width, -height, -depth, +0.0f, -1.0f, +0.0f, +1.0f, +1.0f, -1.0f, +0.0f, +0.0f },
		{ +width, -height, -depth, +0.0f, -1.0f, +0.0f, +0.0f, +1.0f, -1.0f, +0.0f, +0.0f },
		{ +width, -height, +depth, +0.0f, -1.0f, +0.0f, +0.0f, +0.0f, -1.0f, +0.0f, +0.0f },
		{ -width, -height, +depth, +0.0f, -1.0f, +0.0f, +1.0f, +0.0f, -1.0f, +0.0f, +0.0f },

		{ -width, -height, +depth, -1.0f, +0.0f, +0.0f, +0.0f, +1.0f, -0.0f, +0.0f, -1.0f },
		{ -width, +height, +depth, -1.0f, +0.0f, +0.0f, +0.0f, +0.0f, -0.0f, +0.0f, -1.0f },
		{ -width, +height, -depth, -1.0f, +0.0f, +0.0f, +1.0f, +0.0f, -0.0f, +0.0f, -1.0f },
		{ -width, -height, -depth, -1.0f, +0.0f, +0.0f, +1.0f, +1.0f, -0.0f, +0.0f, -1.0f },

		{ +width, -height, -depth, +1.0f, +0.0f, +0.0f, +0.0f, +1.0f, +0.0f, +0.0f, +1.0f },
		{ +width, +height, -depth, +1.0f, +0.0f, +0.0f, +0.0f, +0.0f, +0.0f, +0.0f, +1.0f },
		{ +width, +height, +depth, +1.0f, +0.0f, +0.0f, +1.0f, +0.0f, +0.0f, +0.0f, +1.0f },
		{ +width, -height, +depth, +1.0f, +0.0f, +0.0f, +1.0f, +1.0f, +0.0f, +0.0f, +1.0f }
	};

	// Indices
	unsigned int indices[] =
	{
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23,
	};

	model->Vertices.assign(&vertices[0], &vertices[24]);
	model->Indices.assign(&indices[0], &indices[36]);
}
