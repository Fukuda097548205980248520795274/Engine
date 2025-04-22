#pragma once
#include <stdlib.h>
#include <time.h>
#include "../../Struct.h"
#include "../../Func/ModelData/ModelData.h"

class ModelManager
{
public:

	// 初期化
	void Initialize();

	// モデルを読み込み、番号を取得する
	uint32_t LoadModelGetNumber(const std::string& directory, const std::string& fileName);

	// Getter
	uint32_t GetNumModel() { return kNumModel; }
	uint32_t GetTextureNumber(uint32_t modelNumber);
	ModelData GetModelData(uint32_t modelNumber);
	
	// Setter
	void SetTextureNumber(uint32_t modelNumber , uint32_t textureNumber);

private:

	// 時間
	unsigned int currentTimer_ = static_cast<unsigned int>(time(nullptr));

	// 使用できるモデル数
	const uint32_t kNumModel = 256;

	// モデルデータ
	ModelData modelDatas_[256] = {};

	// モデル番号
	uint32_t modelNumbers_[256] = { 0 };

	// テクスチャの番号
	uint32_t textureNumbers_[256] = { 0 };

	// 読み込んでいるかどうか（ロードフラグ）
	UINT isLoad_[256] = { false };
};

