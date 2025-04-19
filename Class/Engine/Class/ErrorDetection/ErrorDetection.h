#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "../../Func/StringInfo/StringInfo.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class ErrorDetection
{
public:

	// デストラクタ
	~ErrorDetection();

	// 初期化
	void Initialize();

	// エラーで停止させる
	void MakeItStop(Microsoft::WRL::ComPtr<ID3D12Device> device);


private:

	// デバッグコントローラ
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController_ = nullptr;
	
};

