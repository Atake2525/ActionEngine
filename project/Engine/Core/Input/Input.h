#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>
#include <wrl.h>
#include <Windows.h>
#include "Vector2.h"
#include "Vector3.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#pragma once

enum class DPad {
	None,
	Up,
	UpRight,
	UpLeft,
	Down,
	DownRight,
	DownLeft,
	Left,
	Right,
};

enum class Controller {
	None,
	A,
	B,
	X,
	Y,
	LB,
	RB,
	LT,
	RT,
	View,
	Menu,
	LeftStick,
	RightStick,
};

class Input {
private:
	// シングルトンパターンを適用
	static Input* instance;

	// コンストラクタ、デストラクタの隠蔽
	Input() = default;
	~Input() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	//Input(Input&) = delete;
	//Input& operator=(Input&) = delete;

public:
	// namespcae省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>Input* instance</returns>
	static Input* GetInstance();

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	void Initialize();
	void Update();

	// デバイスの更新(デバイスの再認識に使う)
	void UpdateDevice();

public:
	/// <summary>
	/// キーの押下をチェック
	/// </summary>
	/// <param name="keyNumber">キー番号 例(DIK_0)</param>
	/// <returns>押されているか</returns>
	bool PushKey(BYTE keyNumber);

	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	/// <param name="keyNumber">キー番号 例(DIK_0)</param>
	/// <returns>押したかどうか</returns>
	bool TriggerKey(BYTE keyNumber);


	/// <summary>
	/// キーのリターンをチェック
	/// </summary>
	/// <param name="keyNumber">キー番号 例(DIK_0)</param>
	/// <returns>離されたか</returns>
	bool ReturnKey(BYTE keyNumber);

	/// <summary>
	/// マウスの押下をチェック
	/// </summary>
	/// <param name="mouseNumer">0 = 左クリック</param>
	/// <param name="mouseNumer">1 = 右クリック</param>
	/// <param name="mouseNumer">2 = マウスホイール押し込み</param>
	bool PressMouse(int mouseNumer);

	/// <summary>
	/// マウスのトリガーをチェック
	/// </summary>
	/// <param name="mouseNumer">0 = 左クリック</param>
	/// <param name="mouseNumer">1 = 右クリック</param>
	/// <param name="mouseNumer">2 = マウスホイール押し込み</param>
	bool TriggerMouse(int mouseNumber);

	/// <summary>
	/// マウスのリターンをチェック
	/// </summary>
	/// <param name="mouseNumer">0 = 左クリック</param>
	/// <param name="mouseNumer">1 = 右クリック</param>
	/// <param name="mouseNumer">2 = マウスホイール押し込み</param>
	bool ReturnMouse(int mouseNumber);
	
	Vector2 GetMousePos2();

	Vector3 GetMousePos3();

	Vector2 GetWindowMousePos2();

	Vector3 GetWindowMousePos3();

	// マウスの移動量を取得(Vector2)
	Vector2 GetMouseVel2();

	// マウスの移動量を取得(Vector3)
	Vector3 GetMouseVel3();

	/// <summary>
	/// マウスカーソルの表示変更
	/// </summary>
	/// <param name="">True  = 表示</param>
	/// <param name="">False = 非表示</param>
	void ShowMouseCursor(bool flag);

	// ジョイスティック左の傾きを取得(Vector2)
	Vector2 GetLeftJoyStickPos2(const float deadZone);

	// ジョイスティック左の傾きを取得(Vector3)
	Vector3 GetLeftJoyStickPos3(const float deadZone);

	// ジョイスティック右の傾きを取得(Vector2)
	Vector2 GetRightJoyStickPos2(const float deadZone);

	// ジョイスティック右の傾きを取得(Vector3)
	Vector3 GetRightJoyStickPos3(const float deadZone);

	// ジョイスティックの方向を取得(Vector2)
	float GetJoyStickDirection2(const Vector2 joyStickPos);

	// ジョイスティックの方向を取得(Vector3)
	float GetJoyStickDirection3(const Vector3 joyStickPos);

	// ジョイスティックの移動移動量を計算
	Vector2 GetJoyStickVelocity(const Vector2 joyStickPos, const Vector3 velocity, const bool acceleration = true);

	// ジョイスティックの移動移動量を計算
	Vector3 GetJoyStickVelocity(const Vector3 joyStickPos, const Vector3 velocity, const bool acceleration = true);

	// ジョイスティックが中央からどれだけ離れているかを計算
	float GetJoyStickLength(const Vector2 joyStickPos);

	float GetJoyStickLength(const Vector3 joyStickPos);

	bool IsMoveLeftJoyStick();

	bool IsMoveRightJoyStick();


	/// <summary>
	/// 十字キー(コントローラー)の押下をチェック
	/// </summary>
	bool PushXButton(DPad dPad);

	/// <summary>
	/// 十字キー(コントローラー)のトリガーをチェック
	/// </summary>
	bool TriggerXButton(DPad dPad);

	/// <summary>
	/// ボタン(コントローラー)の押下をチェック
	/// </summary>
	bool PushButton(Controller button);

	/// <summary>
	/// ボタン(コントローラー)のトリガーをチェック
	/// </summary>
	bool TriggerButton(Controller button);

	/// <summary>
	/// ボタン(コントローラー)のリターンをチェック
	/// </summary>
	bool ReturnButton(Controller button);

private:

	// DirectInputのインスタンス生成 キーボード
	ComPtr<IDirectInput8> directInput = nullptr;
	// DorectxInputのインスタンス生成 マウス
	ComPtr<IDirectInput8> directInputMouse = nullptr;
	// DorectxInputのインスタンス生成 コントローラー(ゲームパッド)
	ComPtr<IDirectInput8> directInputGamePad = nullptr;

	void CreateKeyboardDevice();
	void CreateMouseDevice();
	void CreateControllerDevice();

	// キーボードデバイス
	ComPtr<IDirectInputDevice8> keyboard;

	// 全キーの状態
	BYTE keys[256] = {};
	// 前回の全キーの状態
	BYTE keyPres[256] = {};

	// マウスデバイス
	ComPtr<IDirectInputDevice8> mouse;

	// 全マウスの状態
	DIMOUSESTATE mouseState = {};
	// 前回の全マウスの状態
	DIMOUSESTATE mouseStatePre = {};
	// マウスカーソル表示
	bool showCursor = true;
	
	// コントローラーデバイス
	ComPtr<IDirectInputDevice8> gamePad;

	// コントローラーが接続されているか
	bool isControllerConnected = false;

	// スティックの無効範囲
	double unresponsiveRange = 100;

	// コントローラーの状態
	DIJOYSTATE gamePadState;
	// 前回のコントローラーの状態
	DIJOYSTATE gamePadStatePre;

	// 軸モードを絶対値モードとして設定



};
