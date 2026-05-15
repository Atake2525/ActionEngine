// SkinningObject3d.VS.hlslで作ったものと同じPalette
struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInverseTransposeMatrix;
};
StructuredBuffer<Well> gMatrixPalette : register(t0);

// VertexBufferViewのstream0として利用していた入力頂点
struct Vertex
{
    float4 position;
    float2 texcoord;
    float3 normal;
    float3 tangent;
    float3 binormal;
};
StructuredBuffer<Vertex> gInputVertices : register(t1);

// VertexBufferViewのstream1として利用していた入力インフルエンス
struct VertexInfluence
{
    float4 weight;
    int4 index;
};
StructuredBuffer<VertexInfluence> gInfluence : register(t2);

// Skinning計算後の頂点データ。SkinningVertex
RWStructuredBuffer<Vertex> gOutputVertices : register(u0);
// Skinningに関するちょっとした情報
struct SkinningInformation
{
    uint numVertices;
};
ConstantBuffer<SkinningInformation> gSkinningInformation : register(b0);

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint vertexIndex = DTid.x;
    if (vertexIndex < gSkinningInformation.numVertices)
    {
        // 必要な頂点データをStructuredBufferから取ってくる
        // SkinningObject3d.VSででゃ入力頂点として受け取っていた
        Vertex input = gInputVertices[vertexIndex];
        VertexInfluence influence = gInfluence[vertexIndex];
        
        // skinning後の頂点を計算
        Vertex skinned = input; 
        skinned.texcoord = input.texcoord;
        
        // 計算の方法はSkinningObject3d.VSと同じ
        // 位置の変換
        skinned.position = mul(input.position, gMatrixPalette[influence.index.x].skeletonSpaceMatrix) * influence.weight.x;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.y].skeletonSpaceMatrix) * influence.weight.y;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.z].skeletonSpaceMatrix) * influence.weight.z;
        skinned.position += mul(input.position, gMatrixPalette[influence.index.w].skeletonSpaceMatrix) * influence.weight.w;
        skinned.position.w = 1.0f; // 確実に1を入れる
    
        // 法線の変換
        skinned.normal = mul(input.normal, (float3x3) gMatrixPalette[influence.index.x].skeletonSpaceInverseTransposeMatrix) * influence.weight.x;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.y].skeletonSpaceInverseTransposeMatrix) * influence.weight.y;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.z].skeletonSpaceInverseTransposeMatrix) * influence.weight.z;
        skinned.normal += mul(input.normal, (float3x3) gMatrixPalette[influence.index.w].skeletonSpaceInverseTransposeMatrix) * influence.weight.w;
        skinned.normal = normalize(skinned.normal); // 正規化して戻してあげる

        skinned.tangent = mul(input.tangent, (float3x3) gMatrixPalette[influence.index.x].skeletonSpaceInverseTransposeMatrix) * influence.weight.x;
        skinned.tangent += mul(input.tangent, (float3x3) gMatrixPalette[influence.index.y].skeletonSpaceInverseTransposeMatrix) * influence.weight.y;
        skinned.tangent += mul(input.tangent, (float3x3) gMatrixPalette[influence.index.z].skeletonSpaceInverseTransposeMatrix) * influence.weight.z;
        skinned.tangent += mul(input.tangent, (float3x3) gMatrixPalette[influence.index.w].skeletonSpaceInverseTransposeMatrix) * influence.weight.w;
        skinned.tangent = normalize(skinned.tangent);

        skinned.binormal = mul(input.binormal, (float3x3) gMatrixPalette[influence.index.x].skeletonSpaceInverseTransposeMatrix) * influence.weight.x;
        skinned.binormal += mul(input.binormal, (float3x3) gMatrixPalette[influence.index.y].skeletonSpaceInverseTransposeMatrix) * influence.weight.y;
        skinned.binormal += mul(input.binormal, (float3x3) gMatrixPalette[influence.index.z].skeletonSpaceInverseTransposeMatrix) * influence.weight.z;
        skinned.binormal += mul(input.binormal, (float3x3) gMatrixPalette[influence.index.w].skeletonSpaceInverseTransposeMatrix) * influence.weight.w;
        skinned.binormal = normalize(skinned.binormal);
    
        // Skinning後のちょうてんでーたを格納、つまり書き込む
        gOutputVertices[vertexIndex] = skinned;
    }
}
