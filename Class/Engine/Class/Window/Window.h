#pragma once
#include <stdint.h>
#include "./Func/WindowProc/WindowProc.h"


class Window
{
public:

	// デストラクタ
	~Window();

	// 初期化
	void Initialize(const int32_t kClientWdith , const int32_t kClientHeight);

	// ウィンドウの応答処理
	int ProcessMessage();

	// Getter
	HWND GetHwnd() { return hwnd_; }
	WNDCLASS GetWndClass() { return wc_; }


private:

	// ウィンドウクラス
	WNDCLASS wc_{};

	// クライアント領域のサイズ
	int32_t clientWidth_ = 0;
	int32_t clientHeight_ = 0;

	// ウィンドウサイズ
	RECT wrc_{};

	// ウィンドハンドル
	HWND hwnd_ = nullptr;

	// メッセージ
	MSG msg_{};
};

