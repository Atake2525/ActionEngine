#include "JsonLoader.h"
#include <cassert>
#include "Logger.h"
#include "kMath.h"

using namespace Logger;

JsonLoader* JsonLoader::instance = nullptr;

JsonLoader* JsonLoader::GetInstance() {
    if (instance == nullptr)
    {
        instance = new JsonLoader;
    }
    return instance;
}

void JsonLoader::Finalize() {
    delete instance;
    instance = nullptr;
}

void JsonLoader::Initialize() {

}

bool JsonLoader::CheckJsonLoaded(const std::string& jsonName)
{
    return levelDatas.contains(jsonName);
}

void JsonLoader::DeleteJson(const std::string& jsonName)
{
    if (levelDatas.contains(jsonName))
    {
        levelDatas.erase(jsonName);
    }
}

void JsonLoader::LoadJson(const std::string& path, const std::string& jsonName, const bool overwrite)
{
    LevelData lvData;

    if (levelDatas.contains(jsonName) && !overwrite)
    {
        Log("指定したjsonNameは既に使用されています\n");
        return;
    }

    Transform tl = { {0.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f},{0.0f, 0.0f,0.0f} };
    JsonData data;
    data.transform = tl;
    data.file_name = "null";
    data.name = "null";
    data.type = "null";
    lvData.datas["null"] = data;
    lvData.name = "null";

    // 連結してファイルパスを得る
    const std::string fullpath = path;

    // ファイルストリーム
    std::ifstream file;

    // ファイルを開く
    file.open(fullpath);
    // ファイルオープン失敗をチェック
    if (file.fail())
    {
        Log("ファイルの展開に失敗しました\nファイルパスが正しい確認してください\n");
        return;
    }

    // JSON文字列から解凍したデータ
    nlohmann::json deserialized;

    // 解凍
    file >> deserialized;

    // 正しいレベルデータファイルかチェック
    if (!deserialized.is_object())
    {
        Log("正しいレベルデータファイルではありません\nBlockPoint is_object\n");
        return;
    }
    if (!deserialized.contains("name"))
    {
        Log("正しいレベルデータファイルではありません\nBlockPoint contains\n");
        return;
    }
    if (!deserialized["name"].is_string())
    {
        Log("正しいレベルデータファイルではありません\nBlockPoint is_string\n");
        return;
    }

    // "name"を文字列として取得
    std::string name = deserialized["name"].get<std::string>();
    // 正しいレベルデータファイルかチェック
    if (name.compare("scene") != 0)
    {
        Log("正しいレベルデータファイルではありません\nBlockPoint compare\n");
        return;
    }

    // レベルデータ格納用インスタンスを生成
    LevelData levelData;

    // "name"を文字列として取得
    levelData.name = deserialized["name"].get<std::string>();
    if (levelData.name != "scene")
    {
        Log("シーンではない");
        return;
    }

    // "object"の全オブジェクトを走査
    for (nlohmann::json& object : deserialized["objects"])
    {
        if (!object.contains("type"))
        {
            Log("objectにtypeが存在しません\n");
            return;
        }

        // 種別をMESHかCAMERAのみ通るようにする
        if (object["type"].get<std::string>() == "MESH" || object["type"].get<std::string>() == "CAMERA")
        {
            // // 同じ名前なら後ろに数字を追加する
            //if (levelData.datas.contains(object["file_name"].get<std::string>()))
            //{
            //    for (size_t i = 0; i < levelData.datas.size(); i++)
            //    {
            //        std::string num = "_0" + std::to_string(i);
            //        if (!levelData.datas.contains(object["file_name"].get<std::string>() + num))
            //        {
            //            jsonData = levelData.datas[object["file_name"].get<std::string>() + num];
            //            break;
            //        }
            //    }
            //}
            if (!object.contains("file_name"))
            {
                Log("objectにfile_nameが存在しません\n");
                return;
            }
            // 1個分の要素の準備
            JsonData& jsonData = levelData.datas[object["file_name"].get<std::string>()];

            jsonData.type = object["type"].get<std::string>(); // "type"
            jsonData.name = object["name"].get<std::string>(); // "name"

            // transformのパラメータ読み込み
            nlohmann::json& transform = object["transform"];
            // 平行移動 "translation"
            jsonData.transform.translate.x = (float)transform["translation"][0];
            jsonData.transform.translate.y = (float)transform["translation"][2];
            jsonData.transform.translate.z = (float)transform["translation"][1];
            if (object["type"].get<std::string>() == "MESH")
            {
                // 回転角 "rotation"
                jsonData.transform.rotate.x = -SwapRadian((float)transform["rotation"][0]);
                jsonData.transform.rotate.y = -SwapRadian((float)transform["rotation"][2]);
                jsonData.transform.rotate.z = -SwapRadian((float)transform["rotation"][1]);
            }
            else if (object["type"].get<std::string>() == "CAMERA")
            {
                // 回転角 "rotation"
                jsonData.transform.rotate.x = SwapRadian(90.0f) - SwapRadian((float)transform["rotation"][0]);
                jsonData.transform.rotate.y = -SwapRadian((float)transform["rotation"][2]);
                jsonData.transform.rotate.z = -SwapRadian((float)transform["rotation"][1]);
            }
            // 拡大縮小 "scaling"
            jsonData.transform.scale.x = (float)transform["scaling"][0];
            jsonData.transform.scale.y = (float)transform["scaling"][2];
            jsonData.transform.scale.z = (float)transform["scaling"][1];
            
            nlohmann::json trap = object["trap"];

            nlohmann::json& translate = trap["velocity_translation"];
            nlohmann::json& rotate = trap["velocity_rotation"];
            nlohmann::json& scale = trap["velocity_scale"];
            nlohmann::json& runtime = trap["runtime"];
            if (trap["move"].get<std::string>() == "true")
            {
                jsonData.trap.move = true;

                jsonData.trap.velocity.translate.x = (float)translate[0];
                jsonData.trap.velocity.translate.y = (float)translate[1];
                jsonData.trap.velocity.translate.z = (float)translate[2];

                jsonData.trap.velocity.rotate.x = SwapRadian((float)rotate[0]);
                jsonData.trap.velocity.rotate.y = SwapRadian((float)rotate[1]);
                jsonData.trap.velocity.rotate.z = SwapRadian((float)rotate[2]);

                jsonData.trap.velocity.scale.x = (float)scale[0];
                jsonData.trap.velocity.scale.y = (float)scale[1];
                jsonData.trap.velocity.scale.z = (float)scale[2];

                jsonData.trap.runTime = (float)runtime;

                if (trap["loop"].get<std::string>() == "true")
                {
                    jsonData.trap.loop = true;
                } 
                else 
                { 
                    jsonData.trap.loop = false; 
                }
                if (trap["reverse"].get<std::string>() == "true")
                {
                    jsonData.trap.reverse = true;
                }
                else
                {
                    jsonData.trap.reverse = false;
                }

                if (trap["enable_spawner"].get<std::string>() == "true")
                {
                    jsonData.trap.spawner = true;
                    nlohmann::json spawner = trap["spawner"];
                    nlohmann::json& spawnTime = spawner["spawn_time"];
                    if (spawner["type"].get<std::string>() == "constant")
                    {
                        jsonData.trap.spawnerTime = { (float)spawnTime, -1.0f };
                    }
                    else if (spawner["type"].get<std::string>() == "random")
                    {
                        jsonData.trap.spawnerTime = { (float)spawnTime[0], (float)spawnTime[1] };
                    }
                }

            }
            else
            {
                jsonData.trap.move = false;
            }
            // "file_name"
            if (object.contains("file_name"))
            {
                jsonData.file_name = object["file_name"].get<std::string>();
            }

            if (object.contains("children"))
            {
                for (nlohmann::json& children : object["children"])
                {
                    // transformのパラメータ読み込み
                    nlohmann::json& childTransform = children["transform"];
                    // 平行移動 "translation"
                    Transform transformchild;
                    JsonData data;
                    transformchild.translate.x = (float)childTransform["translation"][0];
                    transformchild.translate.y = (float)childTransform["translation"][1];
                    transformchild.translate.z = (float)childTransform["translation"][2];
                    // 回転角 "rotation"
                    transformchild.rotate.x = -(float)childTransform["rotation"][0];
                    transformchild.rotate.y = -(float)childTransform["rotation"][1];
                    transformchild.rotate.z = -(float)childTransform["rotation"][2];
                    // 拡大縮小 "scaling"
                    transformchild.scale.x = (float)childTransform["scaling"][0];
                    transformchild.scale.y = (float)childTransform["scaling"][1];
                    transformchild.scale.z = (float)childTransform["scaling"][2];

                    data.transform = transformchild;
                    if (children["file_name"] == NULL)
                    {
                        Log("file_nameがNULLです\nfile_nameが記入されているか確認してください");
                        continue;
                    }
                    data.file_name = children["file_name"].get<std::string>();
                    data.name = children["name"].get<std::string>();
                    data.type = children["type"].get<std::string>();

                }
            }
        }
    }
    if (overwrite)
    {
        levelDatas[jsonName].datas.clear();
        levelDatas[jsonName].name = "NULL";
    }
    levelDatas[jsonName] = levelData;
}

void JsonLoader::SerchTransformFunctional(const std::string& jsonName, const std::string file_name, std::function<void(Transform transform)> function)
{
    for (auto data : levelDatas[jsonName].datas)
    {
        if (data.second.file_name == file_name)
        {
            function(data.second.transform);
        }
    }
}

const std::vector<JsonData> JsonLoader::GetJsonData(const std::string& jsonName, const std::string file_name) {
    std::vector<JsonData> result;
    for (auto data : levelDatas[jsonName].datas)
    {
        if (data.second.file_name.find(file_name) == !std::string::npos)
        {
            result.push_back(data.second);
        }
    }
    return result;
}

nlohmann::json JsonLoader::LoadJson(const std::string fullPath) {
    // ファイルストリーム
    std::fstream file;
    // ファイル展開
    file.open(fullPath);
    // 展開失敗のチェック
    if (file.fail())
    {
        Log("ファイル展開に失敗しました。正しい位置にファイルがあるか確認してください。/nファイルの位置 : " + fullPath);
        return;
    }

    // Json文字列から解凍したデータ
    nlohmann::json deserialized;

    // 解凍
    file >> deserialized;
    return deserialized;
}
