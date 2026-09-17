#pragma once
#include <cstdint>
#include <vector>
#include "d3d12.h"
#include "DirectXMath.h"

// A class that generates geometric shapes and their vertex/index data for rendering.
class GeometryGenerator {
public:
	using uint16 = std::uint16_t;
	using uint32 = std::uint32_t;

	// Vertex structure represents a single vertex with position, normal, tangent, and texture coordinates.
	// Instantiation of this structure needs float data or DirectX::XMFLOAT3/XMFLOAT2 data .
	struct Vertex {
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;    //法线
		DirectX::XMFLOAT3 TangentU;  //U方向（水平）切线
		DirectX::XMFLOAT2 TexC;     //归一化的纹理坐标

		Vertex() = default;
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v
		):
			Position(px, py, pz),
			Normal(nx, ny, nz),
			TangentU(tx, ty, tz),
			TexC(u, v){}
		
		Vertex(
			const DirectX::XMFLOAT3& position,
			const DirectX::XMFLOAT3& normal,
			const DirectX::XMFLOAT3& tangentU,
			const DirectX::XMFLOAT2& texC
		) :
			Position(position),
			Normal(normal),
			TangentU(tangentU),
			TexC(texC) {}
	};

	struct MeshData {
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices32;

		std::vector<uint16>& GetIndices16(){
			if(mIndices16.empty()) {
				mIndices16.resize(Indices32.size());
				for(size_t i = 0; i < Indices32.size(); ++i) {
					mIndices16[i] = static_cast<uint16>(Indices32[i]);
				}
			}
			return mIndices16;
		}

	private:
		std::vector<uint16> mIndices16;
	};
	
	MeshData CreateBox(float width, float height, float depth, uint32 numSubdivisions);

	//MeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);

	MeshData CreateGeosphere(float radius, uint32 numSubdivisions);

	MeshData CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);

	MeshData CreateGrid(float width, float depth, uint32 m, uint32 n);

	
	MeshData CreateQuad(float x, float y, float w, float h, float depth);

private:

	void Subdivide(MeshData& meshData);
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData);
	Vertex MidPoint(const Vertex& v0, const Vertex& v1);
	
};
