#include "Goal.h"
#include "ImGuiManager.h"
#include "SceneManager.h"
#include "FadeManager.h"
#include "Logger.h"
#include "StageCount.h"
#include "Collision.h"
#include "Player.h"
#include "GameTime.h"
#include "ModelManager.h"

using namespace Logger;
using namespace std;
using namespace ActionEngine::Stage;


Goal::~Goal() {

}

void Goal::Initialize(const LevelEditor& levelEditor, const std::string jsonName, Player* player) {

	m_isGoal = false;
	m_input = Input::GetInstance();
	m_player = player;
	m_result = std::make_unique<Result>();
	m_result->Initialize();

	if (levelEditor.CheckJsonLoaded(jsonName))
	{
		// スタート地点の取得
		vector<LevelEditorData> m_jsonDatas = levelEditor.GetJsonData(jsonName, "goal");
		// スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
		if (!m_jsonDatas.empty())
		{
			Model* model = ModelManager::GetInstance()->LoadModel("Resources/Model/obj", "goal.obj", false);
			for (int i = 0; i < m_jsonDatas.size(); i++)
			{
				unique_ptr<Object3d> goal;
				goal = make_unique<Object3d>();
				goal->Initialize();
				goal->SetModel(model);
				goal->SetEnableLighting(true);
				goal->SetTransform(m_jsonDatas[i].transform);
				goal->SetColor({ 0.0f, 1.0f, 0.2f, 0.4f });
				m_goalObjects.push_back(move(goal));
			}
		}
	}
}

void Goal::Update()
{
	m_result->Update();
	// ゴールした時の処理
	if (ChceckIsGoal())
	{
		//GameTime::GetInstance()->SetTimeScale(0.5f);
		m_player->SetFreeze(true);
		m_result->StageClear();
	}
}

const bool& Goal::ChceckIsGoal() {
	AABB aabb = m_player->GetAABB();
	for (int i = 0; i < m_goalObjects.size(); i++)
	{
		m_goalObjects[i]->Update();
		if (CollisionAABB(m_goalObjects[i]->GetAABB(), aabb))
		{
			m_isGoal = true;
		}
	}
	return m_isGoal;
}

void Goal::DrawGoalObject() {
	for (int i = 0; i < m_goalObjects.size(); i++)
	{
		m_goalObjects[i]->Draw();
	}
}

void Goal::DrawResult()
{
	m_result->Draw();
}
