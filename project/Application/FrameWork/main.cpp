#include "FrameWork.h"
#include "MyGame.h"



//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	FrameWork* game = new MyGame();

	// 出ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	game->Run();

	delete game;

	return 0;
}