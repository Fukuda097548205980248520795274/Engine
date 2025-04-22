#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <time.h>
#include <cassert>
#include <wrl.h>
#include <xaudio2.h>
#include <fstream>
#include "../../Struct.h"

#pragma comment(lib,"xaudio2.lib")

class Sound
{
public:

	// デストラクタ
	~Sound();

	// 初期化
	void Initialize();

	// .wavを読み込みサウンドデータの番号を取得する
	uint32_t LoadSoundGetNumber(const char* fileName);

	// 指定した番号のサウンドデータを再生する
	void SelectNumberPlaySoundWav(uint32_t soundNumber);


private:

	// .wavを読み込む
	SoundData LoadSoundWav(const char* fileName);

	// 音声を再生する
	void PlaySoundWav(const SoundData& soundData);

	// サウンドデータを解放する
	void SoundUnload(SoundData* soundData);


	// 時間
	unsigned int currentTimer_ = static_cast<unsigned int>(time(nullptr));

	// XAudio2
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;

	// マスターボイス
	IXAudio2MasteringVoice* masterVoice_;

	// サウンドの数
	const uint32_t kNumSounds = 256;

	// サウンドデータ
	SoundData soundDatas_[256] = {};

	// サウンドデータの番号
	uint32_t soundDataNumbers_[256] = { 0 };

	// 読み込まれているかどうか（ロードフラグ）
	uint32_t isSoundLoad_[256] = { false };
};

