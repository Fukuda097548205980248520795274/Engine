#include "Input.h"

// 初期化
void Input::Initialize(HINSTANCE hInstance , HWND hWnd)
{
	// DirectInput
	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(hr));

	// キーボードデバイス
	hr = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(hr));

	// 入力データ形式のセット
	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	// 排他レベルセット
	hr = keyboard_->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));
}

// キーボード情報の取得を開始する
void Input::Acquire()
{
	keyboard_->Acquire();
	keyboard_->GetDeviceState(sizeof(keys_), keys_);
}

// 今のキーの状態をコピーする
void Input::CopyKeys()
{
	memcpy(preKeys_, keys_ , 256);
}

// Press
UINT Input::PressKeys(BYTE key)
{
	if (keys_[key])
	{
		return true;
	}

	return false;
}

// Trigger
UINT Input::TriggerKeys(BYTE key)
{
	if (!preKeys_[key] && keys_[key])
	{
		return true;
	}

	return false;
}

// Release
UINT Input::ReleaseKeys(BYTE key)
{
	if (preKeys_[key] && !keys_[key])
	{
		return true;
	}

	return false;
}