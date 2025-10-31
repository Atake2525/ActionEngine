#include "Trap.h"
#include "JsonLoader.h"
#include "Logger.h"

using namespace Logger;
using namespace std;

void Trap::Initialize(std::string path) {

	JsonLoader::GetInstance()->LoadJsonTransform(path, "Trap", true);

	vector<JsonData> json = JsonLoader::GetInstance()->GetJsonData("Trap", "trap");
	Log("指定したデータが" + std::to_string(json.size()) + "個見つかりました\n");
	for (auto data : json) {
		Traps trap;
		trap.type = TrapType::Spike;
		trap.object = make_unique<Object3d>();
		trap.object->Initialize();
		trap.object->SetTransform(data.transform);
		trap.object->SetModel("Resources/Debug/obj", "box.obj");
		traps.push_back(std::move(trap));
	}
	

}

void Trap::Update() {

	UpdateSpikeTrap();
}

void Trap::Draw() {

	for (int i = 0; i < traps.size(); i++)
	{
		traps[i].object->Draw();
	}
}

void Trap::UpdateSpikeTrap() {

	for (int i = 0; i < traps.size(); i++)
	{
		if (traps[i].type == TrapType::Spike)
		{
			Vector3 rotate = traps[i].object->GetRotate();
			traps[i].object->SetRotate({ rotate.x, rotate.y + SwapRadian(1.0f), rotate.z});
			traps[i].object->Update();
		}
	}

}
