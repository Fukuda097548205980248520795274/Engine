#pragma once
#include <cassert>
#define _USE_MATH_DEFINES
#include <cmath>
#include "../../Struct.h"

/// <summary>
/// 座標変換を行う
/// </summary>
/// <param name="vecotr">ベクトル</param>
/// <param name="matrix">行列</param>
/// <returns>変換した座標</returns>
Vector3 Transform(Vector3 vector, Matrix4x4 matrix);

/// <summary>
/// 行列の積を求める
/// </summary>
/// <param name="m1">行列1</param>
/// <param name="m2">行列2</param>
/// <returns>行列の積</returns>
Matrix4x4 Multiply(Matrix4x4 m1, Matrix4x4 m2);

/// <summary>
/// 単位行列を作る
/// </summary>
/// <returns>単位行列</returns>
Matrix4x4 Make4x4IdenityMatrix();

/// <summary>
/// 拡大縮小行列を作る
/// </summary>
/// <param name="scale">拡縮</param>
/// <returns>拡大縮小行列</returns>
Matrix4x4 Make4x4ScaleMatrix(Vector3 scale);

/// <summary>
/// X軸の回転行列を作る
/// </summary>
/// <param name="radian">ラジアン</param>
/// <returns>X軸の回転行列</returns>
Matrix4x4 Make4x4RotateXMatrix(float radian);

/// <summary>
/// Y軸の回転行列を作る
/// </summary>
/// <param name="radian">ラジアン</param>
/// <returns>Y軸の回転行列</returns>
Matrix4x4 Make4x4RotateYMatrix(float radian);

/// <summary>
/// Z軸の回転行列を作る
/// </summary>
/// <param name="radian">ラジアン</param>
/// <returns>Z軸の回転行列</returns>
Matrix4x4 Make4x4RotateZMatrix(float radian);

/// <summary>
/// 回転行列を作る
/// </summary>
/// <param name="rotation">回転</param>
/// <returns>回転行列</returns>
Matrix4x4 Make4x4RotateMatrix(Vector3 rotation);

/// <summary>
/// 平行移動行列を作る
/// </summary>
/// <param name="translation">移動</param>
/// <returns>平行移動行列</returns>
Matrix4x4 Make4x4TranslateMatrix(Vector3 translation);

/// <summary>
/// アフィン変換行列を作る
/// </summary>
/// <param name="scale">拡縮</param>
/// <param name="rotation">回転</param>
/// <param name="translation">移動</param>
/// <returns>アフィン変換行列</returns>
Matrix4x4 Make4x4AffineMatrix(Vector3 scale, Vector3 rotation, Vector3 translation);

/// <summary>
/// 逆行列を作る
/// </summary>
/// <param name="m">行列</param>
/// <returns>逆行列</returns>
Matrix4x4 Make4x4InverseMatrix(Matrix4x4 m);

/// <summary>
/// 透視投影行列を作る
/// </summary>
/// <param name="fovY">画角</param>
/// <param name="aspectRatio">アスペクト比</param>
/// <param name="nearClip">近平面との距離</param>
/// <param name="farClip">遠平面との距離</param>
/// <returns>透視投影行列</returns>
Matrix4x4 Make4x4PerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

/// <summary>
/// 正射影行列を作る
/// </summary>
/// <param name="left">左</param>
/// <param name="top">上</param>
/// <param name="right">右</param>
/// <param name="bottom">下</param>
/// <param name="nearClip">近平面との距離</param>
/// <param name="farClip">遠平面との距離</param>
/// <returns>正射影行列</returns>
Matrix4x4 Make4x4OrthographicsMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

/// <summary>
/// ビューポート変換行列を作る
/// </summary>
/// <param name="left">左</param>
/// <param name="top">上</param>
/// <param name="width">幅</param>
/// <param name="height">高さ</param>
/// <param name="minDepth">最小深度値</param>
/// <param name="maxDepth">最大深度値</param>
/// <returns>ビューポート変換行列</returns>
Matrix4x4 Make4x4ViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);