#pragma once
#include <Windows.h>
#include <cassert>
#include <stdint.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib , "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


class Input
{
public:

	// 初期化
	void Initialize(HINSTANCE hInstance, HWND hWnd);

	// キーボード情報の取得を開始する
	void Acquire();

	// 今のキーの状態をコピーする
	void CopyKeys();

	// Press
	UINT PressKeys(BYTE key);

	// Trigger
	UINT TriggerKeys(BYTE key);

	// Release
	UINT ReleaseKeys(BYTE key);


private:

	// DirectInput
	IDirectInput8* directInput_ = nullptr;

	// キーボードデバイス
	IDirectInputDevice8* keyboard_ = nullptr;

	// キー
	BYTE keys_[256] = {};

	// 前押したキー
	BYTE preKeys_[256] = {};
};