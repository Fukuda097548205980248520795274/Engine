#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "Func/WindowProc/WindowProc.h"
#include "Func/StringInfo/StringInfo.h"
#include "Func/Create/Create.h"
#include "Func/Get/Get.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Engine
{
public:

	// デストラクタ
	~Engine();

	// 初期化
	void Initialize(const int32_t kClientWidth, const int32_t kClientHeight);

	// ウィンドウの応答処理
	int ProcessMessage();


private:

	/*   ウィンドウ表示   */

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


	/*   DirectX   */

	// DXGIファクトリ
	IDXGIFactory7* dxgiFactory_ = nullptr;

	// 使用するアダプタ（GPU）
	IDXGIAdapter4* useAdapter_ = nullptr;

	// デバイス
	ID3D12Device* device_ = nullptr;
};

