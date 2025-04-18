#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "../../Func/StringInfo/StringInfo.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")

/// <summary>
/// DescriptorHeapを生成する
/// </summary>
/// <param name="device"></param>
/// <param name="heapType">ヒープの種類</param>
/// <param name="nunDescriptors">ディスクリプタの数</param>
/// <param name="shaderVisible"></param>
/// <returns></returns>
ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT nunDescriptors, bool  shaderVisible);

/// <summary>
/// 
/// </summary>
/// <param name="device"></param>
/// <param name="sizeInBytes"></param>
/// <returns></returns>
ID3D12Resource* CreateBufferResource(ID3D12Device* device, UINT sizeInBytes);