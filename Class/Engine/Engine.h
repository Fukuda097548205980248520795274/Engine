#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "Class/Window/Window.h"
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

	// ウィンドウが開いているかどうか
	bool IsWindowOpen();


private:

	// ウィンドウ
	Window* window_;


	/*   DirectX   */

	// DXGIファクトリ
	IDXGIFactory7* dxgiFactory_ = nullptr;

	// 使用するアダプタ（GPU）
	IDXGIAdapter4* useAdapter_ = nullptr;

	// デバイス
	ID3D12Device* device_ = nullptr;
};

