#ifndef AMPLIFICATION_HLSL
#define AMPLIFICATION_HLSL

#define GROUP_SIZE 32

[NumThreads(GROUP_SIZE, 1, 1)]
void ASMain(in uint GroupIndex	: SV_GroupIndex,
			in uint GroupID		: SV_GroupID)
{

}

#endif // AMPLIFICATION_HLSL
