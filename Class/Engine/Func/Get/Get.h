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