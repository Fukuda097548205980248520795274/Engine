#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <format>
#include <fstream>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "../../Func/StringInfo/StringInfo.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")

/// <summary>
/// DXGIfactoryを取得する
/// </summary>
/// <returns></returns>
IDXGIFactory7* GetDXGIFactory();

/// <summary>
/// 使用するアダプタ（GPU）を取得する
/// </summary>
/// <param name="dxgiFactory"></param>
/// <returns></returns>
IDXGIAdapter4* GetUseAdapter(std::ostream& os, IDXGIFactory7* dxgiFactory);

/// <summary>
/// Deviceを取得する
/// </summary>
/// <param name="useAdapter">使用するアダプタ（GPU）</param>
/// <returns></returns>
ID3D12Device* GetDevice(std::ostream& os, IDXGIAdapter4* useAdapter);