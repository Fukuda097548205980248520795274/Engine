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
		{0.0f , 0.0f , -10.0f}
	};

	// 光源
	DirectionalLight light
	{
		{0.0f , 0.0f , 1.0f , 1.0f},
		{0.0f , -1.0f , 0.0f},
		1.0f
	};

	// スプライト
	Transform3D sprite
	{
		{1.0f , 1.0f , 1.0f},
		{0.0f , 0.0f , 0.0f},
		{0.0f , 0.0f , 0.0f}
	};

	// 三角形
	Transform3D triangle
	{
		{1.0f , 1.0f , 1.0f},
		{0.0f , 0.0f , 0.0f},
		{0.0f , 0.0f , 0.0f}
	};

	// 球
	Transform3D sphere
	{
		{1.0f , 1.0f , 1.0f},
		{0.0f , 0.0f , 0.0f},
		{0.0f , 0.0f , 0.0f}
	};

	// 分割数
	int subdivisions = 24;

	uint32_t ghUvChecker = engine->LoadTexture("Resources/uvChecker.png");
	uint32_t ghMonsterBall = engine->LoadTexture("Resources/monsterBall.png");
	uint32_t ghWhite = engine->LoadTexture("Resources/white.png");


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
		ImGui::DragFloat3("light normal", &light.direction.x, 0.01f);
		ImGui::DragFloat3("sphere scale", &sphere.scale.x, 0.01f);
		ImGui::DragFloat3("sphere rotate", &sphere.rotate.x, 0.01f);
		ImGui::DragFloat3("sphere translate", &sphere.translate.x, 0.01f);
		ImGui::DragInt("lat", &subdivisions, 1.0f, 6, 24);
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
		
		engine->DrawSphere(subdivisions, sphere, Multiply(viewMatrix, projectionMatrix),light, ghMonsterBall);
		engine->DrawTriangle(triangle, light, Multiply(viewMatrix, projectionMatrix), ghWhite);
		engine->DrawSprite(0.0f, 0.0f, 320.0f, 0.0f, 0.0f, 180.0f, 320.0f, 180.0f, sprite, Multiply(viewMatrix, orthoMatrix), ghUvChecker);

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