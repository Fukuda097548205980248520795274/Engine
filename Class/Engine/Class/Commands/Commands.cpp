#include "Commands.h"

// デストラクタ
Commands::~Commands()
{
	commandList_->Release();
	commandAllocator_->Release();
	commandQueue_->Release();
}

// 初期化
void Commands::Initialize(ID3D12Device* device)
{
	// コマンドキュー
	HRESULT hr = device->CreateCommandQueue(&commandQueueDesc_, IID_PPV_ARGS(&commandQueue_));
	assert(SUCCEEDED(hr));

	// コマンドアロケータ
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	assert(SUCCEEDED(hr));

	// コマンドリスト
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_, nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hr));
}