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

void Goal::Initialize(const std::string jsonName) {

	isGoal_ = false;
	input = Input::GetInstance();

	if (JsonLoader::GetInstance()->CheckJsonLoaded(jsonName))
	{
		// スタート地点の取得
		vector<JsonData> data = JsonLoader::GetInstance()->GetJsonData(jsonName, "goal");
		// スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
		if (!data.empty())
		{
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