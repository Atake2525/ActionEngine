#include "Goal.h"
#include "ImGuiManager.h"
#include "SceneManager.h"
#include "FadeManager.h"
#include "Logger.h"
#include "StageCount.h"
#include "Collision.h"

using namespace Logger;
using namespace std;
using namespace ActionEngine::Stage;


Goal::~Goal() {

}

void Goal::Initalize() {

	isGoal_ = false;
	input = Input::GetInstance();
	int stageCount = StageCount::GetInstance()->GetStageCount();
	jsonDatas = JsonLoader::GetInstance()->GetJsonData("map" + to_string(stageCount), "goal");

	for (int i = 0; i < jsonDatas.size(); i++)
	{
		unique_ptr<Object3d> goal;
		goal = make_unique<Object3d>();
		goal->Initialize();
		goal->SetModel("Resources/Model/obj", "goal.obj");
		goal->SetTransform(jsonDatas[i].transform);
		goal->SetColor({ 0.0f, 1.0f, 0.2f, 0.4f });
		goalObjects.push_back(move(goal));
	}
}

void Goal::Update(AABB aabb) {
	for (int i = 0; i < jsonDatas.size(); i++)
	{
		goalObjects[i]->Update();
		if (CollisionAABB(goalObjects[i]->GetAABB(), aabb))
		{
			isGoal_ = true;
		}
	}
}

void Goal::Draw() {
	for (int i = 0; i < jsonDatas.size(); i++)
	{
		goalObjects[i]->Draw();
	}
}