#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")


class Fence
{
public:

	// デストラクタ
	~Fence();

	// 初期化
	void Initialize(ID3D12Device* device);

	// GPUを待つ
	void WaitForGPU(ID3D12CommandQueue* commandQueue);


private:

	// フェンス
	ID3D12Fence* fence_ = nullptr;

	// フェンスが指定した値
	uint64_t fenceValue_ = 0;

	// イベント
	HANDLE fenceEvent_{};
};

