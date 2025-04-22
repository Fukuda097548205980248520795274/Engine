#include "DebugCamera.h"

// 初期化
void DebugCamera::Initialize()
{
	// ワールド行列
	Matrix4x4 worldMatrix = Make4x4AffineMatrix({ 1.0f , 1.0f ,  1.0f }, rotation_, translation_);

	// ビュー行列
	viewMatrix_ = Make4x4InverseMatrix(worldMatrix);
	projectionMatrix_ = Make4x4PerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);
}

// 更新処理
void DebugCamera::Update()
{
	// ワールド行列
	Matrix4x4 worldMatrix = Make4x4AffineMatrix({ 1.0f , 1.0f ,  1.0f }, rotation_, translation_);

	// ビュー行列
	viewMatrix_ = Make4x4InverseMatrix(worldMatrix);
}