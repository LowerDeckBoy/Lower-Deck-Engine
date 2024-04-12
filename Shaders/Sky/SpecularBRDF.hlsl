#ifndef SPECULAR_BRDF_CS
#define SPECULAR_BRDF_CS

// ==================================================
// Shaders/Sky/SpecularBRDF.hlsl
// Generates BRDF Look Up Table for PBR workflow.
// ==================================================

#include "SkyCommon.hlsli"

// Pre-integrates Cook-Torrance specular BRDF for varying roughness and viewing directions.
// Results are saved into 2D LUT texture in the form of DFG1 and DFG2 split-sum approximation terms,
// which act as a scale and bias to F0 (Fresnel reflectance at normal incidence) during rendering.

cbuffer TextureInput : register(b1, space0)
{
	uint InputIndex;
}

static const uint NumSamples = 1024;
static const float InvNumSamples = 1.0f / float(NumSamples);

// Sample i-th point from Hammersley point set of NumSamples points total.
float2 SampleHammersley(uint i)
{
	return float2(i * InvNumSamples, RadicalInverse_VdC(i));
}

[numthreads(DISPATCH_X, DISPATCH_Y, DISPATCH_Z)]
void CSmain(uint2 ThreadID : SV_DispatchThreadID)
{
    // Output dimensions
	float width = 0.0f;
	float height = 0.0f;
	RWTexture2D<float2> LUT = ResourceDescriptorHeap[InputIndex];
	LUT.GetDimensions(width, height);

	// Get integration parameters.
	float cosLo = ThreadID.x / width;
	float roughness = ThreadID.y / height;

	// Make sure viewing angle is non-zero to avoid divisions by zero (and subsequently NaNs).
	cosLo = max(cosLo, Epsilon);

	// Derive tangent-space viewing vector from angle to normal (pointing towards +Z in this reference frame).
	float3 Lo = float3(sqrt(1.0f - cosLo * cosLo), 0.0f, cosLo);

	// We will now pre-integrate Cook-Torrance BRDF for a solid white environment and save results into a 2D LUT.
	// DFG1 & DFG2 are terms of split-sum approximation of the reflectance integral.
	// For derivation see: "Moving Frostbite to Physically Based Rendering 3.0", SIGGRAPH 2014, section 4.9.2.
	float DFG1 = 0.0f;
	float DFG2 = 0.0f;

	for (uint i = 0; i < NumSamples; ++i)
	{
		float2 u = SampleHammersley(i);

		// Sample directly in tangent/shading space since we don't care about reference frame as long as it's consistent.
		float3 Lh = SampleGGX(u.x, u.y, roughness);

		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh).
		float3 Li = 2.0f * dot(Lo, Lh) * Lh - Lo;

		float cosLi = Li.z;
		float cosLh = Lh.z;
		float cosLoLh = max(dot(Lo, Lh), 0.0);

		if (cosLi > 0.0)
		{
			float G = gaSchlickGGX_IBL(cosLi, cosLo, roughness);
			float Gv = G * cosLoLh / (cosLh * cosLo);
			float Fc = pow(1.0f - cosLoLh, 5.0f);

			DFG1 += (1.0f - Fc) * Gv;
			DFG2 += Fc * Gv;
		}
	}
	
	LUT[ThreadID] = float2(DFG1, DFG2) * InvNumSamples;
	//LUT[float2(ThreadID.x, 255 - ThreadID.y)] = float2(DFG1, DFG2) * InvNumSamples;
}

#endif // SPECULAR_BRDF_CS
