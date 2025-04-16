#include "Class/Engine/Engine.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// エンジンの初期化
	Engine* engine = new Engine();
	engine->Initialize(1280, 720);


	// メインループ
	while(engine->IsWindowOpen())
	{
		///
		/// ↓ 更新処理ここから
		/// 

		///
		/// ↑ 更新処理ここまで
		/// 

		engine->preDraw();

		///
		/// ↓ 描画処理ここから
		/// 

		///
		/// ↑ 描画処理ここまで
		/// 

		engine->postDraw();
	}


	// エンジンを終了する
	delete engine;

	return 0;
}