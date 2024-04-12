#ifndef IRRADIANCE_CS_HLSL
#define IRRADIANCE_CS_HLSL

// ==================================================
// Shaders/Sky/IrradianceCS.hlsl
//
// ==================================================

#include "SkyCommon.hlsli"

cbuffer InputTexture	: register(b0, space0) { uint InputIndex;  }
cbuffer OutputTexture	: register(b1, space0) { uint OutputIndex; }

static const uint NumSamples = 32 * 1024; // * 64
static const float InvNumSamples = 1.0f / float(NumSamples);

// Sample i-th point from Hammersley point set of NumSamples points total.
float2 SampleHammersley(uint i)
{
	return float2(i * InvNumSamples, RadicalInverse_VdC(i));
}

SamplerState texSampler : register(s0);

[numthreads(DISPATCH_X, DISPATCH_Y, DISPATCH_Z)]
void CSmain(uint3 ThreadID : SV_DispatchThreadID)
{
	TextureCube<float4> inputTexture = ResourceDescriptorHeap[InputIndex];
	RWTexture2DArray<float4> outputTexture = ResourceDescriptorHeap[OutputIndex];
	
	float3 N = GetSamplingVector(ThreadID, outputTexture);
	
	float3 S, T;
	ComputeBasisVectors(N, S, T);

	// Monte Carlo integration of hemispherical irradiance.
	// As a small optimization this also includes Lambertian BRDF assuming perfectly white surface (albedo of 1.0)
	// so we don't need to normalize in PBR fragment shader (so technically it encodes exitant radiance rather than irradiance).
	float3 irradiance = 0.0f;
	for (uint i = 0; i < NumSamples; ++i)
	{
		float2 u = SampleHammersley(i);
		float3 Li = TangentToWorld(SampleHemisphere(u.x, u.y), N, S, T);
		float cosTheta = max(0.0, dot(Li, N));

		// PIs here cancel out because of division by pdf.
		irradiance += 2.0 * inputTexture.SampleLevel(texSampler, Li, 0).rgb * cosTheta;
	}
	irradiance /= float(NumSamples);
	
	outputTexture[ThreadID] = float4(irradiance, 1.0);
}

#endif // IRRADIANCE_CS_HLSL
