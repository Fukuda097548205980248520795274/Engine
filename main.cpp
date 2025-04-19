#include "Class/Engine/Engine.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// エンジンの初期化
	Engine* engine = new Engine();
	engine->Initialize(1280, 720);

	// カメラ
	Transform3D cameraTransform =
	{
		{1.0f , 1.0f , 1.0f},
		{0.0f , 0.0f ,0.0f},
		{0.0f , 0.0f , -20.0f}
	};

	// 位置
	Vector3 pos = { 0.0f , 0.0f , 0.0f };

	// メインループ
	while(engine->IsWindowOpen())
	{
		// フレーム開始
		engine->BeginFrame();

		///
		/// ↓ 更新処理ここから
		/// 

		ImGui::Begin("Triangle");
		ImGui::DragFloat3("cmaera Translate", &cameraTransform.translate.x , 0.01f);
		ImGui::DragFloat3("pos", &pos.x, 0.01f);
		ImGui::End();


		Matrix4x4 viewMatrix = Make4x4InverseMatrix(Make4x4AffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate));
		Matrix4x4 projectionMatrix = Make4x4PerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
		Matrix4x4 orthoMatrix = Make4x4OrthographicsMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 100.0f);

		///
		/// ↑ 更新処理ここまで
		/// 

		///
		/// ↓ 描画処理ここから
		/// 

		engine->DrawSphere(pos.x, pos.y, pos.z, 1.0f, 1.0f, 1.0f, 24, 24 , Multiply(viewMatrix,projectionMatrix));

		///
		/// ↑ 描画処理ここまで
		/// 

		// フレーム終了
		engine->EndFrame();
	}


	// エンジンを終了する
	delete engine;

	return 0;
}