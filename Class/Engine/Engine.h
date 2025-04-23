#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <filesystem>
#include <fstream>
#include <chrono>
#include "Struct.h"
#include "Class/Window/Window.h"
#include "Class/ErrorDetection/ErrorDetection.h"
#include "Class/Commands/Commands.h"
#include "Class/SwapChain/SwapChain.h"
#include "Class/Fence/Fence.h"
#include "Class/Shader/Shader.h"
#include "Class/TextureManager/TextureManager.h"
#include "Class/Sound/Sound.h"
#include "Class/Input/Input.h"
#include "Class/ModelManager/ModelManager.h"
#include "Func/StringInfo/StringInfo.h"
#include "Func/Matrix/Matrix.h"
#include "Func/Create/Create.h"
#include "Func/Get/Get.h"
#include "Func/TransitionBarrier/TransitionBarrier.h"
#include "Func/Crash/Crash.h"
#include "Func/Texture/Texture.h"
#include "Func/ModelData/ModelData.h"

class Engine
{
public:

	// デストラクタ
	~Engine();

	// 初期化
	void Initialize(const int32_t kClientWidth, const int32_t kClientHeight);

	// ウィンドウが開いているかどうか
	bool IsWindowOpen();

	// フレーム開始
	void BeginFrame();

	// フレーム終了
	void EndFrame();

	// テクスチャを読み込む
	uint32_t LoadTexture(const std::string& filePath);

	// モデルデータを読み込む
	uint32_t LoadModelData(const std::string& directory, const std::string& fileName);

	// サウンドデータを読み込む
	uint32_t LoadSound(const char* fileName);

	// サウンドデータを再生する
	void PlayerSoundWav(uint32_t soundHandle);

	// キー操作（Press）
	UINT PushPressKeys(BYTE key);

	// キー操作（Trigger）
	UINT PushTriggerKeys(BYTE key);

	// キー操作（Release）
	UINT PushReleaseKeys(BYTE key);

	// 三角形を描画する
	void DrawTriangle(struct Transform3D& transform,const Matrix4x4& viewProjectionMatrix,uint32_t textureHandle , Vector3 color);

	// スプライトを描画する
	void DrawSprite(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
		const Transform3D& transform, const Matrix4x4& viewOrthograhpicsMatrix,uint32_t textureHandle);

	// 球を描画する
	void DrawSphere(uint32_t subdivisions,const Transform3D& transform, const Matrix4x4& viewProjectionMatrix,
		const DirectionalLight& light, uint32_t textureHandle);

	// モデルを描画する
	void DrawModel(uint32_t modelHandle, Transform3D& transform, const Matrix4x4& viewProjectionMatrix, const DirectionalLight& light);


private:

	// リークチェッカー
	D3DResourceLeakChecker leakChecker;


	// ウィンドウ
	Window* window_;

	// 入力
	Input* input_;


	// エラーを感知するクラス
	ErrorDetection* errorDetection_;


	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;

	// 使用するアダプタ（GPU）
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;

	// デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;


	// コマンド
	Commands* commands_;


	// RTVのディスクリプタの数
	const UINT kNumRtvDescriptor_ = 2;

	// RTV用のディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_ = nullptr;

	
	// SRVのディスクリプタの数
	const UINT kNunSrvDescriptor_ = 128;

	// SRV用のディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_ = nullptr;


	// DSV用のディスクリプタの数
	const UINT kNunDsvDescriptor_ = 1;

	// DSV用のディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_ = nullptr;


	// スワップチェーン
	SwapChain* swapChain_;

	// スワップチェーンのリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource_[2] = { nullptr };


	
	// RTVディスクリプタ と スワップチェーンのリソース を紐づける
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];


	// デプスステンシルのリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;


	// フェンス
	Fence* fence_;


	// リソースを保存する場所
	const uint32_t kNumResourceMemories = 1024;
	Microsoft::WRL::ComPtr<ID3D12Resource> resourceMemories[1024] = { nullptr };


	// テクスチャマネージャ
	TextureManager* textureManager_;

	// モデルマネージャ
	ModelManager* modelManager_;

	// サウンド
	Sound* sound_;


	// シェーダー
	Shader* shader_;

	// バイナリデータ
	ID3DBlob* signatureBlob_ = nullptr;
	ID3DBlob* errorBlob_ = nullptr;

	// ルートシグネチャ
	ID3D12RootSignature* rootSignature_ = nullptr;

	// 頂点シェーダのバイナリデータ
	IDxcBlob* vertexShaderBlob_ = nullptr;

	// ピクセルシェーダーのバイナリデータ
	IDxcBlob* pixelShaderBlob_ = nullptr;

	// PSO
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

	// ビューポート
	D3D12_VIEWPORT viewport_{};

	// シザーレクト
	D3D12_RECT scissorRect_{};
};

