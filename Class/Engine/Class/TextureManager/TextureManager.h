#pragma once
#include <stdlib.h>
#include <time.h>
#include <wrl.h>
#include <stdint.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include "../../Func/Get/Get.h"
#include "../../Func/Texture/Texture.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

class TextureManager
{
public:

	// 初期化
	void Initialize();

	// テクスチャを読み込む
	uint32_t LoadTextureGetNumber(const std::string& filePath ,Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap,Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

	// 指定したテクスチャを使う
	void SelectTexture(uint32_t textureNumber ,  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

private:

	// 乱数
	unsigned int currentTimer_ = static_cast<unsigned int>(time(nullptr));

	// 格納できるテクスチャの数
	const uint32_t kNumTexture_ = 256;

	// 使用したテクスチャの数をカウントする
	uint32_t textureCount_ = 0;

	// テクスチャ番号
	uint32_t textureNumbers_[256] = { 0 };

	// テクスチャリソースを格納する
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResources_[256] = { nullptr };

	// リソースに転送するデータ
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResources_[256] = { nullptr };

	// SRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc_[256] = {};

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle_[256] = {};
	D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle_[256] = {};
};

