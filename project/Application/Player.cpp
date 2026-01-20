#include "Player.h"
#include "JsonLoader.h"
#include "Camera.h"
#include "Input.h"

using namespace std;

void Player::Initialize(Camera* camera, const std::string& jsonName)
{
    // カメラのセット
    m_pCamera = camera;

    // JsonDataからステージ情報を取得してプレイヤーの初期位置とゴールの位置を設定する処理
    if (JsonLoader::GetInstance()->CheckJsonLoaded(jsonName))
    {
        // スタート地点の取得
        vector<JsonData> data = JsonLoader::GetInstance()->GetJsonData(jsonName, "startpoint");
        // スタート地点が設定されていない又はjsonが読み込めなかった場合はデフォルト位置を使用
        if (!data.empty())
        {
            m_transform = data[0].transform;
        }
    }

    // プレイヤーモデルの初期化
    m_pModel = make_unique<Object3d>();
    m_pModel->Initialize();
    m_pModel->SetModel("Resources/Model/obj/Player", "Player.obj", true);
    m_pModel->SetTransform(m_transform);
}

void Player::Update()
{
    m_pModel->SetTransform(m_transform);
    m_pModel->Update();
    m_pCamera->SetTranslate(m_transform.translate);
    m_pCamera->Update();

}

void Player::Draw()
{
    m_pModel->Draw();
}

void Player::HandleInput()
{
    if (Input::GetInstance()->PushKey(DIK_W))
    {
        m_moveInput.y = -1.0f;
    }
    if (Input::GetInstance()->PushKey(DIK_S))
    {

    }
}
