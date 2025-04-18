#pragma once
#include <Windows.h>
#include "../../../../externals/imgui/imgui.h"
#include "../../../../externals/imgui/imgui_impl_dx12.h"
#include "../../../../externals/imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

/// <summary>
/// ウィンドウプロシージャ
/// </summary>
/// <param name="hwnd"></param>
/// <param name="msg"></param>
/// <param name="wparam"></param>
/// <param name="lparam"></param>
/// <returns></returns>
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);