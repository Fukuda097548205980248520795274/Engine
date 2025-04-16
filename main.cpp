#include "Class/Engine/Engine.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// エンジンの初期化
	Engine* engine = new Engine();
	engine->Initialize(1280, 720);


	// メインループ
	while(engine->IsWindowOpen())
	{
		
	}


	// エンジンを終了する
	delete engine;

	return 0;
}