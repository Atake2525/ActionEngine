#include "JsonLoader.h"
#include "Logger.h"

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

nlohmann::json JsonLoader::LoadJson(const std::string fullPath) {
    std::fstream file;
    file.open(fullPath);

    nlohmann::json deserialized;
    if (file.fail())
    {
        Log("ファイル展開に失敗しました。正しい位置にファイルがあるか確認してください。\nファイルの位置 : " + fullPath);
        return nlohmann::json{};
    }

    file >> deserialized;
    return deserialized;
}
