#include <xaudio2.h>
#include <fstream>
#include <wrl.h>
#include <vector>
#include <map>

#include "Vector2.h"
#include "Vector3.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

#pragma once

#pragma comment(lib, "xaudio2.lib")

// 音声データ
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
	// ファイル
	std::string filePath;
	// ファイルの再生時間
	float playTime;
	// 音量
	float volume;
};

struct AudioList
{
	IXAudio2SourceVoice* sourceVoice;
	SoundData soundData;
	XAUDIO2_BUFFER buf{};
	int startFrameTime;
};


class Audio {
private:
	// シングルトンパターンを適用
	static Audio* instance;

	// コンストラクタ、デストラクタの隠蔽
	Audio() = default;
	~Audio() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	Audio(Audio&) = delete;
	Audio& operator=(Audio&) = delete;

public:

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// インスタンスの取得
	static Audio* GetInstance();

	// 終了処理
	void Finalize();

	// wave読み込み
	bool LoadWave(const std::string filePath, const std::string soundName, const float volume = 1.0f);

	// mp3読み込み
	bool LoadMP3(const std::string filePath, const std::string soundName, const float volume = 1.0f);

	// 音量設定
	void SetVolume(const std::string soundName, const float volume);

	// 主音量設定
	void SetMasterVolume(const float volume);

	// 音声再生
	void Play(const std::string soundName, const bool loop = false);

	// 音声再生(上下無し)
	void Play2D(const std::string soundName, const Vector2 position, const bool loop = false);

	// 音声再生(上下有り)
	void Play3D(const std::string soundName, const Vector3 position, const bool loop = false);

	//void PlayMp3(const bool loop = false, const float volume = 1.0f);

	// 全ての音声停止
	void StopAll();

	// 音声停止
	void Stop(const std::string soundName);

	// 一時停止
	void Pause(const std::string soundName);

	// 一時停止した音声の再開
	void Resume(const std::string soundName);

	// 音声データ解放
	void SoundUnload(const std::string soundName);

	void Unload(SoundData* soundData);

private:

	// 最大SRV数(最大テクスチャ枚数)
	static const uint32_t maxSourceVoiceCount;

	// audio test
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice = nullptr;

	std::vector<AudioList> audioList;

	std::map<std::string, SoundData> soundMap;

	// オーディオデータ
	std::map<std::string, std::vector<BYTE>> mp3AudioData;
	// WAVEフォーマット
	WAVEFORMATEX mp3waveFormat;

	int frameTime = 0;

	// Media Foundation SourceReader
	Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader{ nullptr };

};