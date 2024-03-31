#ifndef MATERIAL_HLSL
#define MATERIAL_HLSL

#define INVALID_INDEX -1

struct Material
{
	int BaseColorIndex;
	int NormalIndex;
	int MetalRoughnessIndex;
	int EmissiveIndex;
	
	float MetallicFactor;
	float RoughnessFactor;
	float AlphaCutoff;
	bool  bDoubleSided;
	
	float4 BaseColorFactor;
	float4 EmissiveFactor;
};

#endif // MATERIAL_HLSL
