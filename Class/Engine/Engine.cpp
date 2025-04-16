#include "Engine.h"

// デストラクタ
Engine::~Engine()
{
	device_->Release();
	useAdapter_->Release();
	dxgiFactory_->Release();
}

// 初期化
void Engine::Initialize(const int32_t kClientWidth , const int32_t kClientHeight)
{
	/*-----------------------------
	    ウィンドウクラスを登録する
	-----------------------------*/

	// ウィンドウプロシージャ
	wc_.lpfnWndProc = WindowProc;

	// ウィンドウクラス名
	wc_.lpszClassName = L"EngineClass";

	// インスタンスハンドル
	wc_.hInstance = GetModuleHandle(nullptr);

	// カーソル
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc_);


	/*---------------------------
	    ウィンドウサイズを決める
	---------------------------*/

	// クライアント領域のサイズ
	clientWidth_ = kClientWidth;
	clientHeight_ = kClientHeight;

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	wrc_ = { 0 , 0 , clientWidth_ , clientHeight_ };

	// クライアント領域を基に、実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc_, WS_OVERLAPPEDWINDOW, false);


	/*------------------------------
	    ウィンドウを生成して表示する
	------------------------------*/

	hwnd_ = CreateWindow
	(
		// 利用するクラス名
		wc_.lpszClassName,

		// タイトルバーの文字
		L"LE2A_11_フクダ_ソウワ",

		// よく見るウィンドウスタイル
		WS_OVERLAPPEDWINDOW,

		// 表示X座標
		CW_USEDEFAULT,

		// 表示Y座標
		CW_USEDEFAULT,

		// ウィンドウ幅
		wrc_.right - wrc_.left,

		// ウィンドウ高さ
		wrc_.bottom - wrc_.top,

		// 親ウィンドウハンドル
		nullptr,

		// メニューハンドル
		nullptr,

		// インスタンスハンドル
		wc_.hInstance,

		// オプション
		nullptr
	);

	// ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);


	/*-----------------------
	    DirectXを初期化する
	-----------------------*/

	// DXGIfactoryを取得する
	dxgiFactory_ = GetDXGIFactory();

	// 使用するアダプタ（GPU）を取得する
	useAdapter_ = GetUseAdapter(dxgiFactory_);

	// Deviceを取得する
	device_ = GetDevice(useAdapter_);

	// 初期化完了!!!
	Log("Complate create ID3D12Device!! \n");
}


// ウィンドウの応答処理
int Engine::ProcessMessage()
{
	// ウィンドウがxボタンを押されるまでループする
	while (msg_.message != WM_QUIT)
	{
		// Windowsにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg_, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg_);
			DispatchMessage(&msg_);
		}
		else
		{
			// ゲームの処理へ
			return true;
		}
	}

	// ゲーム終了へ
	return false;
}