#ifndef ROOTSIGNATURES_HLSLI
#define ROOTSIGNATURES_HLSLI

/*
	Pre-defines Root Signatures for easier usage;
	no need to define them in C++ code.
*/

// https://learn.microsoft.com/en-us/windows/win32/direct3d12/specifying-root-signatures-in-hlsl


/*
	b0 - Per Object Matrices
	b1 - Vertex Buffer; vertices count and offset; 2 * 4bytes
	b2 - Material indices; 16 * 4bytes
	Flagged for Bindless design
*/
#define RS_GBUFFER "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | SAMPLER_HEAP_DIRECTLY_INDEXED),"\
					"CBV(b0, space=0),"\
					"CBV(b1, space=0),"\
					"CBV(b2, space=0),"\
					"StaticSampler(s0, " \
                             "addressU = TEXTURE_ADDRESS_WRAP, " \
                             "filter = FILTER_MIN_MAG_MIP_LINEAR )"

#endif // ROOTSIGNATURES_HLSLI
