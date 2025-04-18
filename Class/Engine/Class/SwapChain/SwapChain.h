#pragma once
#include <stdint.h>
#include <string>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "../../Func/StringInfo/StringInfo.h"
#include "../../Func/Create/Create.h"
#include "../../Func/Get/Get.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")


class SwapChain
{
public:

	// デストラクタ
	~SwapChain();

	// 初期化
	void Initialize(HWND hwnd, IDXGIFactory7* dxgiFactory, ID3D12Device* device, ID3D12CommandQueue* commandQueue,
		const int32_t kClientWidth, const int32_t kClientHeight);

	// Getter
	IDXGISwapChain* GetSwapChain() { return swapChain_; }
	UINT GetCurrentBackBufferIndex() { return swapChain_->GetCurrentBackBufferIndex(); }
	DXGI_SWAP_CHAIN_DESC1 GetSwapChainDesc() { return swapChainDesc_; }

private:

	// スワップチェーン
	IDXGISwapChain4* swapChain_ = nullptr;

	// スワップチェーンの設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};
};

