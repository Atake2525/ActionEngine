#include "Trap.h"
#include "Logger.h"
#include "GameTime.h"
#include "ImGuiManager.h"

using namespace Logger;
using namespace std;

void Trap::Initialize(std::string path) {

	JsonLoader::GetInstance()->LoadJsonTransform(path, "Trap", true);
	jsonPath = path;
	gameTimer_ = 0.0f;
	traps.clear();
	vector<JsonData> json = JsonLoader::GetInstance()->GetJsonData("Trap", "trap");
	Log("指定したデータが" + std::to_string(json.size()) + "個見つかりました\n");
	for (auto data : json) {
		Traps trap;
		trap.type = TrapType::Spike;
		trap.object = make_unique<Object3d>();
		trap.object->Initialize();
		trap.object->SetTransform(data.transform);
		trap.object->SetModel("Resources/Debug/obj", "box.obj");
		trap.start = data.transform;
		trap.trapData = data.trap;
		trap.startFrame = gameTimer_;
		traps.push_back(std::move(trap));
	}
}

void Trap::Update() {
	gameTimer_ += GameTime::GetInstance()->GetDeltaTime();

#ifndef _NDEBUG
	ImGui::Begin("Trap");
	ImGui::TextColored({ 1.0f, 1.0f, 1.0f, 1.0f }, "経過時間 %.1f", gameTimer_);
	if (ImGui::Button("Json再読み込み"))
	{
		Initialize(jsonPath);
	}
	ImGui::End();
#endif
	for (int i = 0; i < traps.size(); i++)
	{
		if (gameTimer_ > traps[i].startFrame + traps[i].trapData.runTime && traps[i].trapData.runTime > 0.0f)
		{
			if (!traps[i].trapData.loop)
			{
				traps.erase(traps.cbegin() + i);
				continue;
			}
			traps[i].startFrame = gameTimer_;
			traps[i].object->SetTransform(traps[i].start);


		}
		Transform newTransform = traps[i].object->GetTransform();
		newTransform.scale += traps[i].trapData.velocity.scale;
		newTransform.rotate += traps[i].trapData.velocity.rotate;
		newTransform.translate += traps[i].trapData.velocity.translate;
		traps[i].object->SetTransform(newTransform);
		traps[i].object->Update();
	}
}

void Trap::Draw() {

	for (int i = 0; i < traps.size(); i++)
	{
		traps[i].object->Draw();
	}
}
