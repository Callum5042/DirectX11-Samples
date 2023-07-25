// Vertex input
struct VertexInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

// Vertex output / pixel input structure
struct PixelInput
{
    float4 positionClipSpace : SV_POSITION;
    float3 position : POSITION;
};