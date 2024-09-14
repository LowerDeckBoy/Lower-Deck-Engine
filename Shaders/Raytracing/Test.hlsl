
#include "Shared.hlsli"

// https://github.com/kcloudy0717/Kaguya/blob/master/Source/Application/Kaguya/Shaders/PathTrace.hlsl

StructuredBuffer<Vertex> VertexBuffer : register(t0, space1);
StructuredBuffer<uint> IndexBuffer : register(t1, space1);

struct VertexAttributes
{
	float3 Position;
	float3 Ng;
	float3 Ns;
	float2 TexCoords;
};

VertexAttributes GetVertexAttributes(BuiltInTriangleIntersectionAttributes Attributes)
{
	// Fetch indices
	uint idx0 = IndexBuffer[PrimitiveIndex() * 3 + 0];
	uint idx1 = IndexBuffer[PrimitiveIndex() * 3 + 1];
	uint idx2 = IndexBuffer[PrimitiveIndex() * 3 + 2];

	// Fetch vertices
	Vertex vtx0 = VertexBuffer[idx0];
	Vertex vtx1 = VertexBuffer[idx1];
	Vertex vtx2 = VertexBuffer[idx2];

	float3 p0 = vtx0.Position, p1 = vtx1.Position, p2 = vtx2.Position;
	// Compute 2 edges of the triangle
	float3 e0 = p1 - p0;
	float3 e1 = p2 - p0;
	float3 n = normalize(cross(e0, e1));
	n = normalize(mul(n, transpose((float3x3) ObjectToWorld3x4())));

	float3 barycentrics = float3(
		1.f - Attributes.barycentrics.x - Attributes.barycentrics.y,
		Attributes.barycentrics.x,
		Attributes.barycentrics.y);
	
	
	Vertex vertex{};
	vertex.Position = p0;
	vertex.Normal = normalize(mul(vertex.Normal, transpose((float3x3) ObjectToWorld3x4())));

	VertexAttributes vertexAttributes;
	vertexAttributes.Position = WorldRayOrigin() + (WorldRayDirection() * RayTCurrent());
	vertexAttributes.Ng = n;
	vertexAttributes.Ns = vertex.Normal;
	vertexAttributes.TexCoords = vertex.TexCoords;

	return vertexAttributes;
}
