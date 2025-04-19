#include "TransitionBarrier.h"

/// <summary>
/// TransitionBarrierを張る
/// </summary>
/// <param name="commandList"></param>
/// <param name="before">現在のResourceState</param>
/// <param name="after">遷移後のResourceState</param>
void TransitionBarrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources,
	D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrier{};

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	// バリアを張る対象（バックバッファ）
	barrier.Transition.pResource = swapChainResources.Get();

	// 遷移前（現在）のResourceState
	barrier.Transition.StateBefore = before;

	// 遷移後のResourceState
	barrier.Transition.StateAfter = after;

	// TransitionBarreirを張る
	commandList->ResourceBarrier(1, &barrier);
}