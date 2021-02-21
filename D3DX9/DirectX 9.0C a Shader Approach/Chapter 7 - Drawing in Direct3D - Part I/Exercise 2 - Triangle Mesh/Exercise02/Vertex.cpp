#include "Vertex.h"
#include "d3dUtil.h"

// Initialise static variables
IDirect3DVertexDeclaration9* VertexPos::Decl = 0;

void InitAllVertexDeclarations(void)
{
	D3DVERTEXELEMENT9 VertexPosElements[] =
	{
		{
			0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0
		},
		D3DDECL_END()
	};
	HR(gd3dDevice->CreateVertexDeclaration(VertexPosElements, &VertexPos::Decl));
}

void DestroyAllVertexDeclarations(void)
{
	ReleaseCOM(VertexPos::Decl);
}