#include "TextureManager.h"

// 初期化する
void TextureManager::Initialize()
{
	srand(currentTimer_);
}

// テクスチャを読み込む
uint32_t TextureManager::LoadTextureGetNumber(const std::string& filePath, Microsoft::WRL::ComPtr<ID3D12Device> device,
	 Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap,Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
	DirectX::ScratchImage mipImage = LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImage.GetMetadata();


	// 使用していないテクスチャリソースに格納する
	for (uint32_t i = 0; i < kNumTexture_; i++)
	{
		if (textureResources_[i])
			continue;


		while (textureNumbers_[i] == 0)
		{
			textureNumbers_[i] = rand() % 10000 + 1;

			// 同じ数値があったら初期化する
			for (uint32_t j = 0; j < kNumTexture_; j++)
			{
				if (textureResources_[j] == nullptr)
					continue;

				if (textureNumbers_[i] != textureNumbers_[j])
					continue;

				textureNumbers_[i] = 0;
			}
		}

		textureResources_[i] = CreateTextureResource(device, metadata);
		intermediateResources_[i] = UploadTextureData(textureResources_[i], mipImage, device, commandList);

		// metaDataを基にSRVを作成する
		srvDesc_[i].Format = metadata.format;
		srvDesc_[i].Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc_[i].ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc_[i].Texture2D.MipLevels = UINT(metadata.mipLevels);

		cpuDescriptorHandle_[i] = GetCPUDescriptorHandle(srvDescriptorHeap, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 1 + textureCount_);
		gpuDescriptorHandle_[i] = GetGPUDescriptorHandle(srvDescriptorHeap, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 1 + textureCount_);

		// SRVを生成する
		device->CreateShaderResourceView(textureResources_[i].Get(), &srvDesc_[i], cpuDescriptorHandle_[i]);

		// テクスチャの数をカウントする
		textureCount_++;

		return textureNumbers_[i];
	}

	assert(false);

	return -1;
}

// 指定したテクスチャを使う
void TextureManager::SelectTexture(uint32_t textureNumber, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
{
	for (uint32_t i = 0; i < kNumTexture_; i++)
	{
		if (textureNumber != textureNumbers_[i])
			continue;

		if (textureResources_[i] == nullptr)
			continue;

		commandList->SetGraphicsRootDescriptorTable(2, gpuDescriptorHandle_[i]);

		return;
	}

	assert(false);

	return;
}