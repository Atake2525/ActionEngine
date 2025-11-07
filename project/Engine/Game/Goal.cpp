#include "Goal.h"
#include "ImGuiManager.h"
#include "SceneManager.h"
#include "FadeManager.h"
#include "Logger.h"
#include "StageCount.h"

using namespace Logger;
using namespace std;


Goal::~Goal() {

}

void Goal::Initalize() {
	input = Input::GetInstance();
	int stageCount = StageCount::GetInstance()->GetStageCount();
	string str = "Resources/Json/Stage/map" + to_string(stageCount) + ".json";

	
	JsonLoader::GetInstance()->LoadJson(str, "map" + to_string(stageCount), false);
	jsonDatas = JsonLoader::GetInstance()->GetJsonData("map" + to_string(stageCount), "goal");

	for (int i = 0; i < jsonDatas.size(); i++)
	{
		unique_ptr<Object3d> goal;
		goal = make_unique<Object3d>();
		goal->Initialize();
		goal->SetModel("Resources/Debug/obj", "box.obj");
		goal->SetTransform(jsonDatas[i].transform);
		goalObjects.push_back(goal);
	}
}

void Goal::Update() {
	for (int i = 0; i < jsonDatas.size(); i++)
	{
		goalObjects[i]->Update();
	}
}

void Goal::Draw() {
	for (int i = 0; i < jsonDatas.size(); i++)
	{
		goalObjects[i]->Draw();
	}
}