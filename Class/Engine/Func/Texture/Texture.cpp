#include "Texture.h"

/// <summary>
/// テクスチャをCPUに読み込む
/// </summary>
/// <param name="filePath">ファイルパス</param>
/// <returns></returns>
DirectX::ScratchImage LoadTexture(const std::string& filePath)
{
	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミップマップを作成する
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	// ミップマップ付きのデータを返却する
	return mipImages;
}

/// <summary>
/// テクスチャのメタデータを基にに、テクスチャリソースを作成する
/// </summary>
/// <param name="device"></param>
/// <param name="metadata"></param>
/// <returns></returns>
Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metadata)
{
	/*------------------------------------
	    メタデータを基に、リソースを作成する
	------------------------------------*/

	D3D12_RESOURCE_DESC resourceDesc{};

	// Textureの幅と高さ
	resourceDesc.Width = UINT(metadata.width);
	resourceDesc.Height = UINT(metadata.height);

	// mipmapの数
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);

	// 奥行き or 配列Textureの配列数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);

	// Textureのフォーマット
	resourceDesc.Format = metadata.format;

	// サンプリングカウント
	resourceDesc.SampleDesc.Count = 1;

	// テクスチャの次元数
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);


	/*------------------------
	    利用するヒープの設定
	------------------------*/

	D3D12_HEAP_PROPERTIES heapProperties{};

	// 細かい設定を行う
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;

	// WriteBackポリシーでCPUアクセス可能
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;

	// プロセッサの近くに配置する
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;


	/*----------------------
	    リソースを生成する
	----------------------*/

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource
	(
		// Heapの設定
		&heapProperties,

		// Heapの特殊な設定
		D3D12_HEAP_FLAG_NONE,

		// リソースの設定
		&resourceDesc,

		// 初回のリソースの状態
		D3D12_RESOURCE_STATE_GENERIC_READ,

		// Clear最適値
		nullptr,

		// 作成するリソースのポインタのポインタ
		IID_PPV_ARGS(&resource)
	);

	assert(SUCCEEDED(hr));

	return resource;
}

/// <summary>
/// テクスチャリソースにテクスチャ情報を転送する
/// </summary>
/// <param name="texture"></param>
/// <param name="mipImages"></param>
void UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages)
{
	// メタ情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	// 全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel)
	{
		// MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);

		// テクスチャリソースに転送
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),

			// 全領域へコピー
			nullptr,

			// 元データアドレス
			img->pixels,

			// 1ラインサイズ
			UINT(img->rowPitch),

			// 1枚サイズ
			UINT(img->slicePitch)
		);

		assert(SUCCEEDED(hr));
	}
}