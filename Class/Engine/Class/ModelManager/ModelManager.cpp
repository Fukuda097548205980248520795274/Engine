#include "ModelManager.h"

// 初期化
void ModelManager::Initialize()
{
	srand(currentTimer_);
}

// モデルを読み込み、番号を取得する
uint32_t ModelManager::LoadModelGetNumber(const std::string& directory, const std::string& fileName)
{
	// 使用していない（ロードフラグがfalse）場所に、格納する
	for (uint32_t i = 0; i < kNumModel; ++i)
	{
		if (isLoad_[i])
			continue;

		// 番号を取得する
		while (modelNumbers_[i] == 0)
		{
			modelNumbers_[i] = rand() % 10000 + 1;

			// 一致していなかを確かめる
			for (uint32_t j = 0; j < kNumModel; j++)
			{
				if (isLoad_[j] == false)
					continue;

				if (i == j)
					continue;

				// 同じ番号は初期化する
				if (modelNumbers_[i] == modelNumbers_[j])
				{
					modelNumbers_[i] = 0;

					break;
				}
			}
		}

		// ロードする（ロードフラグをtrueにする）
		isLoad_[i] = true;
		modelDatas_[i] = LoadObjFile(directory, fileName);
		
		return modelNumbers_[i];
	}

	assert(false);

	return 0;
}

// 指定した番号のモデルの要素数を取得する
ModelData ModelManager::GetModelData(uint32_t modelNumber)
{
	for (uint32_t i = 0; i < kNumModel; ++i)
	{
		if (isLoad_[i] == false)
			continue;

		if (modelNumber != modelNumbers_[i])
			continue;

		return modelDatas_[i];
	}

	assert(false);
	return modelDatas_[0];
}

// 指定した番号のモデルのテクスチャ番号を入力する
void ModelManager::SetTextureNumber(uint32_t modelNumber, uint32_t textureNumber)
{
	for (uint32_t i = 0; i < kNumModel; ++i)
	{
		if (isLoad_[i] == false)
			continue;

		if (modelNumber != modelNumbers_[i])
			continue;

		textureNumbers_[i] = textureNumber;

		break;
	}

	return;
}

// 指定した番号のモデルのテクスチャを取得する
uint32_t ModelManager::GetTextureNumber(uint32_t modelNumber)
{
	for (uint32_t i = 0; i < kNumModel; ++i)
	{
		if (isLoad_[i] == false)
			continue;

		if (modelNumber != modelNumbers_[i])
			continue;

		return textureNumbers_[i];
	}

	assert(false);
	return textureNumbers_[0];
}