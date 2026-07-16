#include "ModelLoader.h"
#include "Logger.h"

using namespace Logger;

ModelData ModelLoader::LoadModelFileOBJ(const std::string& directoryPath, const std::string& fileName)
{
    ModelData modelData;            // 構築するModelData
    Assimp::Importer importer;
    std::string filePath = directoryPath + "/" + fileName;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_CalcTangentSpace);
    //assert(scene->HasMeshes()); 
    // メッシュが無いのは対応しない
    if (!scene->HasMeshes())
    {
        Log("ファイルの展開に失敗しました\n指定したファイルパスにファイルが存在するか、名前が一致しているか確認してください\n");
        modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
        return modelData;
    }

    //modelData.matVertexData.resize(scene->mNumMeshes + 1);
    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh* mesh = scene->mMeshes[meshIndex];

        // メッシュ名を取得(日本語に対応させるために変換)
        std::string utf8 = mesh->mName.C_Str();
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
        std::wstring meshName(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &meshName[0], len);
        modelData.matVertexData[meshName];

        if (!mesh->HasNormals() || !mesh->HasTextureCoords(0))
        {
            Log("指定したファイルには法線またはTexcoordsが存在しません\n上記が存在しないまたは破損したファイルを読み込んでいる可能性があります\n");
            modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
            return modelData;
        }

        modelData.vertices.resize(mesh->mNumVertices); // 最初に頂点数分のメモリを保管しておく
        modelData.matVertexData[meshName].vertices.resize(mesh->mNumVertices); // マルチマテリアルでもメモリ保管

        if (mesh->mMaterialIndex != 0)
        {
            modelData.matVertexData[meshName].materialIndex = mesh->mMaterialIndex - 1;
        }
        else
        {
            modelData.matVertexData[meshName].materialIndex = mesh->mMaterialIndex;
        }
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
        {
            aiVector3D& position = mesh->mVertices[vertexIndex];
            aiVector3D& normal = mesh->mNormals[vertexIndex];
            aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
            aiVector3D& tangent = mesh->mTangents[vertexIndex];
            aiVector3D& bitangent = mesh->mBitangents[vertexIndex];
            // 右手系->左手系への返還を忘れずに
            modelData.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
            modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
            modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
            modelData.vertices[vertexIndex].tangent = { -tangent.x, tangent.y, tangent.z };
            modelData.vertices[vertexIndex].binormal = { -bitangent.x, bitangent.y, bitangent.z };

            modelData.matVertexData[meshName].vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
            modelData.matVertexData[meshName].vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
            modelData.matVertexData[meshName].vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
            modelData.matVertexData[meshName].vertices[vertexIndex].tangent = { -tangent.x, tangent.y, tangent.z };
            modelData.matVertexData[meshName].vertices[vertexIndex].binormal = { -bitangent.x, bitangent.y, bitangent.z };
        }
        // Indexの解析
        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
        {
            aiFace& face = mesh->mFaces[faceIndex];
            if (face.mNumIndices != 3)
            {
                Log("指定したファイルに三角化されていない面が存在します\nフォルダをもう一度確認してください\n");
                modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
                return modelData;
            }

            for (uint32_t element = 0; element < face.mNumIndices; element++)
            {
                uint32_t vertexIndex = face.mIndices[element];
                modelData.indices.push_back(vertexIndex);
                modelData.matVertexData[meshName].indices.push_back(vertexIndex);
            }
        }

    }
    //uint32_t texIndex = m_pTextureManager->LoadTexture("Resources/Sprite/black1x1.png");
    // マテリアルが作成されていない場合はwhite1x1を使用
    if (scene->mNumMaterials == 1 || scene->mNumMaterials == 0)
    {
        MaterialData matData;
        matData.textureFilePath = "Resources/Sprite/white1x1.png";

        matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
        //matData.normalMapIndex = texIndex;
        matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
        //matData.metallicMapIndex = texIndex;
        matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
        //matData.roughnessMapIndex = texIndex;

        // 読み込んだテクスチャの番号尾を取得
        //matData.textureIndex = m_pTextureManager->LoadTexture(matData.textureFilePath);

        MaterialTemplate matTempData;
        // メタリックの数値		
        matTempData.metallic = 0.0f;
        matTempData.roughness = 1.0f;

        modelData.materialTemplate.push_back(matTempData);
        modelData.material.push_back(matData);
    }
    for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
    {
        if (materialIndex == 0)
        {
            continue;
        }

        aiMaterial* material = scene->mMaterials[materialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
        {
            aiString textureFilePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

            MaterialData matData;
            matData.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();

            // 読み込んだテクスチャの番号尾を取得
            //matData.textureIndex = m_pTextureManager->LoadTexture(matData.textureFilePath);

            // ノーマルマップ、メタリックマップ、ラフネスマップの読み込み
            if (material->GetTexture(aiTextureType_NORMALS, 0, &textureFilePath) == AI_SUCCESS ||
                material->GetTexture(aiTextureType_HEIGHT, 0, &textureFilePath) == AI_SUCCESS) {
                matData.normalMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                //matData.normalMapIndex = m_pTextureManager->LoadTexture(matData.normalMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
                //matData.normalMapIndex = texIndex;
            }
            if (material->GetTexture(aiTextureType_METALNESS, 0, &textureFilePath) == AI_SUCCESS) {
                matData.metallicMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                //matData.metallicMapIndex = m_pTextureManager->LoadTexture(matData.metallicMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
                //matData.metallicMapIndex = texIndex;
            }
            if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &textureFilePath) == AI_SUCCESS) {
                matData.roughnessMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                //matData.roughnessMapIndex = m_pTextureManager->LoadTexture(matData.roughnessMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
                //matData.roughnessMapIndex = texIndex;
            }

            // メタリックの数値
            float metallic = 0.0f;
            float roughness = 1.0f;
            MaterialTemplate matTempData;

            if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
                matTempData.metallic = metallic;
            }
            else
            {
                matTempData.metallic = 0.0f;
            }
            if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
                matTempData.roughness = roughness;
            }
            else
            {
                matTempData.roughness = 1.0f;
            }

            modelData.materialTemplate.push_back(matTempData);
            modelData.material.push_back(matData);
        }
        else // マテリアルが割り当てられていない場合はwhite1x1を割り当てる
        {
            MaterialData matData;
            matData.textureFilePath = "Resources/Sprite/white1x1.png";

            matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
            //matData.normalMapIndex = texIndex;
            matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
            //matData.metallicMapIndex = texIndex;
            matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
            //matData.roughnessMapIndex = texIndex;

            // 読み込んだテクスチャの番号尾を取得
            //matData.textureIndex = m_pTextureManager->LoadTexture(matData.textureFilePath);

            MaterialTemplate matTempData;
            // メタリックの数値		
            matTempData.metallic = 0.0f;
            matTempData.roughness = 1.0f;

            modelData.materialTemplate.push_back(matTempData);
            modelData.material.push_back(matData);
        }
    }



    for (auto it = modelData.matVertexData.begin(); it != modelData.matVertexData.end();)
    {
        if (it->second.vertices.empty() || it->second.indices.empty())
        {
            Log("Model loader skipped an empty mesh.\n");
            it = modelData.matVertexData.erase(it);
            continue;
        }
        ++it;
    }
    modelData.rootNode = ReadNode(scene->mRootNode);
    return modelData;
}

// マルチスレッド化予定
ModelData ModelLoader::LoadModelFileGLTF(const std::string& directoryPath, const std::string& fileName) {
    ModelData modelData;            // 構築するModelData
    Assimp::Importer importer;
    std::string filePath = directoryPath + "/" + fileName;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);
    if (!scene->HasMeshes())
    {
        Log("ファイルの展開に失敗しました\n指定したファイルパスにファイルが存在するか、名前が一致しているか確認してください\n");
        modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
        return modelData;
    }

    // マルチマテリアル対応のためにメモリを保管しておく

    for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
    {
        aiMesh* mesh = scene->mMeshes[meshIndex];

        // メッシュ名を取得(日本語に対応させるために変換)
        std::string utf8 = mesh->mName.C_Str();
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
        std::wstring meshName(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &meshName[0], len);
        modelData.matVertexData[meshName];

        if (!mesh->HasNormals() || !mesh->HasTextureCoords(0))
        {
            Log("指定したファイルには法線またはTexcoordsが存在しません\n上記が存在しないまたは破損したファイルを読み込んでいる可能性があります\n");
            modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
            return modelData;
        }

        modelData.vertices.resize(mesh->mNumVertices); // 最初に頂点数分のメモリを保管しておく
        modelData.matVertexData[meshName].vertices.resize(mesh->mNumVertices); // マルチマテリアルでもメモリ保管
        modelData.matVertexData[meshName].materialIndex = mesh->mMaterialIndex;
        for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
        {
            aiVector3D& position = mesh->mVertices[vertexIndex];
            aiVector3D& normal = mesh->mNormals[vertexIndex];
            aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];
            aiVector3D& tangent = mesh->mTangents[vertexIndex];
            aiVector3D& bitangent = mesh->mBitangents[vertexIndex];
            // 右手系->左手系への返還を忘れずに
            modelData.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
            modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
            modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
            modelData.vertices[vertexIndex].tangent = { -tangent.x, tangent.y, tangent.z };
            modelData.vertices[vertexIndex].binormal = { -bitangent.x, bitangent.y, bitangent.z };

            modelData.matVertexData[meshName].vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
            modelData.matVertexData[meshName].vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
            modelData.matVertexData[meshName].vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
            modelData.matVertexData[meshName].vertices[vertexIndex].tangent = { -tangent.x, tangent.y, tangent.z };
            modelData.matVertexData[meshName].vertices[vertexIndex].binormal = { -bitangent.x, bitangent.y, bitangent.z };
        }
        // Indexの解析
        for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
        {
            aiFace& face = mesh->mFaces[faceIndex];
            if (face.mNumIndices != 3)
            {
                Log("指定したファイルに三角化されていない面が存在します\nフォルダをもう一度確認してください\n");
                modelData = LoadModelFileOBJ("Resources/Debug/obj", "box.obj");
                return modelData;
            }

            for (uint32_t element = 0; element < face.mNumIndices; ++element)
            {
                uint32_t vertexIndex = face.mIndices[element];
                modelData.indices.push_back(vertexIndex);
                modelData.matVertexData[meshName].indices.push_back(vertexIndex);
            }
        }

        // SkinCluster構築用のデータ取得を追加
        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
        {
            // Jointごとの格納領域を作る
            aiBone* bone = mesh->mBones[boneIndex];
            std::string JointName = bone->mName.C_Str();
            JointWeightData jointWeightData;// = modelData.skinClusterData[JointName];

            // InverseBindPoseMatrixの抽出
            aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
            aiVector3D scale, translate;
            aiQuaternion rotate;
            bindPoseMatrixAssimp.Decompose(scale, rotate, translate);
            Matrix4x4 bindPoseMatrix = MakeAffineMatrix({ scale.x, scale.y, scale.z }, { rotate.x, -rotate.y, -rotate.z, rotate.w }, { -translate.x, translate.y, translate.z });
            jointWeightData.inverseBindPoseMatrix = Inverse(bindPoseMatrix);

            // Weight情報を取り出す
            for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++)
            {
                jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight , bone->mWeights[weightIndex].mVertexId });
            }
            modelData.skinClusterData[JointName] = jointWeightData;
            modelData.matVertexData[meshName].skinClusterData[JointName] = jointWeightData;
        }
    }
    //uint32_t texIndex = m_pTextureManager->LoadTexture("Resources/Sprite/black1x1.png");
    // テクスチャが無い場合white1x1を張るようにする
    if (scene->mNumMaterials == 0)
    {
        MaterialData matData;
        matData.textureFilePath = "Resources/Sprite/white1x1.png";

        matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
        //matData.normalMapIndex = texIndex;
        matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
        //matData.metallicMapIndex = texIndex;
        matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
        //matData.roughnessMapIndex = texIndex;

        // 読み込んだテクスチャの番号尾を取得
        //matData.textureIndex = m_pTextureManager->LoadTexture(matData.textureFilePath);

        MaterialTemplate matTempData;
        // メタリックの数値
        matTempData.metallic = 0.0f;
        matTempData.roughness = 1.0f;

        modelData.materialTemplate.push_back(matTempData);
        modelData.material.push_back(matData);
    }
    // マテリアルを設定されている数読み込む
    for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex)
    {
        aiMaterial* material = scene->mMaterials[materialIndex];
        if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
        {
            aiString textureFilePath;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);

            MaterialData matData;
            matData.textureFilePath = directoryPath + "/" + textureFilePath.C_Str();

            // テクスチャ読み込み
            //m_pTextureManager->LoadTexture(matData.textureFilePath);
            // 読み込んだテクスチャの番号尾を取得
            //matData.textureIndex = m_pTextureManager->LoadTexture(matData.textureFilePath);


            // ノーマルマップ、メタリックマップ、ラフネスマップの読み込み
            if (material->GetTexture(aiTextureType_NORMALS, 0, &textureFilePath) == AI_SUCCESS ||
                material->GetTexture(aiTextureType_HEIGHT, 0, &textureFilePath) == AI_SUCCESS) {
                matData.normalMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                //matData.normalMapIndex = m_pTextureManager->LoadTexture(matData.normalMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
                //matData.normalMapIndex = texIndex;
            }
            if (material->GetTexture(aiTextureType_METALNESS, 0, &textureFilePath) == AI_SUCCESS) {
                matData.metallicMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                //matData.metallicMapIndex = m_pTextureManager->LoadTexture(matData.metallicMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
                //matData.metallicMapIndex = texIndex;
            }
            if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &textureFilePath) == AI_SUCCESS) {
                matData.roughnessMapFilePath = directoryPath + "/" + textureFilePath.C_Str();
                //matData.roughnessMapIndex = m_pTextureManager->LoadTexture(matData.roughnessMapFilePath, TextureColorSpace::Linear);
            }
            else
            {
                matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
                //matData.roughnessMapIndex = texIndex;
            }


            // メタリックの数値
            float metallic = 0.0f;
            float roughness = 1.0f;
            MaterialTemplate matTempData;

            if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
                matTempData.metallic = metallic;
            }
            else
            {
                matTempData.metallic = 0.0f;
            }
            if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
                matTempData.roughness = roughness;
            }
            else
            {
                matTempData.roughness = 1.0f;
            }

            modelData.materialTemplate.push_back(matTempData);
            modelData.material.push_back(matData);


        }
        else // マテリアルが割り当てられていない場合はwhite1x1を割り当てる
        {
            MaterialData matData;
            matData.textureFilePath = "Resources/Sprite/white1x1.png";

            matData.normalMapFilePath = "Resources/Sprite/black1x1.png";
            //matData.normalMapIndex = texIndex;
            matData.metallicMapFilePath = "Resources/Sprite/black1x1.png";
            //matData.metallicMapIndex = texIndex;
            matData.roughnessMapFilePath = "Resources/Sprite/black1x1.png";
            //matData.roughnessMapIndex = texIndex;

            // 読み込んだテクスチャの番号尾を取得
            //matData.textureIndex = m_pTextureManager->LoadTexture(matData.textureFilePath);

            // メタリックの数値
            float metallic = 0.0f;
            float roughness = 1.0f;
            MaterialTemplate matTempData;

            if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
                matTempData.metallic = metallic;
            }
            else
            {
                matTempData.metallic = 0.0f;
            }
            if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
                matTempData.roughness = roughness;
            }
            else
            {
                matTempData.roughness = 1.0f;
            }

            modelData.materialTemplate.push_back(matTempData);
            modelData.material.push_back(matData);

        }
    }

    for (auto it = modelData.matVertexData.begin(); it != modelData.matVertexData.end();)
    {
        if (it->second.vertices.empty() || it->second.indices.empty())
        {
            Log("Model loader skipped an empty mesh.\n");
            it = modelData.matVertexData.erase(it);
            continue;
        }

        ++it;
    }

    modelData.rootNode = ReadNode(scene->mRootNode);
    return modelData;
}

Node ModelLoader::ReadNode(aiNode* node)
{
    Node result;
    aiVector3D scale, translate;
    aiQuaternion rotate;
    node->mTransformation.Decompose(scale, rotate, translate); // assimpの行列からSRTを抽出する関数を利用

    result.transform.scale = { scale.x, scale.y, scale.z }; // Scaleはそのまま
    result.transform.rotate = { rotate.x, -rotate.y, -rotate.z, rotate.w }; // x軸を反転、さらに回転方向が逆なので軸を反転させる
    result.transform.position = { -translate.x, translate.y, translate.z }; // x軸を反転
    result.localMatrix = MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.position);
    result.name = node->mName.C_Str(); // Node名を格納
    result.children.resize(node->mNumChildren); // 子供の数だけ確保
    for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
    {
        // 再帰的に読んで階層構造を作っていく
        result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
    }
    return result;
}

Animation ModelLoader::LoadAnimationFile(const std::string& directoryPath, const std::string& fileName)
{
    Animation result;
    Assimp::Importer importer;
    std::string filePath = directoryPath + "/" + fileName;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
    if (scene->mNumAnimations == 0)// アニメーションが無い
    {
        Log("this scene have not Animation");
        assert(0);
    };
    aiAnimation* animationAssimp = scene->mAnimations[0]; // 最初の差にメーションだけ採用。
    result.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond); // 時間の単位を秒に変換
    // NodeAnimationを解析する
    // assimpでは個々のNodeのAnimationをchannelと読んでいるのでchannelを回してNodeAnimationの情報をとってくる
    for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex)
    {
        aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
        NodeAnimation& nodeAnimation = result.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex)
        {
            aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
            KeyframeVector3 keyframe;
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
            keyframe.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; // 右手->左手
            nodeAnimation.position.push_back(keyframe);
        }
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex)
        {
            aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
            KeyframeQuaternion keyframe;
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
            keyframe.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w }; // 右手->左手
            nodeAnimation.rotate.push_back(keyframe);
        }
        for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex)
        {
            aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
            KeyframeVector3 keyframe;
            keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); // ここも秒に変換
            keyframe.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; // 右手->左手
            nodeAnimation.scale.push_back(keyframe);
        }
    }


    return result;
}

