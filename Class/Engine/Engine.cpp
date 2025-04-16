#include "Engine.h"

// デストラクタ
Engine::~Engine()
{
	device_->Release();
	useAdapter_->Release();
	dxgiFactory_->Release();
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