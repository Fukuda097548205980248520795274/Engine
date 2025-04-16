#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include "Func/WindowProc/WindowProc.h"

class Engine
{
public:

	// 初期化
	void Initialize(const int32_t kClientWidth, const int32_t kClientHeight);

	// ウィンドウの応答処理
	int ProcessMessage();


private:

	// ウィンドウクラス
	WNDCLASS wc{};

	// クライアント領域のサイズ
	int32_t clientWidth = 0;
	int32_t clientHeight = 0;

	// ウィンドウサイズ
	RECT wrc{};

	// ウィンドハンドル
	HWND hwnd = nullptr;

	// メッセージ
	MSG msg{};
};

