#include "Trap.h"
#include "Logger.h"
#include "GameTime.h"
#include "ImGuiManager.h"
#include "StageCount.h"
#include "CollisionManager.h"

using namespace Logger;
using namespace std;
using namespace ActionEngine::Stage;

Trap::Trap() {

}

Trap::~Trap() {
	for (int i = 0; i < traps.size(); i++)
	{
		CollisionManager::GetInstance()->DeleteCollision("trap" + to_string(traps[i].number));
	}
}

void Trap::Initialize(std::string jsonName) {
	random_device seedGenerator;
	mt19937 random(seedGenerator());
	randomEngine = random;
	num = 0;
	gameTimer_ = 0.0f;
	traps.clear();
	string str;
	vector<JsonData> json;
	jsonName_ = jsonName;
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
	for (auto data : json) {
		Traps trap;
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
		if (data.trap.reverse)
		{
			trap.trapData.runTime /= 2.0f;
		}
		if (data.trap.spawner)
		{
			if (data.trap.spawnerTime.y == -1.0f)
			{
				trap.trapData.spawnTime = data.trap.spawnerTime.x;
			}
			else
			{
				trap.trapData.spawnerTime = data.trap.spawnerTime;
				uniform_real_distribution<float> distribution(data.trap.spawnerTime.x, data.trap.spawnerTime.y);
				trap.trapData.spawnTime = distribution(randomEngine);
			}

		}
		trap.number = num;
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
		for (int i = 0; i < traps.size(); i++)
		{
			CollisionManager::GetInstance()->DeleteCollision("trap" + to_string(traps[i].number));
		}
		Initialize(jsonName_);
	}
	ImGui::End();
#endif
	for (int i = 0; i < traps.size(); i++)
	{
		if (traps[i].trapData.spawner)
		{
			if (traps[i].trapData.spawnTime <= gameTimer_)
			{
				MakeTrap(traps[i]);
				if (traps[i].trapData.spawnerTime.y != -1.0f)
				{
					uniform_real_distribution<float> distribution(traps[i].trapData.spawnerTime.x, traps[i].trapData.spawnerTime.y);
					traps[i].trapData.spawnTime = distribution(randomEngine) + gameTimer_;
				}
				else
				{
					traps[i].trapData.spawnTime = traps[i].trapData.spawnerTime.x + gameTimer_;
				}
			}
		}else if (gameTimer_ > traps[i].startFrame + traps[i].trapData.runTime && traps[i].trapData.runTime > 0.0f && traps[i].trapData.move)
		{
			if (!traps[i].trapData.loop && traps[i].reverse)
			{
				CollisionManager::GetInstance()->DeleteCollision("trap" + to_string(traps[i].number));
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

		}
		Transform newTransform = traps[i].object->GetTransform();

		float time = (gameTimer_ - traps[i].startFrame) / traps[i].trapData.runTime;
		time = std::clamp(time, 0.0f, 1.0f);
		newTransform.scale = Lerp(traps[i].start.scale, traps[i].start.scale + traps[i].trapData.velocity.scale, time);
		newTransform.rotate = Lerp(traps[i].start.rotate, traps[i].start.rotate + traps[i].trapData.velocity.rotate, time);
		newTransform.translate = Lerp(traps[i].start.translate, traps[i].start.translate + traps[i].trapData.velocity.translate, time);

		traps[i].object->SetTransform(newTransform);
		traps[i].object->Update();
	}
}

void Trap::Draw() {

	for (int i = 0; i < traps.size(); i++)
	{
		if (!traps[i].trapData.spawner)
		{
			traps[i].object->Draw();
		}
	}
}

void Trap::SetDrawHeight(const float height) {

	for (int i = 0; i < traps.size(); i++)
	{
		if (!traps[i].trapData.spawner)
		{
			traps[i].object->SetDrawHeiht(height);
		}
	}
}

void Trap::MakeTrap(Traps& data) {
	Traps trap;
	trap.object = make_unique<Object3d>();
	trap.object->Initialize();
	trap.object->SetTransform(data.start);
	trap.object->SetModel("Resources/Model/obj", "trap.obj", true);
	trap.start = data.start;
	trap.trapData = data.trapData;
	trap.startFrame = gameTimer_;
	if (!data.trapData.loop && !data.trapData.reverse)
	{
		trap.reverse = true;
	}
	else
	{
		trap.reverse = false;
	}
	trap.trapData.spawner = false;
	trap.number = num;
	trap.object->Update();
	traps.push_back(std::move(trap));
	CollisionManager::GetInstance()->AddCollision(traps[int(traps.size() - 1)].object.get(), "trap" + to_string(num));
	num++;
}
