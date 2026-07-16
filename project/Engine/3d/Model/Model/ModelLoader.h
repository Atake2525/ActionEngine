#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Animator.h"

// 頂点情報を格納するための構造体
struct VertexData {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
    Vector3 tangent;
    Vector3 binormal;
};

// マルチメッシュ、マルチマテリアル用の各種情報を格納するための構造体
struct MaterialVertexData {
    std::vector<VertexData> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<std::string, JointWeightData> skinClusterData;
    size_t materialIndex;
};


// マテリアル
struct Material {
    Vector4 color;

    int32_t enableLighting;
    int pad0[3];

    Matrix4x4 uvTransform;

    float shininess;
    //float pad1[3];

    Vector3 specularColor;
    //float pad3;

    uint32_t enableMetallic;
    float environmentCoefficient;
    float pad2[2];
};

// マテリアル
struct MaterialTemplate
{
    float metallic;
    float roughness;
    float padding[2];
};

struct MaterialData {
    std::string textureFilePath;
    uint32_t textureIndex = 0;

    std::string normalMapFilePath;
    uint32_t normalMapIndex = 0;

    std::string metallicMapFilePath;
    uint32_t metallicMapIndex = 0;

    std::string roughnessMapFilePath;
    uint32_t roughnessMapIndex = 0;
};


struct ModelData {
    std::unordered_map<std::string, JointWeightData> skinClusterData;
    std::vector<uint32_t> indices;
    std::vector<VertexData> vertices;
    std::unordered_map<std::wstring, MaterialVertexData> matVertexData;
    std::vector<MaterialData> material;
    std::vector<MaterialTemplate> materialTemplate;
    Node rootNode;
};

class ModelLoader {
public:     // .mtlファイルの読み取り
    static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName);
    // ノード情報のロード
    static Node ReadNode(aiNode* node);
    // アニメーションの読み込み
    static Animation LoadAnimationFile(const std::string& directoryPath, const std::string& fileName);
    // .gltfファイルの読み取り
    static ModelData LoadModelFileGLTF(const std::string& directoryPath, const std::string& fileName);
    // .objファイルの読み取り
    static ModelData LoadModelFileOBJ(const std::string& directoryPath, const std::string& fileName);
};

