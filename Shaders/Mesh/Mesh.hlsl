#ifndef MESH_HLSL
#define MESH_HLSL

#define MAX_MESHLET_SIZE 128

[NumThreads(128, 1, 1)]
[OutputTopology("triangles")]
void MSMain()
{

}

#endif // MESH_HLSL
