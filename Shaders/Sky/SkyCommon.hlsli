#ifndef SKY_COMMON_HLSLI
#define SKY_COMMON_HLSLI

// ==========================================================================
// Shaders/Sky/SkyCommon.hlsli
// Commonly shared constants and functions used across sky related shaders.
// ==========================================================================

#define DISPATCH_X 8
#define DISPATCH_Y 8
#define DISPATCH_Z 1

static const float PI = 3.141592f;
static const float TwoPI = 2.0f * PI;
static const float Epsilon = 0.001f;

//static const uint NumSamples = 64 * 1024;
//static const float InvNumSamples = 1.0f / float(NumSamples);

// Compute Van der Corput radical inverse
// Reference: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10f; // / 0x100000000
}

// Uniformly sample point on a hemisphere.
// Cosine-weighted sampling would be a better fit for Lambertian BRDF but since this
// compute shader runs only once as a pre-processing step performance is not *that* important.
// See: "Physically Based Rendering" 2nd ed., section 13.6.1.
float3 SampleHemisphere(float u1, float u2)
{
	const float u1p = sqrt(max(0.0f, 1.0f - u1 * u1));
	return float3(cos(TwoPI * u2) * u1p, sin(TwoPI * u2) * u1p, u1);
}

// Compute orthonormal basis for converting from tanget/shading space to world space.
void ComputeBasisVectors(const float3 N, out float3 S, out float3 T)
{
	// Branchless select non-degenerate T.
	T = cross(N, float3(0.0, 1.0, 0.0));
	T = lerp(cross(N, float3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));

	T = normalize(T);
	S = normalize(cross(N, T));
}

// Convert point from tangent/shading space to world space.
float3 TangentToWorld(const float3 V, const float3 N, const float3 S, const float3 T)
{
	return S * V.x + T * V.y + N * V.z;
}

// Calculate normalized sampling direction vector based on current fragment coordinates.
// This is essentially "inverse-sampling": we reconstruct what the sampling vector would be if we wanted it to "hit"
// this particular fragment in a cubemap.
float3 GetSamplingVector(uint3 ThreadID, in RWTexture2DArray<float4> OutputTexture)
{
	float width  = 0.0f;
	float height = 0.0f;
	float depth  = 0.0f;
	OutputTexture.GetDimensions(width, height, depth);

	float2 st = ThreadID.xy / float2(width, height);
	float2 uv = 2.0f * float2(st.x, 1.0f - st.y) - 1.0f;

	// Select vector based on cubemap face index.
	float3 result = float3(0.0f, 0.0f, 0.0f);
	switch (ThreadID.z)
	{
		case 0:
			result = float3(1.0f, uv.y, -uv.x);
			break;
		case 1:
			result = float3(-1.0f, uv.y, uv.x);
			break;
		case 2:
			result = float3(uv.x, 1.0f, -uv.y);
			break;
		case 3:
			result = float3(uv.x, -1.0f, uv.y);
			break;
		case 4:
			result = float3(uv.x, uv.y, 1.0f);
			break;
		case 5:
			result = float3(-uv.x, uv.y, -1.0f);
			break;
	}
	
	return normalize(result);
}

// Importance sample GGX normal distribution function for a fixed roughness value.
// This returns normalized half-vector between Li & Lo.
// For derivation see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
float3 SampleGGX(float u1, float u2, float Roughness)
{
	float alpha = Roughness * Roughness;

	float cosTheta = sqrt((1.0f - u2) / (1.0f + (alpha * alpha - 1.0f) * u2));
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta); // Trig. identity
	float phi = TwoPI * u1;

	// Convert to Cartesian upon return.
	return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0f - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method (IBL version).
float gaSchlickGGX_IBL(float cosLi, float cosLo, float Roughness)
{
	float k = (Roughness * Roughness) / 2.0f; // Epic suggests using this roughness remapping for IBL lighting.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

#endif // SKY_COMMON_HLSLI
