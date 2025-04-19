#pragma once
#include <Windows.h>
#include <wrl.h>
#include <stdint.h>
#include <string>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include "Struct.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "Class/Window/Window.h"
#include "Class/ErrorDetection/ErrorDetection.h"
#include "Class/Commands/Commands.h"
#include "Class/SwapChain/SwapChain.h"
#include "Class/Fence/Fence.h"
#include "Class/Shader/Shader.h"
#include "Func/StringInfo/StringInfo.h"
#include "Func/Matrix/Matrix.h"
#include "Func/Create/Create.h"
#include "Func/Get/Get.h"
#include "Func/TransitionBarrier/TransitionBarrier.h"
#include "Func/Crash/Crash.h"
#include "Func/Texture/Texture.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

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

	// 三角形を描画する
	void DrawTriangle(struct Transform3D& transform, const Matrix4x4& viewProjectionMatrix,
		float red, float green, float blue, float alpha);


private:

	// リークチェッカー
	D3DResourceLeakChecker leakChecker;


	/*   描画   */

	// ウィンドウ
	Window* window_;

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


	// スワップチェーン
	SwapChain* swapChain_;

	// スワップチェーンのリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource_[2] = { nullptr };


	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
	
	// RTVディスクリプタ と スワップチェーンのリソース を紐づける
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];


	// フェンス
	Fence* fence_;


	// リソースを保存する場所
	const uint32_t kNumResourceMemories = 256;
	Microsoft::WRL::ComPtr<ID3D12Resource> resourceMemories[256] = { nullptr };



	/*   テクスチャ   */

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource_ = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU_{};
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};



	/*   描画設定   */

	// シェーダー
	Shader* shader_;

	// ルートシグネチャの設定
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature_{};

	// ルートパラメータ
	D3D12_ROOT_PARAMETER rootParameters_[2] = {};

	// バイナリデータ
	ID3DBlob* signatureBlob_ = nullptr;
	ID3DBlob* errorBlob_ = nullptr;

	// ルートシグネチャ
	ID3D12RootSignature* rootSignature_ = nullptr;

	// 頂点シェーダのどの変数にinputするかを選ぶ
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[1] = {};

	// 頂点シェーダにinputするデータ
	D3D12_INPUT_LAYOUT_DESC inputLayoutDescs_{};

	// ピクセルシェーダの出力の設定
	D3D12_BLEND_DESC blendDesc_{};

	// ラスタライザの設定
	D3D12_RASTERIZER_DESC rasterizeDesc_{};

	// 頂点シェーダのバイナリデータ
	IDxcBlob* vertexShaderBlob_ = nullptr;

	// ピクセルシェーダーのバイナリデータ
	IDxcBlob* pixelShaderBlob_ = nullptr;

	// PSOの設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc_{};

	// PSO
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_ = nullptr;

	// ビューポート
	D3D12_VIEWPORT viewport_{};

	// シザーレクト
	D3D12_RECT scissorRect_{};
};

