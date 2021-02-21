#pragma once

#include <d3dx9.h>

// Call in constructor and destructor, respectively, of derived application class
void InitAllVertexDeclarations(void);
void DestroyAllVertexDeclarations(void);

struct VertexPos
{
	VertexPos():pos(0.0f, 0.0f, 0.0f){}
	VertexPos(float x, float y, float z):pos(x,y,z){}
	VertexPos(const D3DXVECTOR3& v):pos(v){}

	D3DXVECTOR3 pos;
	static IDirect3DVertexDeclaration9* Decl;
};