#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")

/// <summary>
/// TransitionBarrierを張る
/// </summary>
/// <param name="commandList"></param>
/// <param name="before">現在のResourceState</param>
/// <param name="after">遷移後のResourceState</param>
void TransitionBarrier(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList, Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources,
	D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);