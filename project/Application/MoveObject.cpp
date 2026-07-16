#include "MoveObject.h"
#include "Logger.h"
#include "GameTime.h"
#include "ImGuiManager.h"
#include "StageCount.h"
#include "CollisionManager.h"
#include "ModelManager.h"
#include "EngineContext.h"

using namespace Logger;
using namespace std;
using namespace ActionEngine::Stage;

std::string getFileName(const std::string& path) {
	size_t pos = path.find_last_of("/\\");
	return (pos == std::string::npos) ? path : path.substr(pos + 1);
}


MoveObject::MoveObject() {

}

MoveObject::~MoveObject() {
	for (int i = 0; i < m_moveObjects.size(); i++)
	{
		m_pContext->world.collision.DeleteCollision(m_moveObjects[i].object.get());
	}
}

void MoveObject::Initialize(std::string jsonName) {
	// Jsonが読み込まれていなかったら早期return
	if (!m_pContext->engine.assets.json.CheckJsonLoaded(jsonName)) {
		return;
	}
	random_device seedGenerator;
	mt19937 random(seedGenerator());
	m_randomEngine = random;
	m_num = 0;
	m_gameTimer = 0.0f;
	m_moveObjects.clear();
	string str;
	vector<JsonData> json;
	m_jsonName = jsonName;
	if (m_pContext->engine.assets.json.CheckJsonLoaded(jsonName))
	{
		// スタート地点の取得
		json = m_pContext->engine.assets.json.GetJsonData(jsonName, "trap");
		// スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
		if (!json.empty())
		{
            Model* trapModel = m_pContext->engine.assets.models.LoadModel("Resources/Model/obj", "trap.obj");
			for (auto data : json) {
				MoveObjectStruct moveObject;
				moveObject.object = m_pContext->game.object3dFactory.Create();
				moveObject.object->SetTransform(data.transform);
				moveObject.object->SetModel(trapModel);
                moveObject.object->SetEnableLighting(true);
				moveObject.start = data.transform;
				moveObject.trapData = data.moveObject;
				moveObject.startFrame = m_gameTimer;
				if (!data.moveObject.loop && !data.moveObject.reverse)
				{
					moveObject.reverse = true;
				}
				else
				{
					moveObject.reverse = false;
				}
				if (data.moveObject.reverse)
				{
					moveObject.trapData.runTime /= 2.0f;
				}
				if (data.moveObject.spawner)
				{
					if (data.moveObject.spawnerTime.y == -1.0f)
					{
						moveObject.trapData.spawnTime = data.moveObject.spawnerTime.x;
					}
					else
					{
						moveObject.trapData.spawnerTime = data.moveObject.spawnerTime;
						uniform_real_distribution<float> distribution(data.moveObject.spawnerTime.x, data.moveObject.spawnerTime.y);
						moveObject.trapData.spawnTime = distribution(m_randomEngine);
					}

				}
				moveObject.number = m_num;
				moveObject.object->Update();
				m_moveObjects.push_back(std::move(moveObject));
				m_pContext->world.collision.AddCollision(m_moveObjects[m_num].object.get());
				m_num++;
			}
		}
		else
		{
			Log("指定したJsonは読み込まれていません : " + jsonName + "\n実行プログラム" + getFileName(__FILE__));
		}
	}
}

void MoveObject::Update() {

    // 経過時間を更新
    m_gameTimer += m_pContext->engine.platform.time.GetDeltaTime();

    // -----------------------------
    // 初回更新時の初期化処理
    // -----------------------------
    if (!m_start)
    {
        Log("更新開始 : " + std::to_string(m_gameTimer) + "\n");

        // 全トラップの初期位置を設定し、開始フレームを記録
        for (int i = 0; i < m_moveObjects.size(); i++)
        {
            m_moveObjects[i].object->SetTransform(m_moveObjects[i].start);
            m_moveObjects[i].startFrame = m_gameTimer;
        }

        Log("更新初期処理終了 : " + std::to_string(m_gameTimer) + "\n");
        m_start = true;
    }

#ifndef NDEBUG
    // -----------------------------
    // デバッグ用 ImGui ウィンドウ
    // -----------------------------
    ImGui::Begin("Trap");
    ImGui::TextColored({ 1.0f, 1.0f, 1.0f, 1.0f }, "経過時間 %.1f", m_gameTimer);

    // JSON 再読み込みボタン
    if (ImGui::Button("Json再読み込み"))
    {
        // 既存コリジョン削除
        for (int i = 0; i < m_moveObjects.size(); i++)
        {
            m_pContext->world.collision.DeleteCollision(m_moveObjects[i].object.get());
        }
        // JSON 再ロード
        Initialize(m_jsonName);
    }
    ImGui::End();
#endif

    // -----------------------------
    // 各トラップの更新処理
    // -----------------------------
    for (int i = 0; i < m_moveObjects.size(); i++)
    {
        // -----------------------------------------
        // スポナー型トラップ（一定時間ごとに生成）
        // -----------------------------------------
        if (m_moveObjects[i].trapData.spawner)
        {
            // spawnTime を超えたら生成
            if (m_moveObjects[i].trapData.spawnTime <= m_gameTimer)
            {
                MakeMoveObject(m_moveObjects[i]);

                // 次の spawnTime を設定（ランダム or 固定）
                if (m_moveObjects[i].trapData.spawnerTime.y != -1.0f)
                {
                    uniform_real_distribution<float> distribution(
                        m_moveObjects[i].trapData.spawnerTime.x,
                        m_moveObjects[i].trapData.spawnerTime.y
                    );
                    m_moveObjects[i].trapData.spawnTime = distribution(m_randomEngine) + m_gameTimer;
                }
                else
                {
                    m_moveObjects[i].trapData.spawnTime = m_moveObjects[i].trapData.spawnerTime.x + m_gameTimer;
                }
            }
        }
        // -----------------------------------------
        // 移動トラップの反転・ループ処理
        // -----------------------------------------
        else if (m_gameTimer > m_moveObjects[i].startFrame + m_moveObjects[i].trapData.runTime &&
            m_moveObjects[i].trapData.runTime > 0.0f &&
            m_moveObjects[i].trapData.move)
        {
            // ループしない & すでに reverse 状態 → 削除
            if (!m_moveObjects[i].trapData.loop && m_moveObjects[i].reverse)
            {
                m_pContext->world.collision.DeleteCollision(m_moveObjects[i].object.get());
                m_moveObjects.erase(m_moveObjects.cbegin() + i);
                continue;
            }

            // reverse が false → 初期位置に戻す
            if (!m_moveObjects[i].trapData.reverse)
            {
                m_moveObjects[i].object->SetTransform(m_moveObjects[i].start);
                m_moveObjects[i].reverse = true;
            }
            else
            {
                // reverse が true → 速度反転して折り返し
                m_moveObjects[i].trapData.velocity *= -1.0f;
                m_moveObjects[i].start = m_moveObjects[i].object->GetTransform();
                m_moveObjects[i].reverse = !m_moveObjects[i].reverse;
            }

            // 新しい開始時間を記録
            m_moveObjects[i].startFrame = m_gameTimer;
        }

        // -----------------------------------------
        // トラップの Transform を補間して更新
        // -----------------------------------------
        Transform newTransform = m_moveObjects[i].object->GetTransform();

        // 0〜1 の補間係数
        float time = (m_gameTimer - m_moveObjects[i].startFrame) / m_moveObjects[i].trapData.runTime;
        time = std::clamp(time, 0.0f, 1.0f);

        // 各要素を Lerp で補間
        newTransform.scale = Lerp(m_moveObjects[i].start.scale, m_moveObjects[i].start.scale + m_moveObjects[i].trapData.velocity.scale, time);
        newTransform.rotate = Lerp(m_moveObjects[i].start.rotate, m_moveObjects[i].start.rotate + m_moveObjects[i].trapData.velocity.rotate, time);
        newTransform.position = Lerp(m_moveObjects[i].start.position, m_moveObjects[i].start.position + m_moveObjects[i].trapData.velocity.position, time);

        // Transform を適用して更新
        m_moveObjects[i].object->SetTransform(newTransform);
        m_moveObjects[i].object->Update();
    }
}

void MoveObject::Draw() {

	for (int i = 0; i < m_moveObjects.size(); i++)
	{
		if (!m_moveObjects[i].trapData.spawner)
		{
			m_moveObjects[i].object->Draw();
		}
	}
}

void MoveObject::SetDrawHeight(const float height) {

	for (int i = 0; i < m_moveObjects.size(); i++)
	{
		if (!m_moveObjects[i].trapData.spawner)
		{
			m_moveObjects[i].object->SetDrawHeight(height);
		}
	}
}

void MoveObject::MakeMoveObject(MoveObjectStruct& data) {
    MoveObjectStruct moveObject;
	moveObject.object = m_pContext->game.object3dFactory.Create();
	moveObject.object->SetTransform(data.start);
    moveObject.object->SetModel(m_pContext->engine.assets.models.FindModel("trap.obj"));
    moveObject.object->SetEnableLighting(true);
	moveObject.start = data.start;
	moveObject.trapData = data.trapData;
	moveObject.startFrame = m_gameTimer;
	if (!data.trapData.loop && !data.trapData.reverse)
	{
		moveObject.reverse = true;
	}
	else
	{
		moveObject.reverse = false;
	}
	moveObject.trapData.spawner = false;
	moveObject.number = m_num;
	moveObject.object->Update();
	m_moveObjects.push_back(std::move(moveObject));
	m_pContext->world.collision.AddCollision(m_moveObjects[int(m_moveObjects.size() - 1)].object.get());
	m_num++;
}
