#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <format>
#include <fstream>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "../../Func/StringInfo/StringInfo.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")

/// <summary>
/// DXGIfactoryを取得する
/// </summary>
/// <returns></returns>
Microsoft::WRL::ComPtr<IDXGIFactory7> GetDXGIFactory();

/// <summary>
/// 使用するアダプタ（GPU）を取得する
/// </summary>
/// <param name="dxgiFactory"></param>
/// <returns></returns>
Microsoft::WRL::ComPtr<IDXGIAdapter4>  GetUseAdapter(std::ostream& os, Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory);

/// <summary>
/// Deviceを取得する
/// </summary>
/// <param name="useAdapter">使用するアダプタ（GPU）</param>
/// <returns></returns>
Microsoft::WRL::ComPtr<ID3D12Device> GetDevice(std::ostream& os, Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter);

/// <summary>
/// 
/// </summary>
/// <param name="descriptorHeap"></param>
/// <param name="descriptorSize"></param>
/// <param name="index"></param>
/// <returns></returns>
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

/// <summary>
/// 
/// </summary>
/// <param name="descriptorHeap"></param>
/// <param name="descriptorSize"></param>
/// <param name="index"></param>
/// <returns></returns>
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);