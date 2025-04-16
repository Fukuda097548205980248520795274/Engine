#include "Engine.h"

// デストラクタ
Engine::~Engine()
{
	swapChainResource_[0]->Release();
	swapChainResource_[1]->Release();

	// スワップチェーン
	delete swapChain_;

	rtvDescriptorHeap_->Release();
	
	// コマンドリスト
	delete commands_;

	device_->Release();
	useAdapter_->Release();
	dxgiFactory_->Release();

	// ウィンドウ
	delete window_;
}

// 初期化
void Engine::Initialize(const int32_t kClientWidth , const int32_t kClientHeight)
{
	// ウィンドウの生成と初期化
	window_ = new Window();
	window_->Initialize(kClientWidth,kClientHeight);


	/*-----------------------
	    DirectXを初期化する
	-----------------------*/

	// DXGIfactoryを取得する
	dxgiFactory_ = GetDXGIFactory();

	// 使用するアダプタ（GPU）を取得する
	useAdapter_ = GetUseAdapter(dxgiFactory_);

	// Deviceを取得する
	device_ = GetDevice(useAdapter_);

	// 初期化完了!!!
	Log("Complate create ID3D12Device!! \n");


	// コマンドの生成と初期化
	commands_ = new Commands();
	commands_->Initialize(device_);


	/*----------------------------
	    DescriptorHeapを生成する
	----------------------------*/

	// RTV用のディスクリプタ
	rtvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, numRtvDescriptor_, false);


	/*------------------------
	    wapChainを生成する
	------------------------*/

	// swapChainの生成と初期化
	swapChain_ = new SwapChain();
	swapChain_->Initialize(window_->GetHwnd(), dxgiFactory_, device_, commands_->GetCommandQueue(), kClientWidth, kClientHeight);


	// SwapChainからResourceを引っ張ってくる
	HRESULT hr = swapChain_->GetSwapChain()->GetBuffer(0 , IID_PPV_ARGS(&swapChainResource_[0]));
	assert(SUCCEEDED(hr));
	
	hr = swapChain_->GetSwapChain()->GetBuffer(1, IID_PPV_ARGS(&swapChainResource_[1]));
	assert(SUCCEEDED(hr));

	
	/*---------------
	    RTVを作る
	---------------*/

	// 出力結果をSRGBに変換して書き込む
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	// 2Dテクスチャ
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;


	// ディスクリプタの先頭の取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

	// 1つめ　先頭
	rtvHandles_[0] = rtvStartHandle;
	device_->CreateRenderTargetView(swapChainResource_[0], &rtvDesc_ , rtvHandles_[0]);

	// 2つめ
	rtvHandles_[1].ptr = rtvHandles_[0].ptr + device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device_->CreateRenderTargetView(swapChainResource_[1], &rtvDesc_, rtvHandles_[1]);
}

// ウィンドウが開いているかどうか
bool Engine::IsWindowOpen()
{
	if (window_->ProcessMessage())
	{
		return true;
	}

	return false;
}

// 描画前処理
void Engine::preDraw()
{
	// バックバッファのインデックスを取得する
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	// 描画先のRTVを設定する
	commands_->GetCommandList()->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, nullptr);

	// 指定した色で画面をクリアする
	float clearColor[] = { 0.1f , 0.25f , 0.5f , 1.0f };
	commands_->GetCommandList()->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);

}

// 描画後処理
void Engine::postDraw()
{
	// コマンドリストの内容を確定させる
	HRESULT hr = commands_->GetCommandList()->Close();
	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commands_->GetCommandList() };
	commands_->GetCommandQueue()->ExecuteCommandLists(1, commandLists);

	// GPUとOSに画面の交換を行うように通知する
	swapChain_->GetSwapChain()->Present(1, 0);

	// 次のフレーム用のコマンドリストを準備
	hr = commands_->GetCommandAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = commands_->GetCommandList()->Reset(commands_->GetCommandAllocator(), nullptr);
	assert(SUCCEEDED(hr));
}