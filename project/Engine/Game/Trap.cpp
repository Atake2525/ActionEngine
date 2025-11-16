#include "Trap.h"
#include "Logger.h"
#include "GameTime.h"
#include "ImGuiManager.h"
#include "StageCount.h"
#include "CollisionManager.h"

using namespace Logger;
using namespace std;

Trap::Trap() {

}

Trap::~Trap() {
	for (int i = 0; i < traps.size(); i++)
	{
		CollisionManager::GetInstance()->DeleteCollision("trap" + to_string(i));
	}
}

void Trap::Initialize(std::string jsonName) {

	gameTimer_ = 0.0f;
	traps.clear();
	string str;
	vector<JsonData> json;
	if (jsonName == "normal")
	{
		str = "map" + to_string(StageCount::GetInstance()->GetStageCount());
		json = JsonLoader::GetInstance()->GetJsonData(str, "trap");
	}
	else
	{
		json = JsonLoader::GetInstance()->GetJsonData(jsonName, "trap");
	}
	Log("指定したデータが" + std::to_string(json.size()) + "個見つかりました\n");
	int num = 0;
	for (auto data : json) {
		Traps trap;
		trap.type = TrapType::Spike;
		trap.object = make_unique<Object3d>();
		trap.object->Initialize();
		trap.object->SetTransform(data.transform);
		trap.object->SetModel("Resources/Model/obj", "trap.obj", true);
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
		trap.object->Update();
		traps.push_back(std::move(trap));
		CollisionManager::GetInstance()->AddCollision(traps[num].object.get(), "trap" + to_string(num));
		num++;
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
		Log("更新初期処理終了 : " + std::to_string(gameTimer_) + "\n");
		start = true;
	}
#ifndef NDEBUG
	ImGui::Begin("Trap");
	ImGui::TextColored({ 1.0f, 1.0f, 1.0f, 1.0f }, "経過時間 %.1f", gameTimer_);
	if (ImGui::Button("Json再読み込み"))
	{
		Initialize();
	}
	ImGui::End();
#endif
	for (int i = 0; i < traps.size(); i++)
	{
		if (gameTimer_ > traps[i].startFrame + traps[i].trapData.runTime && traps[i].trapData.runTime > 0.0f && traps[i].trapData.move)
		{
			switch (traps[i].trapData.spawner)
			{
			case true:

				break;
			case false:
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
					traps[i].start = traps[i].object->GetTransform();
					traps[i].reverse = !traps[i].reverse;
				}
				traps[i].startFrame = gameTimer_;
				break;
			}


		}
		Transform newTransform = traps[i].object->GetTransform();

		float time = (gameTimer_ - traps[i].startFrame) / traps[i].trapData.runTime;
		time = std::clamp(time, 0.0f, 1.0f);
		newTransform.scale = Lerp(traps[i].start.scale, traps[i].start.scale + traps[i].trapData.velocity.scale, time);
		newTransform.rotate = Lerp(traps[i].start.rotate, traps[i].start.rotate + traps[i].trapData.velocity.rotate, time);
		newTransform.translate = Lerp(traps[i].start.translate, traps[i].start.translate + traps[i].trapData.velocity.translate, time);

		/*newTransform.scale += traps[i].trapData.velocity.scale;
		newTransform.rotate += traps[i].trapData.velocity.rotate;
		newTransform.translate += traps[i].trapData.velocity.translate;*/
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

void Trap::SetDrawHeight(const float height)
{
	for (int i = 0; i < traps.size(); i++)
	{
		traps[i].object->SetDrawHeiht(height);
	}
}
