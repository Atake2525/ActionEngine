#include "Trap.h"
#include "Logger.h"
#include "GameTime.h"
#include "ImGuiManager.h"
#include "StageCount.h"
#include "CollisionManager.h"

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
	for (int i = 0; i < traps.size(); i++)
	{
		CollisionManager::GetInstance()->DeleteCollision(traps[i].object.get());
	}
}

void Trap::Initialize(std::string jsonName) {
	// Jsonが読み込まれていなかったら早期return
	if (!JsonLoader::GetInstance()->CheckJsonLoaded(jsonName)) {
		return;
	}
	random_device seedGenerator;
	mt19937 random(seedGenerator());
	randomEngine = random;
	num = 0;
	gameTimer_ = 0.0f;
	traps.clear();
	string str;
	vector<JsonData> json;
	jsonName_ = jsonName;
	if (JsonLoader::GetInstance()->CheckJsonLoaded(jsonName))
	{
		// スタート地点の取得
		json = JsonLoader::GetInstance()->GetJsonData(jsonName, "trap");
		// スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
		if (!json.empty())
		{
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
				CollisionManager::GetInstance()->AddCollision(traps[num].object.get());
				num++;
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
    gameTimer_ += GameTime::GetInstance()->GetDeltaTime();

    // -----------------------------
    // 初回更新時の初期化処理
    // -----------------------------
    if (!start)
    {
        Log("更新開始 : " + std::to_string(gameTimer_) + "\n");

        // 全トラップの初期位置を設定し、開始フレームを記録
        for (int i = 0; i < traps.size(); i++)
        {
            traps[i].object->SetTransform(traps[i].start);
            traps[i].startFrame = gameTimer_;
        }

        Log("更新初期処理終了 : " + std::to_string(gameTimer_) + "\n");
        start = true;
    }

#ifndef NDEBUG
    // -----------------------------
    // デバッグ用 ImGui ウィンドウ
    // -----------------------------
    ImGui::Begin("Trap");
    ImGui::TextColored({ 1.0f, 1.0f, 1.0f, 1.0f }, "経過時間 %.1f", gameTimer_);

    // JSON 再読み込みボタン
    if (ImGui::Button("Json再読み込み"))
    {
        // 既存コリジョン削除
        for (int i = 0; i < traps.size(); i++)
        {
            CollisionManager::GetInstance()->DeleteCollision(traps[i].object.get());
        }
        // JSON 再ロード
        Initialize(jsonName_);
    }
    ImGui::End();
#endif

    // -----------------------------
    // 各トラップの更新処理
    // -----------------------------
    for (int i = 0; i < traps.size(); i++)
    {
        // -----------------------------------------
        // スポナー型トラップ（一定時間ごとに生成）
        // -----------------------------------------
        if (traps[i].trapData.spawner)
        {
            // spawnTime を超えたら生成
            if (traps[i].trapData.spawnTime <= gameTimer_)
            {
                MakeTrap(traps[i]);

                // 次の spawnTime を設定（ランダム or 固定）
                if (traps[i].trapData.spawnerTime.y != -1.0f)
                {
                    uniform_real_distribution<float> distribution(
                        traps[i].trapData.spawnerTime.x,
                        traps[i].trapData.spawnerTime.y
                    );
                    traps[i].trapData.spawnTime = distribution(randomEngine) + gameTimer_;
                }
                else
                {
                    traps[i].trapData.spawnTime = traps[i].trapData.spawnerTime.x + gameTimer_;
                }
            }
        }
        // -----------------------------------------
        // 移動トラップの反転・ループ処理
        // -----------------------------------------
        else if (gameTimer_ > traps[i].startFrame + traps[i].trapData.runTime &&
            traps[i].trapData.runTime > 0.0f &&
            traps[i].trapData.move)
        {
            // ループしない & すでに reverse 状態 → 削除
            if (!traps[i].trapData.loop && traps[i].reverse)
            {
                CollisionManager::GetInstance()->DeleteCollision(traps[i].object.get());
                traps.erase(traps.cbegin() + i);
                continue;
            }

            // reverse が false → 初期位置に戻す
            if (!traps[i].trapData.reverse)
            {
                traps[i].object->SetTransform(traps[i].start);
                traps[i].reverse = true;
            }
            else
            {
                // reverse が true → 速度反転して折り返し
                traps[i].trapData.velocity *= -1.0f;
                traps[i].start = traps[i].object->GetTransform();
                traps[i].reverse = !traps[i].reverse;
            }

            // 新しい開始時間を記録
            traps[i].startFrame = gameTimer_;
        }

        // -----------------------------------------
        // トラップの Transform を補間して更新
        // -----------------------------------------
        Transform newTransform = traps[i].object->GetTransform();

        // 0〜1 の補間係数
        float time = (gameTimer_ - traps[i].startFrame) / traps[i].trapData.runTime;
        time = std::clamp(time, 0.0f, 1.0f);

        // 各要素を Lerp で補間
        newTransform.scale = Lerp(traps[i].start.scale, traps[i].start.scale + traps[i].trapData.velocity.scale, time);
        newTransform.rotate = Lerp(traps[i].start.rotate, traps[i].start.rotate + traps[i].trapData.velocity.rotate, time);
        newTransform.translate = Lerp(traps[i].start.translate, traps[i].start.translate + traps[i].trapData.velocity.translate, time);

        // Transform を適用して更新
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
	CollisionManager::GetInstance()->AddCollision(traps[int(traps.size() - 1)].object.get());
	num++;
}
