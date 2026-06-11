#include "Trap.h"
#include "Logger.h"
#include "GameTime.h"
#include "ImGuiManager.h"
#include "StageCount.h"
#include "CollisionManager.h"
#include "ModelManager.h"

using namespace Logger;
using namespace std;
using namespace ActionEngine::Stage;

std::string getFileName(const std::string& path) {
	size_t pos = path.find_last_of("/\\");
	return (pos == std::string::npos) ? path : path.substr(pos + 1);
}


Trap::Trap() {

}

Trap::~Trap() {
	for (int i = 0; i < m_traps.size(); i++)
	{
		CollisionManager::GetInstance()->DeleteCollision(m_traps[i].object.get());
	}
}

void Trap::Initialize(const LevelEditor& levelEditor, std::string jsonName) {
    m_levelEditor = &levelEditor;
	// Jsonが読み込まれていなかったら早期return
	if (!levelEditor.CheckJsonLoaded(jsonName)) {
		return;
	}
	random_device seedGenerator;
	mt19937 random(seedGenerator());
	m_randomEngine = random;
	m_num = 0;
	m_gameTimer = 0.0f;
	m_traps.clear();
	string str;
	vector<LevelEditorData> json;
	m_jsonName = jsonName;
	if (levelEditor.CheckJsonLoaded(jsonName))
	{
		// スタート地点の取得
		json = levelEditor.GetJsonData(jsonName, "trap");
		// スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
		if (!json.empty())
		{
            Model* trapModel = ModelManager::GetInstance()->LoadModel("Resources/Model/obj", "trap.obj", true);
			for (auto data : json) {
				Traps trap;
				trap.object = make_unique<Object3d>();
				trap.object->Initialize();
				trap.object->SetTransform(data.transform);
				trap.object->SetModel(trapModel);
				trap.start = data.transform;
				trap.trapData = data.trap;
				trap.startFrame = m_gameTimer;
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
						trap.trapData.spawnTime = distribution(m_randomEngine);
					}

				}
				trap.number = m_num;
				trap.object->Update();
				m_traps.push_back(std::move(trap));
				CollisionManager::GetInstance()->AddCollision(m_traps[m_num].object.get());
				m_num++;
			}
		}
		else
		{
			Log("指定したJsonは読み込まれていません : " + jsonName + "\n実行プログラム" + getFileName(__FILE__));
		}
	}
}

void Trap::Update() {

    // 経過時間を更新
    m_gameTimer += GameTime::GetInstance()->GetDeltaTime();

    // -----------------------------
    // 初回更新時の初期化処理
    // -----------------------------
    if (!m_start)
    {
        Log("更新開始 : " + std::to_string(m_gameTimer) + "\n");

        // 全トラップの初期位置を設定し、開始フレームを記録
        for (int i = 0; i < m_traps.size(); i++)
        {
            m_traps[i].object->SetTransform(m_traps[i].start);
            m_traps[i].startFrame = m_gameTimer;
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
        for (int i = 0; i < m_traps.size(); i++)
        {
            CollisionManager::GetInstance()->DeleteCollision(m_traps[i].object.get());
        }
        // JSON 再ロード
        if (m_levelEditor)
        {
            Initialize(*m_levelEditor, m_jsonName);
        }
    }
    ImGui::End();
#endif

    // -----------------------------
    // 各トラップの更新処理
    // -----------------------------
    for (int i = 0; i < m_traps.size(); i++)
    {
        // -----------------------------------------
        // スポナー型トラップ（一定時間ごとに生成）
        // -----------------------------------------
        if (m_traps[i].trapData.spawner)
        {
            // spawnTime を超えたら生成
            if (m_traps[i].trapData.spawnTime <= m_gameTimer)
            {
                MakeTrap(m_traps[i]);

                // 次の spawnTime を設定（ランダム or 固定）
                if (m_traps[i].trapData.spawnerTime.y != -1.0f)
                {
                    uniform_real_distribution<float> distribution(
                        m_traps[i].trapData.spawnerTime.x,
                        m_traps[i].trapData.spawnerTime.y
                    );
                    m_traps[i].trapData.spawnTime = distribution(m_randomEngine) + m_gameTimer;
                }
                else
                {
                    m_traps[i].trapData.spawnTime = m_traps[i].trapData.spawnerTime.x + m_gameTimer;
                }
            }
        }
        // -----------------------------------------
        // 移動トラップの反転・ループ処理
        // -----------------------------------------
        else if (m_gameTimer > m_traps[i].startFrame + m_traps[i].trapData.runTime &&
            m_traps[i].trapData.runTime > 0.0f &&
            m_traps[i].trapData.move)
        {
            // ループしない & すでに reverse 状態 → 削除
            if (!m_traps[i].trapData.loop && m_traps[i].reverse)
            {
                CollisionManager::GetInstance()->DeleteCollision(m_traps[i].object.get());
                m_traps.erase(m_traps.cbegin() + i);
                continue;
            }

            // reverse が false → 初期位置に戻す
            if (!m_traps[i].trapData.reverse)
            {
                m_traps[i].object->SetTransform(m_traps[i].start);
                m_traps[i].reverse = true;
            }
            else
            {
                // reverse が true → 速度反転して折り返し
                m_traps[i].trapData.velocity *= -1.0f;
                m_traps[i].start = m_traps[i].object->GetTransform();
                m_traps[i].reverse = !m_traps[i].reverse;
            }

            // 新しい開始時間を記録
            m_traps[i].startFrame = m_gameTimer;
        }

        // -----------------------------------------
        // トラップの Transform を補間して更新
        // -----------------------------------------
        Transform newTransform = m_traps[i].object->GetTransform();

        // 0〜1 の補間係数
        float time = (m_gameTimer - m_traps[i].startFrame) / m_traps[i].trapData.runTime;
        time = std::clamp(time, 0.0f, 1.0f);

        // 各要素を Lerp で補間
        newTransform.scale = Lerp(m_traps[i].start.scale, m_traps[i].start.scale + m_traps[i].trapData.velocity.scale, time);
        newTransform.rotate = Lerp(m_traps[i].start.rotate, m_traps[i].start.rotate + m_traps[i].trapData.velocity.rotate, time);
        newTransform.translate = Lerp(m_traps[i].start.translate, m_traps[i].start.translate + m_traps[i].trapData.velocity.translate, time);

        // Transform を適用して更新
        m_traps[i].object->SetTransform(newTransform);
        m_traps[i].object->Update();
    }
}

void Trap::Draw() {

	for (int i = 0; i < m_traps.size(); i++)
	{
		if (!m_traps[i].trapData.spawner)
		{
			m_traps[i].object->Draw();
		}
	}
}

void Trap::SetDrawHeight(const float height) {

	for (int i = 0; i < m_traps.size(); i++)
	{
		if (!m_traps[i].trapData.spawner)
		{
			m_traps[i].object->SetDrawHeiht(height);
		}
	}
}

void Trap::MakeTrap(Traps& data) {
	Traps trap;
	trap.object = make_unique<Object3d>();
	trap.object->Initialize();
	trap.object->SetTransform(data.start);
    trap.object->SetModel(ModelManager::GetInstance()->FindModel("trap.obj"));
	trap.start = data.start;
	trap.trapData = data.trapData;
	trap.startFrame = m_gameTimer;
	if (!data.trapData.loop && !data.trapData.reverse)
	{
		trap.reverse = true;
	}
	else
	{
		trap.reverse = false;
	}
	trap.trapData.spawner = false;
	trap.number = m_num;
	trap.object->Update();
	m_traps.push_back(std::move(trap));
	CollisionManager::GetInstance()->AddCollision(m_traps[int(m_traps.size() - 1)].object.get());
	m_num++;
}
