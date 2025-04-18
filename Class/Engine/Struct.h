#pragma once

	// 3+1次元ベクトル
	typedef struct Vector4
	{
		float x;
		float y;
		float z;
		float w;
	}Vector4;

	// 3次元ベクトル
	typedef struct Vector3
	{
		float x;
		float y;
		float z;
	}Vector3;

	// 2次元ベクトル
	typedef struct Vector2
	{
		float x;
		float y;
	}Vector2;

	// 4x4行列
	typedef struct Matrix4x4
	{
		float m[4][4];
	}Matrix4x4;

	// 姿勢情報
	typedef struct Transform3D
	{
		// 拡縮
		Vector3 scale;

		// 回転
		Vector3 rotate;

		// 移動
		Vector3 translate;

	}Transform3D;