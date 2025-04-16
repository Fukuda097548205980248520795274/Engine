#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class Commands
{
public:

	// デストラクタ
	~Commands();

	// 初期化
	void Initialize(ID3D12Device* device);

	// Getter
	ID3D12CommandQueue* GetCommandQueue() { return commandQueue_; }
	ID3D12CommandAllocator* GetCommandAllocator() { return commandAllocator_; }
	ID3D12GraphicsCommandList* GetCommandList() { return commandList_; }

private:

	// コマンドキュー
	ID3D12CommandQueue* commandQueue_ = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc_{};

	// コマンドアロケータ
	ID3D12CommandAllocator* commandAllocator_ = nullptr;

	// コマンドリスト
	ID3D12GraphicsCommandList* commandList_ = nullptr;
};

