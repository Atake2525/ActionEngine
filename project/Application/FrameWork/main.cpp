#include "FrameWork.h"
#include "MyGame.h"
#include <memory>


//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	std::unique_ptr<FrameWork> game = std::make_unique<MyGame>();

	// 出ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	game->Run();

	return 0;
}