#pragma once
#include "../../Struct.h"
#include "../../Func/Matrix/Matrix.h"

class DebugCamera
{
public:

	// 初期化
	void Initialize();

	// 更新処理
	void Update();

private:

	// 回転
	Vector3 rotation_ = { 0.0f , 0.0f , 0.0f };

	// ローカル座標
	Vector3 translation_ = { 0.0f , 0.0f , -50.0f };

	// ビュー行列
	Matrix4x4 viewMatrix_{};

	// 射影行列
	Matrix4x4 projectionMatrix_{};
};

