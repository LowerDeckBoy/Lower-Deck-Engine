#ifndef CLUSTER_HLSL
#define CLUSTER_HLSL

// TODO:
// https://github.com/pezcode/Cluster/blob/master/src/Renderer/ClusterShader.h

#define CLUSTERS_X = 16;
#define CLUSTERS_Y = 8;
#define CLUSTERS_Z = 24;

#define DISPATCH_X = 16;
#define DISPATCH_Y = 8;
#define DISPATCH_Z = 4;

#define MAX_LIGHTS_PER_CLUSTER 32

#endif // CLUSTER_HLSL
