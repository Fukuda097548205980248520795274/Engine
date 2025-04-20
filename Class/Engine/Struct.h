#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

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

	// 姿勢情報2D
	typedef struct Transform2D
	{
		// 拡縮
		Vector2 scale;

		// 回転
		float theta;

		// 移動
		Vector2 translate;
	}Transform2D;

	// 姿勢情報3D
	typedef struct Transform3D
	{
		// 拡縮
		Vector3 scale;

		// 回転
		Vector3 rotate;

		// 移動
		Vector3 translate;

	}Transform3D;

	// 頂点データ
	typedef struct VertexData
	{
		// 位置
		Vector4 position;

		// テクスチャ座標
		Vector2 texcoord;

		// 法線
		Vector3 normal;

	}VertexData;

	// マテリアル
	typedef struct Material
	{
		// 色
		Vector4 color;

		// ライティングを有効にするかどうか
		int32_t enableLighting;
	}Material;

	// 座標変換用行列
	typedef struct TransformationMatrix
	{
		// ワールドビュープロジェクション行列
		Matrix4x4 worldViewProjection;

		// ワールド行列
		Matrix4x4 world;
	}TransformationMatrix;

	// 平行光源
	typedef struct DirectionalLight
	{
		// ライトの色
		Vector4 color;

		// ライトの向き
		Vector3 direction;

		// 輝度
		float intensity;
	}DirectionalLight;

	// リソースリークチェッカー
	typedef struct D3DResourceLeakChecker
	{
		~D3DResourceLeakChecker()
		{
			// リソースリークチェッカー
			Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))))
			{
				debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			}
		}
	}D3DResourceLeakChecker;