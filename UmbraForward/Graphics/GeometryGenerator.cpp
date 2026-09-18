#include "GeometryGenerator.h"

//圆柱
GeometryGenerator::MeshData GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount) {
	MeshData meshData;

	//单层高度
	float stackHeight = height / stackCount;

	//每向上一层的半径变化量
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	//环的数量
	int ringCount = stackCount + 1;

	//同一层每个顶点对应的角度增量
	float dTheta =DirectX::XM_2PI / sliceCount;

	//从最下面的环开始，逐层创建顶点
	for (int i = 0; i < ringCount; i++) {
		//计算当前环的半径和高度
		float y = -0.5f * height + i * stackHeight;   //其中，以高度的一半为远点，则底部的y值为-0.5*高
		float r = bottomRadius + i * radiusStep;

		//计算当前环的顶点
		for (int j = 0; j <= sliceCount; j++) {
			Vertex vertex;

			//当前顶点坐标：（cosθ * r, y, sinθ * r）
			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.Position = DirectX::XMFLOAT3(c*r, y, s*r);

			//计算纹理坐标
			vertex.TexC.x = (float)j / sliceCount;
			vertex.TexC.y = 1.0f - (float)i / stackCount;

			//计算切线
			vertex.TangentU = DirectX::XMFLOAT3(-s, 0.0f, c);   //水平切线
			float dr = bottomRadius - topRadius;
			DirectX::XMFLOAT3 bitangent(dr * c, -height, dr * s);    //垂直切线

			//在寄存器中计算法线
			DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&vertex.TangentU);
			DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&bitangent);
			DirectX::XMVECTOR N = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(T, B));

			//将寄存器中的结果存储到顶点的法线中
			DirectX::XMStoreFloat3(&vertex.Normal, N);

			//将顶点添加到网格数据中
			meshData.Vertices.push_back(vertex);
		}

		//计算索引数组
		int ringVertexCount = sliceCount + 1;     //每层的顶点数量，为了保证纹理坐标的连续性，最后一个顶点与第一个顶点重合，因此会多出一个点

		for (int i = 0; i < stackCount; i++) {
			for(int j=0; j < sliceCount; j++) {
				meshData.Indices32.push_back(i * ringVertexCount + j);
				meshData.Indices32.push_back((i + 1) * ringVertexCount + j);
				meshData.Indices32.push_back((i + 1) * ringVertexCount + j + 1);
				meshData.Indices32.push_back(i * ringVertexCount + j);
				meshData.Indices32.push_back((i + 1) * ringVertexCount + j + 1);
				meshData.Indices32.push_back(i * ringVertexCount + j + 1);
			}
		}

		BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
		BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);

		return meshData;
	}
}

void GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData) {
	//记录顶盖索引时的起始索引
	//这个meshData先前已经存储了侧面顶点和索引
	uint32 baseIndex = static_cast<uint32>(meshData.Vertices.size());

	float y = 0.5f * height;
	float dTheta = DirectX::XM_2PI / sliceCount;

	for(int i=0;i<=sliceCount;i++) {
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);
		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, (float)i / sliceCount, 0.0f));
	}

	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f));

	//索引
	for(int i=0;i<sliceCount;i++) {
		meshData.Indices32.push_back(baseIndex + sliceCount + 1);
		meshData.Indices32.push_back(baseIndex + i+1);
		meshData.Indices32.push_back(baseIndex + i);
	}
}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, MeshData& meshData) {
	
	uint32 baseIndex = static_cast<uint32>(meshData.Vertices.size());
	float y = -0.5f * height;
	float dTheta = DirectX::XM_2PI / sliceCount;

	//计算顶点
	for(int i=0;i<=sliceCount;i++) {
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);
		meshData.Vertices.push_back(Vertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, (float)i / sliceCount, 1.0f));
	}

	meshData.Vertices.push_back(Vertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.0f));
	
	//底面索引
	for(int i=0;i<sliceCount;i++) {
		meshData.Indices32.push_back(baseIndex + sliceCount + 1);
		meshData.Indices32.push_back(baseIndex + i);
		meshData.Indices32.push_back(baseIndex + i + 1);
	}
}

GeometryGenerator::MeshData GeometryGenerator::CreateGeosphere(float radius, uint32 numSubdivisions) {
	MeshData meshData;

	//限制最大细分次数为6，防止顶点数量过多
	//每次细分将一个面切分为4个三角面
	numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

	const float x = 0.525731f;
	const float z = 0.850651f;

	DirectX::XMFLOAT3 pos[12] = {
		DirectX::XMFLOAT3(-x, 0.0f, z), DirectX::XMFLOAT3(x, 0.0f, z),
		DirectX::XMFLOAT3(-x, 0.0f, -z), DirectX::XMFLOAT3(x, 0.0f, -z),
		DirectX::XMFLOAT3(0.0f, z, x), DirectX::XMFLOAT3(0.0f, z, -x),
		DirectX::XMFLOAT3(0.0f, -z, x), DirectX::XMFLOAT3(0.0f, -z, -x),
		DirectX::XMFLOAT3(z, x, 0.0f), DirectX::XMFLOAT3(-z, x, 0.0f),
		DirectX::XMFLOAT3(z, -x, 0.0f), DirectX::XMFLOAT3(-z, -x, 0.0f)
	};

	uint32 k[60] = {
		1,4,0, 4,9,0, 4,5,9, 8,5,4, 1,8,4,
		1,10,8, 10,3,8, 8,3,5, 3,2,5, 3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,
		11,2,7
	};

	//重置顶点数组长度，并将索引数组和顶点数组数据拷贝到meshData中
	meshData.Vertices.resize(12);
	meshData.Indices32.assign(&k[0], &k[60]);
	for (int i = 0; i < 12; i++) {
		meshData.Vertices[i].Position = pos[i];
	}

	//细分
	for (int i = 0; i < numSubdivisions; i++) {
		Subdivide(meshData);
	}

	//将顶点投影到球面上，并计算法线和纹理坐标
	for (int i = 0; i < meshData.Vertices.size(); i++) {
		//计算顶点在球面上的位置：归一化向量（法线）*半径
		DirectX::XMVECTOR n = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&meshData.Vertices[i].Position));
		DirectX::XMVECTOR p = DirectX::XMVectorScale(n, radius);

		//将法线和投影后的坐标装载到顶点数据中
		DirectX::XMStoreFloat3(&meshData.Vertices[i].Position, p);
		DirectX::XMStoreFloat3(&meshData.Vertices[i].Normal, n);

		//计算纹理坐标
		//u
		float theta = atan2f(meshData.Vertices[i].Position.z, meshData.Vertices[i].Position.x);
		if (theta < 0.0f) {
			theta += DirectX::XM_2PI;
		}
		meshData.Vertices[i].TexC.x = theta / DirectX::XM_2PI;

		//v
		float phi = acosf(meshData.Vertices[i].Position.y / radius);
		meshData.Vertices[i].TexC.y = phi / DirectX::XM_PI;

		//U方向切线
		meshData.Vertices[i].TangentU.x = -radius * sinf(phi) * sinf(theta);
		meshData.Vertices[i].TangentU.y = 0.0f;
		meshData.Vertices[i].TangentU.z = +radius * sinf(phi) * cosf(theta);

		DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&meshData.Vertices[i].TangentU);
		DirectX::XMStoreFloat3(&meshData.Vertices[i].TangentU, DirectX::XMVector3Normalize(T));
	}

	return meshData;
}


//细分网格数据，将每个三角形细分为4个更小的三角形
	void GeometryGenerator::Subdivide(MeshData & meshData){
		MeshData inputCopy = meshData;
		meshData.Vertices.resize(0);
		meshData.Indices32.resize(0);

		//细分前的三角面数量
		uint32 numTirs = static_cast<uint32>(inputCopy.Indices32.size()) / 3;
		//逐三角面进行细分
		for (uint32 i = 0; i < numTirs; i++) {
			//读取原本的顶点
			Vertex v0 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 0]];
			Vertex v1 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 1]];
			Vertex v2 = inputCopy.Vertices[inputCopy.Indices32[i * 3 + 2]];

			//计算中点顶点
			Vertex m0 = MidPoint(v0, v1);
			Vertex m1 = MidPoint(v1, v2);
			Vertex m2 = MidPoint(v0, v2);

			//将顶点和索引数据添加到meshData中
			meshData.Vertices.push_back(v0); 
			meshData.Vertices.push_back(v1); 
			meshData.Vertices.push_back(v2); 
			meshData.Vertices.push_back(m0);
			meshData.Vertices.push_back(m1);
			meshData.Vertices.push_back(m2);

			meshData.Indices32.push_back(i * 6 + 0);
			meshData.Indices32.push_back(i * 6 + 3);
			meshData.Indices32.push_back(i * 6 + 5);

			meshData.Indices32.push_back(i * 6 + 3);
			meshData.Indices32.push_back(i * 6 + 1);
			meshData.Indices32.push_back(i * 6 + 4);

			meshData.Indices32.push_back(i * 6 + 5);
			meshData.Indices32.push_back(i * 6 + 4);
			meshData.Indices32.push_back(i * 6 + 2);

			meshData.Indices32.push_back(i * 6 + 3);
			meshData.Indices32.push_back(i * 6 + 4);
			meshData.Indices32.push_back(i * 6 + 5);

		}

}

	//计算两个顶点的中点，并返回一个新的顶点，用于细分
	GeometryGenerator::Vertex GeometryGenerator::MidPoint(const Vertex& v0, const Vertex& v1) {
		DirectX::XMVECTOR p0 = DirectX::XMLoadFloat3(&v0.Position);
		DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&v1.Position);

		DirectX::XMVECTOR n0 = DirectX::XMLoadFloat3(&v0.Normal);
		DirectX::XMVECTOR n1 = DirectX::XMLoadFloat3(&v1.Normal);

		DirectX::XMVECTOR t0 = DirectX::XMLoadFloat3(&v0.TangentU);
		DirectX::XMVECTOR t1 = DirectX::XMLoadFloat3(&v1.TangentU);

		DirectX::XMVECTOR tex0 = DirectX::XMLoadFloat2(&v0.TexC);
		DirectX::XMVECTOR tex1 = DirectX::XMLoadFloat2(&v1.TexC);

		DirectX::XMVECTOR pos = DirectX::XMVectorScale(DirectX::XMVectorAdd(p0,p1),0.5);
		DirectX::XMVECTOR normal = DirectX::XMVectorScale(DirectX::XMVectorAdd(n0, n1), 0.5);
		normal = DirectX::XMVector3Normalize(normal);
		DirectX::XMVECTOR tangent = DirectX::XMVectorScale(DirectX::XMVectorAdd(t0, t1), 0.5);
		tangent = DirectX::XMVector3Normalize(tangent);
		DirectX::XMVECTOR tex = DirectX::XMVectorScale(DirectX::XMVectorAdd(tex0, tex1), 0.5);

		Vertex v;
		DirectX::XMStoreFloat3(&v.Position, pos);
		DirectX::XMStoreFloat3(&v.Normal, normal);
		DirectX::XMStoreFloat3(&v.TangentU, tangent);
		DirectX::XMStoreFloat2(&v.TexC, tex);

		return v;
	}

	GeometryGenerator::MeshData GeometryGenerator::CreateBox(float width, float height, float depth, uint32 numSubdivisions) {
		MeshData meshData;
		//限制最大细分次数为6，防止顶点数量过多
		numSubdivisions = std::min<uint32>(numSubdivisions, 6u);

		//计算顶点坐标
		float w = 0.5f * width;
		float h = 0.5f * height;
		float d = 0.5f * depth;

		//front,left,back,right,top,bottom
		Vertex vertices[24] = {
			Vertex(-w, -h, -d, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
			Vertex(-w, +h, -d, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
			Vertex(+w, +h, -d, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
			Vertex(+w, -h, -d, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f),

			Vertex(-w, -h, +d, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f),
			Vertex(-w,+h,+d,-1.0f,0.0f,0.0f,0.0f,0.0f,-1.0f,0.0f,0.0f),
			Vertex(-w,+h,-d,-1.0f,0.0f,0.0f,0.0f,0.0f,-1.0f,1.0f,0.0f),
			Vertex(-w,-h,-d,-1.0f,0.0f,0.0f,0.0f,0.0f,-1.0f,1.0f,1.0f),

			Vertex(+w,-h,+d,0.0f,0.0f,1.0f,-1.0f,0.0f,0.0f,0.0f,1.0f),
			Vertex(+w,+h,+d,0.0f,0.0f,1.0f,-1.0f,0.0f,0.0f,0.0f,0.0f),
			Vertex(-w,+h,+d,0.0f,0.0f,1.0f,-1.0f,0.0f,0.0f,1.0f,0.0f),
			Vertex(-w,-h,+d,0.0f,0.0f,1.0f,-1.0f,0.0f,0.0f,1.0f,1.0f),

			Vertex(+w,-h,-d,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,1.0f),
			Vertex(+w,+h,-d,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f),
			Vertex(+w,+h,+d,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f,0.0f),
			Vertex(+w,-h,+d,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f),

			Vertex(-w,+h,-d,0.0f,1.0f,0.0f,1.0f,0.0f,0.0f,0.0f,1.0f),
			Vertex(-w,+h,+d,0.0f,1.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f),
			Vertex(+w,+h,+d,0.0f,1.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f),
			Vertex(+w,+h,-d,0.0f,1.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f),

			Vertex(-w,-h,+d,0.0f,-1.0f,0.0f,1.0f,0.0f,0.0f,0.0f,1.0f),
			Vertex(-w,-h,-d,0.0f,-1.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f),
			Vertex(+w,-h,-d,0.0f,-1.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f),
			Vertex(+w,-h,+d,0.0f,-1.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f)
		};


		//将顶点数据添加到meshData中
		meshData.Vertices.assign(&vertices[0], &vertices[24]);

		//填充索引数据
		uint32 indices[36] = {
			0,1,2, 0,2,3,
			4,5,6, 4,6,7,
			8,9,10, 8,10,11,
			12,13,14, 12,14,15,
			16,17,18, 16,18,19,
			20,21,22, 20,22,23
		};
		meshData.Indices32.assign(&indices[0], &indices[36]);

		//细分
		for(int i=0;i< numSubdivisions; i++) {
			Subdivide(meshData);
		}

		return meshData;
	}


	//接收左上角顶点的xy坐标，宽度，高度和深度,
	//创建一个位于 z = depth 平面上的四边形
	GeometryGenerator::MeshData GeometryGenerator::CreateQuad(float x, float y, float w, float h, float depth) {
		MeshData meshData;

		meshData.Vertices.resize(4);
		Vertex verties[4] = {
			Vertex(x, y - h, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f),
			Vertex(x, y, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f),
			Vertex(x + w, y, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f),
			Vertex(x + w, y - h, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f)
		};
		meshData.Vertices.assign(&verties[0], &verties[4]);
		meshData.Indices32.assign({ 0,1,2, 0,2,3 });

		return meshData;
	}

	//接收平面的宽度、深度和行列的顶点数
	// 创建一个平行于y=0平面、以原点为中心的网格
	GeometryGenerator::MeshData GeometryGenerator::CreateGrid(float width, float depth, uint32 n, uint32 m) {
		assert(width > 0.0f);
		assert(depth > 0.0f);
		assert(m >= 2);
		assert(n >= 2);

		MeshData meshData;
	
		meshData.Vertices.resize(m * n);
		meshData.Indices32.resize((m - 1) * (n - 1) * 6);

		float patchWidth = width / (m - 1);
		float patchDepth = depth / (n - 1);
		float halfWidth = 0.5f * width;
		float halfDepth = 0.5f * depth;

		//从左下角（-halfWidth，0，-halfDepth）开始，逐行逐列创建顶点
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < m; j++) {
				float w = -halfWidth + j * patchWidth;
				float d = -halfDepth + i * patchDepth;
				float u = (float)j / (m - 1);
				float v = (float)i / (n - 1);

				meshData.Vertices[i * m + j] = Vertex(w,0,d,0.0f,1.0f,0.0f,1.0f,0.0f,0.0f,u,v);
			}
		}

		//填充索引数据
		int numRows = n - 1;
		int numCols = m - 1;
		for (int i = 0; i < numRows; i++) {
			for (int j = 0; j < numCols; j++) {
				meshData.Indices32[(i * numCols + j)*6] = i * m + j;
				meshData.Indices32[(i * numCols + j)*6 + 1] = (i + 1) * m + j;
				meshData.Indices32[(i * numCols + j)*6 + 2] = (i + 1) * m + j + 1;
				
				meshData.Indices32[(i * numCols + j)*6 + 3] = i * m + j;
				meshData.Indices32[(i * numCols + j)*6 + 4] = (i + 1) * m + j + 1;
				meshData.Indices32[(i * numCols + j)*6 + 5] = i * m + j + 1;
			}
		}

		return meshData;
	}