#include "Input.h"
#include <cassert>
#include "WinApp.h"
#include <cmath>
#include "ImGuiManager.h"
#include "kMath.h"
#include "Logger.h"

using namespace Logger;

Input* Input::instance = nullptr;

Input* Input::GetInstance() {
	if (instance == nullptr) {
		instance = new Input;
	}
	return instance;
}

void Input::Finalize() {
	delete instance;
	instance = nullptr;
}

void Input::Initialize() {
	HRESULT result;

	// DirectInputのインスタンス生成 キーボード
	result = DirectInput8Create(WinApp::GetInstance()->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	result = DirectInput8Create(WinApp::GetInstance()->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInputMouse, nullptr);
	assert(SUCCEEDED(result));

	result = DirectInput8Create(WinApp::GetInstance()->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInputGamePad, nullptr);
	assert(SUCCEEDED(result));

	CreateKeyboardDevice();
	CreateMouseDevice();
	CreateControllerDevice();

}

void Input::UpdateDevice() {
	CreateKeyboardDevice();
	CreateMouseDevice();
	CreateControllerDevice();
}

void Input::CreateKeyboardDevice() {
	bool success = false;
	HRESULT result;
	// DirectInputの初期化
	// マウスデバイスの生成
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	if (FAILED(result))
	{
		Log("キーボードの接続に失敗\n");
	}
	else
	{
		success = true;
	}
	assert(SUCCEEDED(result));
	// 入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	// 排他制御レベルセット
	result = keyboard->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
	Log("キーボードの接続に成功\n");
}

void Input::CreateMouseDevice() {
	HRESULT result;
	bool success = false;
	// キーボードデバイスの生成
	result = directInputMouse->CreateDevice(GUID_SysMouse, &mouse, NULL);
	if (FAILED(result))
	{
		Log("マウスの接続に失敗\n");
	}
	else
	{
		success = true;
	}
	assert(SUCCEEDED(result));
	// 入力データ形式のセット
	result = mouse->SetDataFormat(&c_dfDIMouse);
	// 排他制御レベルセット
	result = mouse->SetCooperativeLevel(WinApp::GetInstance()->GetHwnd(), DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	assert(SUCCEEDED(result));
	if (success)
	{
		Log("マウスの接続に成功\n");
	}
}

void Input::CreateControllerDevice() {
	HRESULT result;


	// ゲームパッドデバイスの生成
	result = directInputGamePad->CreateDevice(GUID_Joystick, &gamePad, NULL);
	if (FAILED(result))
	{
		isControllerConnected = false;
		Log("コントローラーの接続に失敗\n");
	}
	else
	{
		isControllerConnected = true;
		Log("コントローラーの接続に成功\n");
	}
	if (isControllerConnected)
	{
		// 入力データ形式のセット
		result = gamePad->SetDataFormat(&c_dfDIJoystick);
		assert(SUCCEEDED(result));

		// 軸モード設定
		DIPROPDWORD diprop;
		ZeroMemory(&diprop, sizeof(diprop));
		diprop.diph.dwSize = sizeof(diprop);
		diprop.diph.dwHeaderSize = sizeof(diprop.diph);
		diprop.diph.dwHow = DIPH_DEVICE;
		diprop.diph.dwObj = 0;
		diprop.dwData = DIPROPAXISMODE_ABS; // 絶対値モードの指定(DIPROPAXISMODE_RELにしたら相対値)

		// 軸モードを変更
		result = gamePad->SetProperty(DIPROP_AXISMODE, &diprop.diph);

		// X軸の値の範囲設定
		DIPROPRANGE diprg;
		ZeroMemory(&diprg, sizeof(diprg));
		diprg.diph.dwSize = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwHow = DIPH_BYOFFSET;
		diprg.diph.dwObj = DIJOFS_X;
		diprg.lMin = -1000;
		diprg.lMax = 1000;

		result = gamePad->SetProperty(DIPROP_RANGE, &diprg.diph);

		// Y軸の値の範囲設定
		diprg.diph.dwObj = DIJOFS_Y;

		result = gamePad->SetProperty(DIPROP_RANGE, &diprg.diph);

		// Y軸の値の範囲設定
		diprg.diph.dwObj = DIJOFS_Z;

		result = gamePad->SetProperty(DIPROP_RANGE, &diprg.diph);

		// RX軸の値の範囲設定
		diprg.diph.dwObj = DIJOFS_RX;

		result = gamePad->SetProperty(DIPROP_RANGE, &diprg.diph);

		// RY軸の値の範囲設定
		diprg.diph.dwObj = DIJOFS_RY;

		result = gamePad->SetProperty(DIPROP_RANGE, &diprg.diph);
	}
}

void Input::ShowMouseCursor(bool flag) {
	if (flag)
	{
		mouse->Unacquire();
	}
	showCursor = flag;
}

void Input::Update() {
	HRESULT result;

	// 前回のキー入力を保存
	memcpy(keyPres, keys, sizeof(keys));
	// キーボード情報の取得開始
	result = keyboard->Acquire();
	// 全キーの入力情報を取得する
	result = keyboard->GetDeviceState(sizeof(keys), keys);

	if (!showCursor)
	{
		mouseStatePre = mouseState;
		// マウスの状態の取得
		result = mouse->Acquire();
		// ポーリング開始
		result = mouse->Poll();
		// 全ボタンの入力情報を取得する
		result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
	}

	if (isControllerConnected)
	{
		// 前回のコントローラーの入力を保存
		gamePadStatePre = gamePadState;
		// コントローラーの状態取得開始
		result = gamePad->Acquire();
		// ポーリング開始
		result = gamePad->Poll();
		// 入力情報を取得
		result = gamePad->GetDeviceState(sizeof(DIJOYSTATE), &gamePadState);
	}

}

bool Input::PushKey(BYTE keyNumber) {
	if (keys[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::TriggerKey(BYTE keyNumber) {
	if (keys[keyNumber] && !keyPres[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::ReturnKey(BYTE keyNumber) {
	if (!keys[keyNumber] && keyPres[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::PressMouse(int mouseNumber) {
	if (mouseState.rgbButtons[mouseNumber] && (0x80))
	{
		return true;
	}
	return false;
}

bool Input::TriggerMouse(int mouseNumber) {
	if (mouseState.rgbButtons[mouseNumber] && !mouseStatePre.rgbButtons[mouseNumber] && (0x80))
	{
		return true;
	}
	return false;
}

bool Input::ReturnMouse(int mouseNumber) {
	if (!mouseState.rgbButtons[mouseNumber] && mouseStatePre.rgbButtons[mouseNumber] && (0x80))
	{
		return true;
	}
	return false;
}

Vector2 Input::GetMousePos2() {
	POINT pos;
	GetCursorPos(&pos);
	Vector2 result = { float(pos.x), float(pos.y) };
	return result;
}

Vector3 Input::GetMousePos3() {
	POINT pos;
	GetCursorPos(&pos);
	Vector3 result = { float(pos.x), float(pos.y), 0.0f };
	return result;
}

Vector2 Input::GetWindowMousePos2() {
	Vector2 pos = GetMousePos2();
	Vector2 mousePos = { 0.0f, 0.0f };
	if (WinApp::GetInstance()->windowMode == WindowMode::Window)
	{
		Vector3 window = WinApp::GetInstance()->GetWindowAABB().min;
		mousePos = { pos.x - window.x - 8.0f, pos.y - window.y - 30.0f };
	}
	else
	{
		Vector3 window = WinApp::GetInstance()->GetWindowAABB().min;
		mousePos = { pos.x - window.x, pos.y - window.y };
	}
	return mousePos;
}

Vector3 Input::GetWindowMousePos3() {
	Vector3 pos = GetMousePos3();
	Vector3 mousePos = { 0.0f, 0.0f, 0.0f };
	if (WinApp::GetInstance()->windowMode == WindowMode::Window)
	{
		Vector3 window = WinApp::GetInstance()->GetWindowAABB().min;
		mousePos = {pos.x - window.x - 8.0f, pos.y - window.y - 30.0f};
	}
	else
	{
		Vector3 window = WinApp::GetInstance()->GetWindowAABB().min;
		mousePos = { pos.x - window.x, pos.y - window.y };
	}
	return mousePos;
}

Vector2 Input::GetMouseVel2() {
	Vector2 result = { static_cast<float>(mouseState.lX), static_cast<float>(mouseState.lY) };
	return result;
}

Vector3 Input::GetMouseVel3() {
	Vector3 result = { static_cast<float>(mouseState.lX), static_cast<float>(mouseState.lY), static_cast<float>(mouseState.lZ) };
	return result;
}

Vector2 Input::GetLeftJoyStickPos2(const float deadZone) {
	// スティックの無効範囲とデッドゾーンを考慮してresultと返す
	Vector2 result = { 0.0f, 0.0f };

	if (gamePadState.lY < -unresponsiveRange - deadZone || gamePadState.lY > unresponsiveRange + deadZone)
	{
		if (gamePadState.lX < -unresponsiveRange - deadZone)
		{
			result.x = static_cast<float>(gamePadState.lX);
		}
		else if (gamePadState.lX > unresponsiveRange + deadZone)
		{
			result.x = static_cast<float>(gamePadState.lX);
		}
	}

	if (gamePadState.lX < -unresponsiveRange - deadZone || gamePadState.lX > unresponsiveRange + deadZone)
	{
		if (gamePadState.lY < -unresponsiveRange - deadZone)
		{
			result.y = static_cast<float>(gamePadState.lY);
		}
		else if (gamePadState.lY > unresponsiveRange + deadZone)
		{
			result.y = static_cast<float>(gamePadState.lY);
		}
	}
	return result;
}

Vector3 Input::GetLeftJoyStickPos3(const float deadZone) {
	Vector3 result = { 0.0f, 0.0f, 0.0f };
	// スティックの無効範囲とデッドゾーンを考慮してresultと返す
	if (gamePadState.lY < -unresponsiveRange - deadZone || gamePadState.lY > unresponsiveRange - deadZone)
	{
		result.x = static_cast<float>(gamePadState.lX);
	}
	else
	{
		if (gamePadState.lX < -unresponsiveRange - deadZone)
		{
			result.x = static_cast<float>(gamePadState.lX);
		}
		else if (gamePadState.lX > unresponsiveRange + deadZone)
		{
			result.x = static_cast<float>(gamePadState.lX);
		}
	}

	if (gamePadState.lX < -unresponsiveRange - deadZone || gamePadState.lX > unresponsiveRange + deadZone)
	{
		result.y = static_cast<float>(gamePadState.lY);
	}
	else
	{
		if (gamePadState.lY < -unresponsiveRange - deadZone)
		{
			result.y = static_cast<float>(gamePadState.lY);
		}
		else if (gamePadState.lY > unresponsiveRange + deadZone)
		{
			result.y = static_cast<float>(gamePadState.lY);
		}
	}

	if (gamePadState.lZ < -unresponsiveRange)
	{
		result.z = static_cast<float>(gamePadState.lZ);
	}
	else if (gamePadState.lZ > unresponsiveRange)
	{
		result.z = static_cast<float>(gamePadState.lZ);
	}
	return result;
}

Vector2 Input::GetRightJoyStickPos2(const float deadZone) {
	Vector2 result = { 0.0f, 0.0f };
	// スティックの無効範囲とデッドゾーンを考慮してresultと返す
	if (gamePadState.lRx < -unresponsiveRange - deadZone)
	{
		result.x = static_cast<float>(gamePadState.lRx);
	}
	else if (gamePadState.lRx > unresponsiveRange + deadZone)
	{
		result.x = static_cast<float>(gamePadState.lRx);
	}

	if (gamePadState.lRy < -unresponsiveRange - deadZone)
	{
		result.y = static_cast<float>(gamePadState.lRy);
	}
	else if (gamePadState.lRy > unresponsiveRange + deadZone)
	{
		result.y = static_cast<float>(gamePadState.lRy);
	}
	return result;
}

Vector3 Input::GetRightJoyStickPos3(const float deadZone) {
	Vector3 result = { 0.0f, 0.0f, 0.0f };
	if (gamePadState.lRx < -unresponsiveRange - deadZone)
	{
		result.x = static_cast<float>(gamePadState.lRx);
	}
	else if (gamePadState.lRx > unresponsiveRange + deadZone)
	{
		result.x = static_cast<float>(gamePadState.lRx);
	}

	if (gamePadState.lRy < -unresponsiveRange - deadZone)
	{
		result.y = static_cast<float>(gamePadState.lRy);
	}
	else if (gamePadState.lRy > unresponsiveRange + deadZone)
	{
		result.y = static_cast<float>(gamePadState.lRy);
	}

	if (gamePadState.lRz < -unresponsiveRange)
	{
		result.z = static_cast<float>(gamePadState.lRz);
	}
	else if (gamePadState.lRz > unresponsiveRange)
	{
		result.z = static_cast<float>(gamePadState.lRz);
	}
	return result;
}

float Input::GetJoyStickDirection2(const Vector2 joyStickPos)
{
	// 角度を求める
	Vector2 result = Normalize(joyStickPos);
	result.y *= -1.0f;

	if (result.x == 0.0f && result.y == 0.0f)
	{
		return 0.0f;
	}

	return std::atan2(result.x, result.y);
}

float Input::GetJoyStickDirection3(const Vector3 joyStickPos)
{
	// 角度を求める
	Vector3 result = Normalize(joyStickPos);
	result.y *= -1.0f;

	if (result.x == 0.0f && result.y == 0.0f)
	{
		return 0.0f;
	}
	return std::atan2(result.x, result.y);
}

Vector2 Input::GetJoyStickVelocity(const Vector2 joyStickPos, const Vector3 velocity, const bool acceleration)
{
	Vector2 joy = { std::fabs(joyStickPos.x), std::fabs(joyStickPos.y)};
	// スティックの傾きの合計が1000以上(上限速度)なら合計を1000になるようにする
	if (acceleration)
	{
		float stick = joy.x + joy.y;
		float overAmount;
		if (stick > 1000.0f)
		{
			overAmount = stick - 1000.0f;
			joy.x -= overAmount / 2.0f;
			joy.y -= overAmount / 2.0f;
		}
	}
	// 角度を求める
	float rot = GetJoyStickDirection2(joyStickPos);

	Matrix4x4 rotateMatrix = Multiply(Multiply(MakeRotateXMatrix(0.0f), MakeRotateYMatrix(rot)), MakeRotateZMatrix(0.0f));
	Vector3 vel = TransformNormal(velocity, rotateMatrix);
	if (acceleration)
	{
		// xとyを絶対値に変換した状態で足す
		float len = std::fabs(joy.x) + std::fabs(joy.y);
		if (len > 1000.0f)
		{
			len = 1000.0f;
		}
		// lenを1000(ジョイスティックの最大値)で割る
		len = len / 1000.0f;
		// lenをvelocityに掛ける
		vel = vel * len;
	}
	Vector2 velo = { vel.y, vel.z };
	return velo;
}

Vector3 Input::GetJoyStickVelocity(const Vector3 joyStickPos, const Vector3 velocity, const bool acceleration)
{
	Vector3 joy = {std::fabs(joyStickPos.x), std::fabs(joyStickPos.y), std::fabs(joyStickPos.z)};
	// スティックの傾きの合計が1000以上(上限速度)なら合計を1000になるようにする
	if (acceleration)
	{
		float stick = joy.x + joy.y;
		float overAmount;
		if (stick > 1000.0f)
		{
			overAmount = stick - 1000.0f;
			joy.x -= overAmount / 2.0f;
			joy.y -= overAmount / 2.0f;
		}
	}
	// 角度を求める
	float rot = GetJoyStickDirection3(joyStickPos);

	Matrix4x4 rotateMatrix = Multiply(Multiply(MakeRotateXMatrix(0.0f), MakeRotateYMatrix(rot)), MakeRotateZMatrix(0.0f));
	Vector3 vel = TransformNormal(velocity, rotateMatrix);
	if (acceleration)
	{
		// xとyを絶対値に変換した状態で足す
		float len = joy.x + joy.y;
		// lenを1000(ジョイスティックの最大値)で割る
		if (len > 1000.0f)
		{
			len = 1000.0f;
		}
		len = len / 1000.0f;
		// lenをvelocityに掛ける
		vel = vel * len;
	}
	return vel;
}

float Input::GetJoyStickLength(const Vector2 joyStickPos)
{
	Vector2 joy = { std::fabs(joyStickPos.x), std::fabs(joyStickPos.y) };
	// スティックの傾きの合計が1000以上(上限速度)なら合計を1000になるようにする
	float stick = joy.x + joy.y;
	float overAmount;
	if (stick > 1000.0f)
	{
		overAmount = stick - 1000.0f;
		joy.x -= overAmount / 2.0f;
		joy.y -= overAmount / 2.0f;
	}
	return (joy.x + joy.y) / 1000.0f;
}

float Input::GetJoyStickLength(const Vector3 joyStickPos)
{
	Vector3 joy = { std::fabs(joyStickPos.x), std::fabs(joyStickPos.y), std::fabs(joyStickPos.z) };
	// スティックの傾きの合計が1000以上(上限速度)なら合計を1000になるようにする
	float stick = joy.x + joy.y;
	float overAmount;
	if (stick > 1000.0f)
	{
		overAmount = stick - 1000.0f;
		joy.x -= overAmount / 2.0f;
		joy.y -= overAmount / 2.0f;
	}
	return (joy.x + joy.y) / 1000.0f;
}

bool Input::IsMoveLeftJoyStick() {
	if (gamePadState.lX < -unresponsiveRange)
	{
		return true;
	}
	else if (gamePadState.lX > unresponsiveRange)
	{
		return true;
	}

	if (gamePadState.lY < -unresponsiveRange)
	{
		return true;
	}
	else if (gamePadState.lY > unresponsiveRange)
	{
		return true;
	}
	//if (gamePadState.lX != 0 || gamePadState.lY != 0)
	//{
	//	return true;
	//}
	return false;
}

bool Input::IsMoveRightJoyStick() {
	if (gamePadState.lRx < -unresponsiveRange)
	{
		return true;
	}
	else if (gamePadState.lRx > unresponsiveRange)
	{
		return true;
	}

	if (gamePadState.lRy < -unresponsiveRange)
	{
		return true;
	}
	else if (gamePadState.lRy > unresponsiveRange)
	{
		return true;
	}
	//if (gamePadState.lRx != 0 || gamePadState.lRy != 0)
	//{
	//	return true;
	//}
	return false;
}

bool Input::PushXButton(DPad dPad) {
	DPad result = DPad::None;
	switch (gamePadState.rgdwPOV[0])
	{
	case 0: // 上
		result = DPad::Up;
		break;

	case 18000: // 下
		result = DPad::Down;
		break;

	case 9000: // 右
		result = DPad::Right;
		break;

	case 27000: // 左
		result = DPad::Left;
		break;

	case 4500: // 右上
		result = DPad::UpRight;
		break;
		
	case 31500: // 左上
		result = DPad::UpLeft;
		break;

	case 13500: // 右下
		result = DPad::DownRight;
		break;

	case 22500: // 左下
		result = DPad::DownLeft;
		break;

	}


	if (dPad == result)
	{
		return true;
	}
	return false;
}

bool Input::TriggerXButton(DPad dPad) {
	DPad result = DPad::None;
	switch (gamePadState.rgdwPOV[0])
	{
	case 0: // 上
		result = DPad::Up;
		break;

	case 18000: // 下
		result = DPad::Down;
		break;

	case 9000: // 右
		result = DPad::Right;
		break;

	case 27000: // 左
		result = DPad::Left;
		break;

	case 4500: // 右上
		result = DPad::UpRight;
		break;

	case 31500: // 左上
		result = DPad::UpLeft;
		break;

	case 13500: // 右下
		result = DPad::DownRight;
		break;

	case 22500: // 左下
		result = DPad::DownLeft;
		break;

	}

	DPad resultPre = DPad::None;

	switch (gamePadStatePre.rgdwPOV[0])
	{
	case 0: // 上
		resultPre = DPad::Up;
		break;

	case 18000: // 下
		resultPre = DPad::Down;
		break;

	case 9000: // 右
		resultPre = DPad::Right;
		break;

	case 27000: // 左
		resultPre = DPad::Left;
		break;

	case 4500: // 右上
		resultPre = DPad::UpRight;
		break;

	case 31500: // 左上
		resultPre = DPad::UpLeft;
		break;

	case 13500: // 右下
		resultPre = DPad::DownRight;
		break;

	case 22500: // 左下
		resultPre = DPad::DownLeft;
		break;

	}


	if (dPad == result && dPad != resultPre)
	{
		return true;
	}
	return false;
}

bool Input::PushButton(Controller button) {
	Controller result = Controller::None;
	for (int i = 0; i < 10; i++)
	{
		if (!gamePadState.rgbButtons[i] && 0x80)
		{
			continue;
		}

		switch (i)
		{
		case 0:
			result = Controller::A;
			if (button == result)
			{
				return true;
			}
				break;
		case 1:
			result = Controller::B;
			if (button == result)
			{
				return true;
			}
			break;
		case 2:
			result = Controller::X;
			if (button == result)
			{
				return true;
			}
			break;
		case 3:
			result = Controller::Y;
			if (button == result)
			{
				return true;
			}
			break;
		case 4:
			result = Controller::LB;
			if (button == result)
			{
				return true;
			}
			break;
		case 5:
			result = Controller::RB;
			if (button == result)
			{
				return true;
			}
			break;
		case 6:
			result = Controller::View;
			if (button == result)
			{
				return true;
			}
			break;
		case 7:
			result = Controller::Menu;
			if (button == result)
			{
				return true;
			}
			break;
		case 8:
			result = Controller::LeftStick;
			if (button == result)
			{
				return true;
			}
			break;
		case 9:
			result = Controller::RightStick;
			if (button == result)
			{
				return true;
			}
			break;
		}
	}
	Vector3 joystick = GetLeftJoyStickPos3(0.0f);
	if (joystick.z < 0)
	{
		result = Controller::RT;
		if (button == result)
		{
			return true;
		}
	}
	else if (joystick.z > 0)
	{
		result = Controller::LT;
		if (button == result)
		{
			return true;
		}
	}
	return false;
}

bool Input::TriggerButton(Controller button) {
	Controller result = Controller::None;
	Controller resultPre = Controller::None;
	for (int i = 0; i < 10; i++)
	{
		if (!gamePadState.rgbButtons[i] && 0x80)
		{
			continue;
		}

		switch (i)
		{
		case 0:
			result = Controller::A;
			break;
		case 1:
			result = Controller::B;
			break;
		case 2:
			result = Controller::X;
			break;
		case 3:
			result = Controller::Y;
			break;
		case 4:
			result = Controller::LB;
			break;
		case 5:
			result = Controller::RB;
			break;
		case 6:
			result = Controller::View;
			break;
		case 7:
			result = Controller::Menu;
			break;
		case 8:
			result = Controller::LeftStick;
			break;
		case 9:
			result = Controller::RightStick;
			break;
		}
	}

	for (int i = 0; i < 10; i++)
	{
		if (!gamePadStatePre.rgbButtons[i] && 0x80)
		{
			continue;
		}

		switch (i)
		{
		case 0:
			resultPre = Controller::A;
			break;
		case 1:
			resultPre = Controller::B;
			break;
		case 2:
			resultPre = Controller::X;
			break;
		case 3:
			resultPre = Controller::Y;
			break;
		case 4:
			resultPre = Controller::LB;
			break;
		case 5:
			resultPre = Controller::RB;
			break;
		case 6:
			resultPre = Controller::View;
			break;
		case 7:
			resultPre = Controller::Menu;
			break;
		case 8:
			resultPre = Controller::LeftStick;
			break;
		case 9:
			resultPre = Controller::RightStick;
			break;
		}
	}

	if (result == button && resultPre != button)
	{
		return true;
	}

	Vector3 joystick = GetLeftJoyStickPos3(0.0f);
	float joystickPre = 0.0f;
	if (gamePadStatePre.lZ < -unresponsiveRange)
	{
		joystickPre = static_cast<float>(gamePadStatePre.lZ);
	}
	else if (gamePadStatePre.lZ > unresponsiveRange)
	{
		joystickPre = static_cast<float>(gamePadStatePre.lZ);
	}

	if (joystick.z < 0 && joystickPre >= 0)
	{
		result = Controller::RT;
		if (button == result)
		{
			return true;
		}
	}
	else if (joystick.z > 0 && joystickPre <= 0)
	{
		result = Controller::LT;
		if (button == result)
		{
			return true;
		}
	}
	return false;
}

bool Input::ReturnButton(Controller button) {
	Controller result = Controller::None;
	Controller resultPre = Controller::None;
	for (int i = 0; i < 10; i++)
	{
		if (!gamePadState.rgbButtons[i] && 0x80)
		{
			continue;
		}

		switch (i)
		{
		case 0:
			result = Controller::A;
			break;
		case 1:
			result = Controller::B;
			break;
		case 2:
			result = Controller::X;
			break;
		case 3:
			result = Controller::Y;
			break;
		case 4:
			result = Controller::LB;
			break;
		case 5:
			result = Controller::RB;
			break;
		case 6:
			result = Controller::View;
			break;
		case 7:
			result = Controller::Menu;
			break;
		case 8:
			result = Controller::LeftStick;
			break;
		case 9:
			result = Controller::RightStick;
			break;
		}
	}

	for (int i = 0; i < 10; i++)
	{
		if (!gamePadStatePre.rgbButtons[i] && 0x80)
		{
			continue;
		}

		switch (i)
		{
		case 0:
			resultPre = Controller::A;
			break;
		case 1:
			resultPre = Controller::B;
			break;
		case 2:
			resultPre = Controller::X;
			break;
		case 3:
			resultPre = Controller::Y;
			break;
		case 4:
			resultPre = Controller::LB;
			break;
		case 5:
			resultPre = Controller::RB;
			break;
		case 6:
			resultPre = Controller::View;
			break;
		case 7:
			resultPre = Controller::Menu;
			break;
		case 8:
			resultPre = Controller::LeftStick;
			break;
		case 9:
			resultPre = Controller::RightStick;
			break;
		}
	}

	if (result != button && resultPre == button)
	{
		return true;
	}

	Vector3 joystick = GetLeftJoyStickPos3(0.0f);
	float joystickPre = 0.0f;
	if (gamePadStatePre.lZ < -unresponsiveRange)
	{
		joystickPre = static_cast<float>(gamePadStatePre.lZ);
	}
	else if (gamePadStatePre.lZ > unresponsiveRange)
	{
		joystickPre = static_cast<float>(gamePadStatePre.lZ);
	}

	if (joystick.z >= 0 && joystickPre < 0)
	{
		result = Controller::RT;
		if (button == result)
		{
			return true;
		}
	}
	else if (joystick.z <= 0 && joystickPre > 0)
	{
		result = Controller::LT;
		if (button == result)
		{
			return true;
		}
	}
	return false;
}