#include "Trap.h"
#include "JsonLoader.h"

void Trap::Initialize(std::string jsonName) {
    JsonData data = JsonLoader::GetInstance()->GetJsonData(jsonName, "trap");


}