#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include "Class/Window/Window.h"
#include "Class/ErrorDetection/ErrorDetection.h"
#include "Class/Commands/Commands.h"
#include "Class/SwapChain/SwapChain.h"
#include "Func/StringInfo/StringInfo.h"
#include "Func/Create/Create.h"
#include "Func/Get/Get.h"
#include "Func/TransitionBarrier/TransitionBarrier.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

class Engine
{
public:

	// デストラクタ
	~Engine();

	// 初期化
	void Initialize(const int32_t kClientWidth, const int32_t kClientHeight);

	// ウィンドウが開いているかどうか
	bool IsWindowOpen();

	// 描画前処理
	void preDraw();

	// 描画後処理
	void postDraw();


private:

	// ウィンドウ
	Window* window_;

	// エラーを感知するクラス
	ErrorDetection* errorDetection_;



	// DXGIファクトリ
	IDXGIFactory7* dxgiFactory_ = nullptr;

	// 使用するアダプタ（GPU）
	IDXGIAdapter4* useAdapter_ = nullptr;

	// デバイス
	ID3D12Device* device_ = nullptr;


	// コマンド
	Commands* commands_;


	// RTVのディスクリプタの数
	const UINT numRtvDescriptor_ = 2;

	// RTV用のディスクリプタ
	ID3D12DescriptorHeap* rtvDescriptorHeap_ = nullptr;


	// スワップチェーン
	SwapChain* swapChain_;

	// スワップチェーンのリソース
	ID3D12Resource* swapChainResource_[2] = { nullptr };


	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	
	// RTVディスクリプタ と スワップチェーンのリソース を紐づける
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];


	// FenceとEvent
	ID3D12Fence* fence_ = nullptr;
	uint64_t fenceValue_ = 0;
	HANDLE fenceEvent_{};
};

