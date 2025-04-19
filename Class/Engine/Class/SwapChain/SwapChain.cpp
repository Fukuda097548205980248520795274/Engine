#include "SwapChain.h"

// デストラクタ
SwapChain::~SwapChain()
{
	
}

// 初期化
void SwapChain::Initialize(HWND hwnd,
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory, Microsoft::WRL::ComPtr<ID3D12Device> device,
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
	const int32_t kClientWidth, const int32_t kClientHeight)
{

	/*-----------------------------
	    スワップチェーンを設定する
	-----------------------------*/

	// 画面のサイズ
	swapChainDesc_.Width = kClientWidth;
	swapChainDesc_.Height = kClientHeight;
	
	// 色の形式
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// マルチサンプルしない
	swapChainDesc_.SampleDesc.Count = 1;

	// 描画のターゲットとして利用する
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// ダブルバッファ
	swapChainDesc_.BufferCount = 2;

	// モニタに移したら中身を破棄
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hr = dxgiFactory->CreateSwapChainForHwnd
	(commandQueue.Get(), hwnd, &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr));
}