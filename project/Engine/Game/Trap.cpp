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
		trap.object->SetModel("Resources/Debug/obj", "box.obj", true);
		trap.start = data.transform;
		trap.trapData = data.trap;
		trap.startFrame = gameTimer_;
		if (!data.trap.loop && !data.trap.reverse)
		{
			trap.reverse = true;
		}
		else
		{
			trap.reverse = false;
		}
		traps.push_back(std::move(trap));
	}
}

void Trap::Update() {
	gameTimer_ += GameTime::GetInstance()->GetDeltaTime();
	if (!start)
	{
		Log("更新開始 : " + std::to_string(gameTimer_) + "\n");
		for (int i = 0; i < traps.size(); i++)
		{
			traps[i].object->SetTransform(traps[i].start);
			traps[i].startFrame = gameTimer_;
		}
		gameTimer_ = 0.0f;
		GameTime::GetInstance()->SetDeltaPoint();
		Log("更新初期処理終了 : " + std::to_string(gameTimer_) + "\n");
		start = true;
	}
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
			if (!traps[i].trapData.loop && traps[i].reverse)
			{
				traps.erase(traps.cbegin() + i);
				continue;
			}
			if (!traps[i].trapData.reverse)
			{
				traps[i].object->SetTransform(traps[i].start);
				traps[i].reverse = true;
			}
			else
			{
				traps[i].trapData.velocity *= -1.0f;
				traps[i].reverse = !traps[i].reverse;
			}
			traps[i].startFrame = gameTimer_;


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
