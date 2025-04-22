#include "Class/Engine/Engine.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	// エンジンの初期化
	Engine* engine = new Engine();
	engine->Initialize(1280, 720);


	/*   DirectInput   */

	// DirectInputを初期化する
	IDirectInput8* directInput_ = nullptr;
	HRESULT hr = DirectInput8Create(engine->window_->GetWndClass().hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(hr));

	// キーボードデバイスを初期化する
	IDirectInputDevice8* keyboard_ = nullptr;
	hr = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(hr));

	// 入力データ形式のセット
	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	// 排他レベルセット
	hr = keyboard_->SetCooperativeLevel(engine->window_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));




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
		{1.0f , 1.0f , 1.0f , 1.0f},
		{0.0f , -1.0f , 0.0f},
		1.0f
	};

	// 三角形
	Transform3D triangle
	{
		{1.0f , 1.0f , 1.0f},
		{0.0f , 0.0f , 0.0f},
		{0.0f , 0.0f , 0.0f},
	};

	// テクスチャ
	uint32_t ghUvChecker = engine->LoadTexture("Resources/Textures/uvChecker.png");

	// サウンド
	uint32_t shAlarm01 = engine->LoadSound("Resources/Sounds/Se/Alarm01.wav");
	uint32_t shAlarm02 = engine->LoadSound("Resources/Sounds/Se/Alarm02.wav");
	uint32_t shAlarm03 = engine->LoadSound("Resources/Sounds/Se/Alarm03.wav");

	engine->PlayerSoundWav(shAlarm01);


	// メインループ
	while(engine->IsWindowOpen())
	{
		// フレーム開始
		engine->BeginFrame();

		keyboard_->Acquire();
		BYTE key[256] = {};
		keyboard_->GetDeviceState(sizeof(key), key);


		///
		/// ↓ 更新処理ここから
		/// 

		ImGui::Begin("Triangle");
		ImGui::DragFloat3("cmaera Translate", &cameraTransform.translate.x , 0.01f);
		ImGui::DragFloat3("light normal", &light.direction.x, 0.01f);
		ImGui::End();

		Matrix4x4 viewMatrix = Make4x4InverseMatrix(Make4x4AffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate));
		Matrix4x4 projectionMatrix = Make4x4PerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
		Matrix4x4 orthoMatrix = Make4x4OrthographicsMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 100.0f);

		if (key[DIK_A])
		{
			engine->PlayerSoundWav(shAlarm02);
		}

		if (key[DIK_D])
		{
			engine->PlayerSoundWav(shAlarm03);
		}

		///
		/// ↑ 更新処理ここまで
		/// 

		///
		/// ↓ 描画処理ここから
		/// 
		
		engine->DrawTriangle(triangle, light, Multiply(viewMatrix, projectionMatrix), ghUvChecker);

		///
		/// ↑ 描画処理ここまで
		/// 

		// フレーム終了
		engine->EndFrame();

		if (key[DIK_ESCAPE])
		{
			break;
		}
	}

	// エンジンを終了する
	delete engine;

	return 0;
}