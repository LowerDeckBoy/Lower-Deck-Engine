#ifndef MIPMAP3D_HLSL
#define MIPMAP3D_HLSL

#define BLOCK_SIZE 8
#define GROUP_SIZE BLOCK_SIZE * BLOCK_SIZE

#define WIDTH_HEIGHT_EVEN 0
#define WIDTH_ODD_HEIGHT_EVEN 1
#define WIDTH_EVEN_HEIGHT_ODD 2
#define WIDTH_HEIGHT_ODD 3

struct MipData
{
	uint InputIndex;	// SRV 
	uint OutputIndex;	// UAV
	uint MipBase;		// Resource to mip from
	uint MipCount;
	uint IsSRGB;		// defaults to false for now
	float3 TexelSize;
};

ConstantBuffer<MipData> MipGenData : register(b0);

SamplerState SampLinearClamp : register(s0);

groupshared float group_R[GROUP_SIZE];
groupshared float group_G[GROUP_SIZE];
groupshared float group_B[GROUP_SIZE];
groupshared float group_A[GROUP_SIZE];

void Store(uint index, float4 color)
{
	group_R[index] = color.r;
	group_G[index] = color.g;
	group_B[index] = color.b;
	group_A[index] = color.a;
}

float4 Load(uint index)
{
	return float4(group_R[index], group_G[index], group_B[index], group_A[index]);
}

float3 ToLinear(float3 srgb)
{
	if (any(srgb < float3(0.04045f, 0.04045f, 0.04045f)))
		return srgb / float3(12.92f, 12.92f, 12.92f);
	
	return pow((srgb + float3(0.055f, 0.055f, 0.055f)) / 1.055f, 2.4f);
}

float3 ToSRGB(float3 lin)
{
	if (any(lin < 0.0031308))
		return 12.92 * lin;

	return (1.055 * pow(abs(lin), 1.0f / 2.4f) - 0.055f);
}

float4 PackColor(float4 color)
{
	//if (MipGenData.IsSRGB)
	//	return float4(ToSRGB(color.rgb), color.a);
	//else
		return color;
}

struct ComputeShaderInput
{
	uint3 GroupID : SV_GroupID;
	uint3 GroupThreadID : SV_GroupThreadID;
	uint3 DispatchThreadID : SV_DispatchThreadID;
	uint GroupIndex : SV_GroupIndex;
};

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void CSmain(ComputeShaderInput IN)
{
	//Texture2DArray<float4> inputTexture = ResourceDescriptorHeap[MipGenData.inputTextureIndex];
	//RWTexture2DArray<float4> outputTexture = ResourceDescriptorHeap[MipGenData.outputTextureIndex];
	//int4 sampleLocation = int4(2 * IN.DispatchThreadID.x, 2 * IN.DispatchThreadID.y, IN.DispatchThreadID.z, 0);
	//
	//float4 gatherValue =
	//	inputTexture.Load(sampleLocation, int2(0, 0)) +
	//	inputTexture.Load(sampleLocation, int2(1, 0)) +
	//	inputTexture.Load(sampleLocation, int2(0, 1)) +
	//	inputTexture.Load(sampleLocation, int2(1, 1));
	//outputTexture[IN.DispatchThreadID] = 0.25 * gatherValue;
	
	Texture2DArray<float4> inputTexture = ResourceDescriptorHeap[MipGenData.InputIndex];
	float3 uvw = float3(MipGenData.TexelSize * (IN.DispatchThreadID.xyz + 0.5f.xxx));
	
	float4 sourceSample = inputTexture.SampleLevel(SampLinearClamp, uvw, MipGenData.MipBase);
	
	RWTexture2DArray<float4> outputTexture = ResourceDescriptorHeap[MipGenData.OutputIndex];
	outputTexture[IN.DispatchThreadID] = sourceSample;
}

#endif // MIPMAP3D_HLSL
