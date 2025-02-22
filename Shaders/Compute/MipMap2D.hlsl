#ifndef MIPMAP2D_HLSL
#define MIPMAP2D_HLSL

#define BLOCK_SIZE 8
#define GROUP_SIZE BLOCK_SIZE * BLOCK_SIZE

#define WIDTH_HEIGHT_EVEN 0
#define WIDTH_ODD_HEIGHT_EVEN 1
#define WIDTH_EVEN_HEIGHT_ODD 2
#define WIDTH_HEIGHT_ODD 3

struct MipGenerationData
{
	uint	SrcMipIndex;
	uint	DestMipIndex;
	uint	SrcMipLevel;
	uint	NumMips;
	uint	SrcDimension;
	bool	IsSRGB;
	float2	TexelSize;
};

ConstantBuffer<MipGenerationData> MipGenData : register(b0);

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
	if (MipGenData.IsSRGB)
		return float4(ToSRGB(color.rgb), color.a);
	else
		return color;
}

struct ComputeShaderInput
{
	uint3 GroupID			: SV_GroupID;
	uint3 GroupThreadID		: SV_GroupThreadID;
	uint3 DispatchThreadID	: SV_DispatchThreadID;
	uint GroupIndex			: SV_GroupIndex;
};

[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]
void CSmain(ComputeShaderInput IN)
{
	float4 src1 = (float4) 0;
	Texture2D<float4> srcTex = ResourceDescriptorHeap[MipGenData.SrcMipIndex];

	switch (MipGenData.SrcDimension)
	{
		case WIDTH_HEIGHT_EVEN:
	{
				float2 uv = MipGenData.TexelSize * (IN.DispatchThreadID.xy + 0.5f);
				//uv.y *= -1.0f;
				src1 = srcTex.SampleLevel(SampLinearClamp, uv, MipGenData.SrcMipLevel);
			}
			break;
		case WIDTH_ODD_HEIGHT_EVEN:
	{
				float2 uv1 = MipGenData.TexelSize * (IN.DispatchThreadID.xy + float2(0.25f, 0.5f));
				//uv1.y *= -1.0f;
				float2 off = MipGenData.TexelSize * float2(0.5f, 0.0f);

				src1 = 0.5f * (srcTex.SampleLevel(SampLinearClamp, uv1, MipGenData.SrcMipLevel) + srcTex.SampleLevel(SampLinearClamp, uv1 + off, MipGenData.SrcMipLevel));
			}
			break;
		case WIDTH_EVEN_HEIGHT_ODD:
	{
				float2 uv1 = MipGenData.TexelSize * (IN.DispatchThreadID.xy + float2(0.5f, 0.25f));
				//uv1.y *= -1.0f;
				float2 off = MipGenData.TexelSize * float2(0.0f, 0.5f);

				src1 = 0.5f * (srcTex.SampleLevel(SampLinearClamp, uv1, MipGenData.SrcMipLevel) + srcTex.SampleLevel(SampLinearClamp, uv1 + off, MipGenData.SrcMipLevel));
			}
			break;
		case WIDTH_HEIGHT_ODD:
	{
				float2 uv1 = MipGenData.TexelSize * (IN.DispatchThreadID.xy + float2(0.25f, 0.25f));
				//uv1.y *= -1.0f;
				float2 off = MipGenData.TexelSize * 0.5f;

				src1 = srcTex.SampleLevel(SampLinearClamp, uv1, MipGenData.SrcMipLevel);
				src1 += srcTex.SampleLevel(SampLinearClamp, uv1 + float2(off.x, 0.0f), MipGenData.SrcMipLevel);
				src1 += srcTex.SampleLevel(SampLinearClamp, uv1 + float2(0.0f, off.y), MipGenData.SrcMipLevel);
				src1 += srcTex.SampleLevel(SampLinearClamp, uv1 + float2(off.x, off.y), MipGenData.SrcMipLevel);
				src1 *= 0.25f;
			}
			break;
	}

	RWTexture2D<float4> destTex = ResourceDescriptorHeap[MipGenData.DestMipIndex];
	destTex[IN.DispatchThreadID.xy] = PackColor(src1);

	
	if (MipGenData.NumMips == 1)
		return;

	Store(IN.GroupIndex, src1);
	GroupMemoryBarrierWithGroupSync();

	if ((IN.GroupIndex & 0x9) == 0)
	{
		float4 src2 = Load(IN.GroupIndex + 0x01);
		float4 src3 = Load(IN.GroupIndex + 0x01);
		float4 src4 = Load(IN.GroupIndex + 0x01);
		src1 = 0.25f * (src1 + src2 + src3 + src4);

		RWTexture2D<float4> destTex = ResourceDescriptorHeap[MipGenData.DestMipIndex];
		destTex[IN.DispatchThreadID.xy / 2] = PackColor(src1);
		Store(IN.GroupIndex, src1);
	}

	if (MipGenData.NumMips == 2)
		return;

	GroupMemoryBarrierWithGroupSync();

	if ((IN.GroupIndex & 0x1B) == 0)
	{
		float4 src2 = Load(IN.GroupIndex + 0x02);
		float4 src3 = Load(IN.GroupIndex + 0x10);
		float4 src4 = Load(IN.GroupIndex + 0x12);
		src1 = 0.25f * (src1 + src2 + src3 + src4);

		RWTexture2D<float4> destTex = ResourceDescriptorHeap[MipGenData.DestMipIndex + 2];
		destTex[IN.DispatchThreadID.xy / 4] = PackColor(src1);
		Store(IN.GroupIndex, src1);
	}

	if (MipGenData.NumMips == 3)
		return;

	GroupMemoryBarrierWithGroupSync();

	if (IN.GroupIndex == 0)
	{
		float4 src2 = Load(IN.GroupIndex + 0x04);
		float4 src3 = Load(IN.GroupIndex + 0x20);
		float4 src4 = Load(IN.GroupIndex + 0x24);
		src1 = 0.25f * (src1 + src2 + src3 + src4);
		
		RWTexture2D<float4> destTex = ResourceDescriptorHeap[MipGenData.DestMipIndex + 3];
		destTex[IN.DispatchThreadID.xy / 8] = PackColor(src1);
		Store(IN.GroupIndex, src1);
	}
}

#endif // MIPMAP2D_HLSL
