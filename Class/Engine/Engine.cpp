#include "Engine.h"

// 初期化
void Engine::Initialize(const int32_t kClientWidth , const int32_t kClientHeight)
{
	/*-----------------------------
	    ウィンドウクラスを登録する
	-----------------------------*/

	// ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;

	// ウィンドウクラス名
	wc.lpszClassName = L"EngineClass";

	// インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);

	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc);


	/*---------------------------
	    ウィンドウサイズを決める
	---------------------------*/

	// クライアント領域のサイズ
	clientWidth = kClientWidth;
	clientHeight = kClientHeight;

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	wrc = { 0 , 0 , clientWidth , clientHeight };

	// クライアント領域を基に、実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);


	/*------------------------------
	    ウィンドウを生成して表示する
	------------------------------*/

	hwnd = CreateWindow
	(
		// 利用するクラス名
		wc.lpszClassName,

		// タイトルバーの文字
		L"LE2A_11_フクダ_ソウワ",

		// よく見るウィンドウスタイル
		WS_OVERLAPPEDWINDOW,

		// 表示X座標
		CW_USEDEFAULT,

		// 表示Y座標
		CW_USEDEFAULT,

		// ウィンドウ幅
		wrc.right - wrc.left,

		// ウィンドウ高さ
		wrc.bottom - wrc.top,

		// 親ウィンドウハンドル
		nullptr,

		// メニューハンドル
		nullptr,

		// インスタンスハンドル
		wc.hInstance,

		// オプション
		nullptr
	);

	// ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);
}


// ウィンドウの応答処理
int Engine::ProcessMessage()
{
	// ウィンドウがxボタンを押されるまでループする
	while (msg.message != WM_QUIT)
	{
		// Windowsにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
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