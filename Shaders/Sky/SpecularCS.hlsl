#ifndef SPECULAR_CS_HLSL
#define SPECULAR_CS_HLSL

#define DIPATCH_X 32
#define DIPATCH_Y 32
#define DIPATCH_Z 1

cbuffer InputTexture : register(b0, space0)
{
	uint InputIndex;
};

cbuffer OutputTexture : register(b1, space0)
{
	uint OutputIndex;
};

cbuffer SpecularMapFilterSettings : register(b2, space0)
{
	// Roughness value to pre-filter for.
	float roughness;
};

static const float PI = 3.141592f;
static const float TwoPI = 2.0f * PI;
static const float Epsilon = 0.001f;

static const uint NumSamples = 1024;
static const float InvNumSamples = 1.0f / float(NumSamples);

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

// Sample i-th point from Hammersley point set of NumSamples points total.
float2 SampleHammersley(uint i)
{
	return float2(i * InvNumSamples, RadicalInverse_VdC(i));
}

// Importance sample GGX normal distribution function for a fixed roughness value.
// This returns normalized half-vector between Li & Lo.
// For derivation see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
float3 SampleGGX(float u1, float u2, float Roughness)
{
	float alpha = Roughness * Roughness;

	float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha * alpha - 1.0) * u2));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta); // Trig. identity
	float phi = TwoPI * u1;

	// Convert to Cartesian upon return.
	return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method (IBL version).
float gaSchlickGGX_IBL(float cosLi, float cosLo, float Roughness)
{
	float k = (Roughness * Roughness) / 2.0; // Epic suggests using this roughness remapping for IBL lighting.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Calculate normalized sampling direction vector based on current fragment coordinates.
// This is essentially "inverse-sampling": we reconstruct what the sampling vector would be if we wanted it to "hit"
// this particular fragment in a cubemap.
float3 GetSamplingVector(uint3 ThreadID, in RWTexture2DArray<float4> OutputTexture)
{
	float width = 0.0f;
	float height = 0.0f;
	float depth = 0.0f;
	OutputTexture.GetDimensions(width, height, depth);

	float2 st = ThreadID.xy / float2(width, height);
	float2 uv = 2.0 * float2(st.x, 1.0 - st.y) - 1.0;

	// Select vector based on cubemap face index.
	float3 result = float3(0.0f, 0.0f, 0.0f);
	switch (ThreadID.z)
	{
		case 0:
			result = float3(1.0, uv.y, -uv.x);
			break;
		case 1:
			result = float3(-1.0, uv.y, uv.x);
			break;
		case 2:
			result = float3(uv.x, 1.0, -uv.y);
			break;
		case 3:
			result = float3(uv.x, -1.0, uv.y);
			break;
		case 4:
			result = float3(uv.x, uv.y, 1.0);
			break;
		case 5:
			result = float3(-uv.x, uv.y, -1.0);
			break;
	}
	return normalize(result);
}

// Uniformly sample point on a hemisphere.
// Cosine-weighted sampling would be a better fit for Lambertian BRDF but since this
// compute shader runs only once as a pre-processing step performance is not *that* important.
// See: "Physically Based Rendering" 2nd ed., section 13.6.1.
float3 SampleHemisphere(float u1, float u2)
{
	const float u1p = sqrt(max(0.0, 1.0 - u1 * u1));
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

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float Roughness)
{
	float alpha = Roughness * Roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

SamplerState defaultSampler : register(s0);

[numthreads(DIPATCH_X, DIPATCH_Y, DIPATCH_Z)]
void CSmain(uint3 ThreadID : SV_DispatchThreadID)
{
	TextureCube<float4> inputTexture = ResourceDescriptorHeap[InputIndex];
	RWTexture2DArray<float4> outputTexture = ResourceDescriptorHeap[OutputIndex];
	
	// Make sure we won't write past output when computing higher mipmap levels.
	uint outputWidth = 0;
	uint outputHeight = 0;
	uint outputDepth = 0;
	
	outputTexture.GetDimensions(outputWidth, outputHeight, outputDepth);
	if (ThreadID.x >= outputWidth || ThreadID.y >= outputHeight)
		return;
 
	// Get input cubemap dimensions at zero mipmap level.
	float inputWidth, inputHeight, inputLevels;
	inputTexture.GetDimensions(0, inputWidth, inputHeight, inputLevels);

	// Solid angle associated with a single cubemap texel at zero mipmap level.
	// This will come in handy for importance sampling below.
	float wt = 4.0f * PI / (6.0f * inputWidth * inputHeight);
	
	// Approximation: Assume zero viewing angle (isotropic reflections).
	float3 N = GetSamplingVector(ThreadID, outputTexture);
	float3 Lo = N;
	
	float3 S = float3(0.0f, 0.0f, 0.0f);
	float3 T = float3(0.0f, 0.0f, 0.0f);
	ComputeBasisVectors(N, S, T);

	float3 color = float3(0.0f, 0.0f, 0.0f);
	float weight = 0.0f;

	// Convolve environment map using GGX NDF importance sampling.
	// Weight by cosine term since Epic claims it generally improves quality.
	for (uint i = 0; i < NumSamples; ++i)
	{
		float2 u = SampleHammersley(i);
		float3 Lh = TangentToWorld(SampleGGX(u.x, u.y, roughness), N, S, T);

		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh).
		float3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

		float cosLi = dot(N, Li);
		if (cosLi > 0.0f)
		{
			// Use Mipmap Filtered Importance Sampling to improve convergence.
			// See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html, section 20.4

			float cosLh = max(dot(N, Lh), 0.0f);

			// GGX normal distribution function (D term) probability density function.
			// Scaling by 1/4 is due to change of density in terms of Lh to Li (and since N=V, rest of the scaling factor cancels out).
			float pdf = ndfGGX(cosLh, roughness) * 0.25f;

			// Solid angle associated with this sample.
			float ws = 1.0f / (NumSamples * pdf);

			// Mip level to sample from.
			float mipLevel = max(0.5f * log2(ws / wt) + 1.0f, 0.0f);

			color += inputTexture.SampleLevel(defaultSampler, Li, mipLevel).rgb * cosLi;
			weight += cosLi;
		}
	}
	color /= weight;

	outputTexture[ThreadID] = float4(color, 1.0f);
}

#endif // SPECULAR_CS_HLSL
