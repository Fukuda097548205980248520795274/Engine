#include "Sound.h"

// デストラクタ
Sound::~Sound()
{
	// XAudio2
	xAudio2_.Reset();

	// 使用したサウンドデータを解放する
	for (uint32_t i = 0; i < kNumSounds; ++i)
	{
		if (isSoundLoad_[i] == false)
			continue;

		SoundUnload(&soundDatas_[i]);
	}
}

// 初期化
void Sound::Initialize()
{
	srand(currentTimer_);

	// XAudioエンジンの初期化
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	// マスターボイスを生成する
	hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(hr));
}

// .wavを読み込む
SoundData Sound::LoadSoundWav(const char* fileName)
{
	/*------------------
		ファイルを開く
	------------------*/

	// ファイル入力ストリームのインスタンス
	std::ifstream file;

	// .wavファイルをバイナリモードで開く
	file.open(fileName, std::ios_base::binary);
	assert(file.is_open());


	/*---------------------
		wavデータ読み込み
	---------------------*/

	// RIFFヘッダ読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));

	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0)
	{
		assert(0);
	}

	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0)
	{
		assert(0);
	}


	// formatチャンクの読み込み
	FormatChunk format = {};

	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt", 3) != 0)
	{
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);


	// Dataチャンク読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));

	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0)
	{
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);

		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0)
	{
		assert(0);
	}

	// Dataチャンクのデータ部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// Waveファイルを閉じる
	file.close();


	/*-----------------------------
		読み込んだデータを返却する
	-----------------------------*/

	// 返却するための音声データ
	SoundData soundData{};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

// サウンドデータを解放する
void Sound::SoundUnload(SoundData* soundData)
{
	// バッファメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

// 音声を再生する
void Sound::PlaySoundWav(const SoundData& soundData)
{
	// 波形フォーマットを基にSourceVoiceを生成する
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	HRESULT hr = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(hr));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	hr = pSourceVoice->Start();
}

// .wavを読み込み、サウンドデータの番号を取得する
uint32_t Sound::LoadSoundGetNumber(const char* fileName)
{
	// 使用していない（ロードフラグがfalseの）サウンドデータで読み込む
	for (uint32_t i = 0; i < kNumSounds; ++i)
	{
		if (isSoundLoad_[i])
			continue;

		// 番号を取得する
		while (soundDataNumbers_[i] == 0)
		{
			soundDataNumbers_[i] = rand() % 10000 + 1;

			// 一致していなかを確かめる
			for (uint32_t j = 0; j < kNumSounds; j++)
			{
				if (isSoundLoad_[j] == false)
					continue;

				if (i == j)
					continue;

				// 同じ番号は初期化する
				if (soundDataNumbers_[i] == soundDataNumbers_[j])
				{
					soundDataNumbers_[i] = 0;

					break;
				}
			}
		}

		soundDatas_[i] = LoadSoundWav(fileName);
		isSoundLoad_[i] = true;

		return soundDataNumbers_[i];
	}

	assert(false);

	return 0;
}

// 指定した番号のサウンドデータを再生する
void Sound::SelectNumberPlaySoundWav(uint32_t soundNumber)
{
	for (uint32_t i = 0; i < kNumSounds; ++i)
	{
		if (isSoundLoad_[i] == false)
			continue;

		if (soundNumber != soundDataNumbers_[i])
			continue;

		PlaySoundWav(soundDatas_[i]);

		break;
	}
}