#include "Class/Engine/Engine.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// エンジンの初期化
	Engine* engine = new Engine();
	engine->Initialize(1280, 720);


	// カメラ
	Transform3D camera
	{
		{1.0f , 1.0f , 1.0f},
		{0.0f , 0.0f , 0.0f},
		{0.0f , 0.0f , -5.0f}
	};

	// ライト
	DirectionalLight light;
	light.color = { 1.0f , 1.0f , 1.0f , 1.0f };
	light.direction = { 0.0f , -1.0f , 0.0f };
	light.intensity = 1.0f;

	// 三角形
	Transform3D triangle
	{
		{1.0f , 1.0f , 1.0f},
		{0.0f , 0.0f , 0.0f},
		{0.0f , 0.0f , 0.0f}
	};

	// 色
	Vector3 color = { 1.0f , 1.0f , 1.0f };

	// テクスチャ
	uint32_t ghUvChecker = engine->LoadTexture("Resources/Textures/uvChecker.png");



	// メインループ
	while(engine->IsWindowOpen())
	{
		// フレーム開始
		engine->BeginFrame();

		///
		/// ↓ 更新処理ここから
		/// 

		ImGui::Begin("Triangle");
		ImGui::ColorEdit3("color", &color.x);
		ImGui::DragFloat3("scale",&triangle.scale.x , 0.01f);
		ImGui::DragFloat3("rotation",&triangle.rotate.x , 0.01f);
		ImGui::DragFloat3("translation", &triangle.translate.x, 0.01f);
		ImGui::End();

		Matrix4x4 viewMatrix = Make4x4InverseMatrix(Make4x4AffineMatrix(camera.scale, camera.rotate, camera.translate));
		Matrix4x4 projectionMatrix = Make4x4PerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);

		///
		/// ↑ 更新処理ここまで
		/// 

		///
		/// ↓ 描画処理ここから
		/// 

		engine->DrawTriangle(triangle, Multiply(viewMatrix, projectionMatrix), ghUvChecker, color);

		///
		/// ↑ 描画処理ここまで
		/// 

		// フレーム終了
		engine->EndFrame();

		if (engine->PushTriggerKeys(DIK_ESCAPE))
		{
			break;
		}
	}

	// エンジンを終了する
	delete engine;

	return 0;
}