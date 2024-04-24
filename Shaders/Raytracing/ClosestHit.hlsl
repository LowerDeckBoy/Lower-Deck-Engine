#ifndef CLOSESTHIT_HLSL
#define CLOSESTHIT_HLSL

#include "Shared.hlsli"

//uint3 GetIndices(uint triangleIndex)
//{
//	uint baseIndex = (triangleIndex * 3);
//	int address = (baseIndex * 4);
//	return Indices.Load3(address);
//}
//
//Vertex GetVertex(uint TriangleIndices, float3 barycentrics)
//{
//	uint3 indices = GetIndices(TriangleIndices);
//    
//	Vertex vertex = (Vertex) 0;
//	vertex.Position		= float3(0.0f, 0.0f, 0.0f);
//	vertex.TexCoords	= float2(0.0f, 0.0f);
//	vertex.Normal		= float3(0.0f, 0.0f, 0.0f);
//	vertex.Tangent		= float3(0.0f, 0.0f, 0.0f);
//	vertex.Bitangent	= float3(0.0f, 0.0f, 0.0f);
//    
//	for (uint i = 0; i < 3; i++)
//	{
//		vertex.Position		+= Vertices[indices[i]].Position * barycentrics[i];
//		vertex.TexCoords	+= Vertices[indices[i]].TexCoord * barycentrics[i];
//		vertex.Normal		+= Vertices[indices[i]].Normal * barycentrics[i];
//		vertex.Tangent		+= Vertices[indices[i]].Tangent * barycentrics[i];
//		vertex.Bitangent	+= Vertices[indices[i]].Bitangent * barycentrics[i];
//	}
//    
//	return vertex;
//}

[shader("closesthit")]
void ClosestHit(inout HitPayload Payload, BuiltInTriangleIntersectionAttributes Attributes)
{
	float3 barycentrics = float3(1.f - Attributes.barycentrics.x - Attributes.barycentrics.y, Attributes.barycentrics.x, Attributes.barycentrics.y);
	
	uint id = InstanceID();
	
	//Texture2D<float4> baseColor = ResourceDescriptorHeap[id];
	//float4 samp = baseColor.Sample(textureSampler, float2(vertId, vertId));
	uint vertId = 3 * PrimitiveIndex();
	//float3 hitColor = float3(0.5f, 0.5f, 0.5f);
	float3 hitColor = float3(id, 0.5f, 0.5f);

	Payload.ColorAndT = float4(hitColor, RayTCurrent());
}

#endif // CLOSESTHIT_HLSL
