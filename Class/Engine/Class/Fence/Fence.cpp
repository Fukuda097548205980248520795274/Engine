#include "Fence.h"

// デストラクタ
Fence::~Fence()
{
	CloseHandle(fenceEvent_);
}

// 初期化
void Fence::Initialize(Microsoft::WRL::ComPtr<ID3D12Device> device)
{
	//  fence
	HRESULT hr = device->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

	// FenceのSignalを待つためのEvent
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}

// GPUを待つ
void Fence::WaitForGPU(Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue)
{
	// Fenceの値を更新する
	fenceValue_++;

	// GPUがここまでたどり着いたときに、Fenceの値を、指定した値に代入するようにSignalを送る
	commandQueue->Signal(fence_.Get(), fenceValue_);

	// Fenceの値が指定したSignal値にたどり着いているか確認する
	if (fence_->GetCompletedValue() < fenceValue_)
	{
		// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);

		// イベントを待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}