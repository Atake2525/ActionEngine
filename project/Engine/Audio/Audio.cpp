#include "Audio.h"
#include <cassert>
#include <algorithm>
#include "Logger.h"

#include "externels/imgui/imgui.h"
#include "externels/imgui/imgui_impl_dx12.h"
#include "externels/imgui/imgui_impl_win32.h"

using namespace Logger;

// チャンクヘッダ
struct ChunkHeader {
	char id[4];   // チャンク毎のID
	int32_t size; // チャンクサイズ
};

// RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk; // "RIFF"
	char tpye[4];      // "WAVE"
};

// FMTチャンク
struct FormatChunk {
	ChunkHeader chunk; // "fmt"
	WAVEFORMATEX fmt;  // 波形フォーマット
};

// 同時に再生できる最大数
const uint32_t Audio::maxSourceVoiceCount = 64;

Audio* Audio::instance = nullptr;


Audio* Audio::GetInstance() {
	if (instance == nullptr) {
		instance = new Audio;
	}
	return instance;
}

void Audio::Finalize() {
	for (AudioList list : audioList)
	{
		list.sourceVoice->Stop();
		list.sourceVoice->DestroyVoice();
	}
	audioList.clear();
	soundMap.clear();
	mp3AudioData.clear();
	delete instance;
	instance = nullptr;
}

void Audio::Initialize() {
	xAudio2.Reset();
	HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	hr = xAudio2->CreateMasteringVoice(&masterVoice);

	// mp3読み込みのためのMedia Foundationの初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
}

bool Audio::LoadWave(const std::string filePath, const std::string soundName, const float volume) {
	// 登録する名前が重複していたらfalseでreturn
	if (soundMap.contains(soundName))
	{
		return false;
		Log("this name already registered/n");
	}
	// 登録する音声が重複していたらfalseでreturn
	for (auto it = soundMap.begin(); it != soundMap.end(); ++it)
	{
		if (it->second.filePath == filePath)
		{
			Log("this file loaded\n");
			return false;
		}
	}


	/// ファイルオープン

	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(filePath, std::ios_base::binary);
	if (!file.is_open())
	{
		// ファイルオープン失敗を検出する
#ifdef _DEBUG
		assert(0);
#endif // _DEBUG

		Log("Cant open file\n");
		return false;
	}

	/// .wavデータ読み込み

	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}

	// タイプがWAVEかチェック
	if (strncmp(riff.tpye, "WAVE", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// Dataチャンクのデータ部(波形データ)の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	int time = data.size / format.fmt.nAvgBytesPerSec;
	if (time <= 0)
	{
		time = 1;
	}

	// Waveファイルを閉じる
	file.close();

	// 0.0f ~ 1.0fにclampする
	float vol = std::clamp(volume, 0.0f, 1.0f);

	// returnする為の音声データ
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;
	soundData.filePath = filePath;
	soundData.playTime = float(time);
	soundData.volume = vol;

	soundMap[soundName] = soundData;

	return true;
}

// 音声再生
void Audio::Play(const std::string soundName, const bool loop) {

	// 存在しないサウンド名の場合は早期return
	/*for (auto it = soundMap.begin(); it != soundMap.end(); it++)
	{
		if (soundMap.find(soundName))
		{

		}
	}*/
	if (!soundMap.contains(soundName))
	{
		Log("this name is not registered\n");
		return;
	}

	HRESULT result;

	// 波形フォーマットをもとにSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundMap[soundName].wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.AudioBytes = soundMap[soundName].bufferSize;
	buf.pAudioData = soundMap[soundName].pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	if (loop)
	{
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->SetVolume(soundMap[soundName].volume);
	result = pSourceVoice->Start();
	AudioList list = { pSourceVoice, soundMap[soundName], buf, frameTime };
	audioList.push_back(list);
	Log("sound playing\n");
	// 指定したsourceVoiceよりも多くpush_backしたらassert
	assert(audioList.size() < maxSourceVoiceCount);
}

void Audio::Play2D(const std::string soundName, const Vector2 position, const bool loop)
{
	if (!soundMap.contains(soundName))
	{
		Log("this name is not registered\n");
		return;
	}

	HRESULT result;

	// 波形フォーマットをもとにSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundMap[soundName].wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.AudioBytes = soundMap[soundName].bufferSize;
	buf.pAudioData = soundMap[soundName].pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	if (loop)
	{
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	Vector2 pos = position;

	// 出力チャンネル数(ステレオなら2)
	const UINT32 outputChannels = 2;

	// 入力チャンネル数(モノラルなら1)
	const UINT32 inputChannels = 1;
	XAUDIO2_VOICE_DETAILS sourceVoiceDetails;
	pSourceVoice->GetVoiceDetails(&sourceVoiceDetails);
	XAUDIO2_VOICE_DETAILS masterVoiceDetails;
	masterVoice->GetVoiceDetails(&masterVoiceDetails);

	// 出力マトリクス(左右)
	float matrix[2] = { 1.0f, 0.0f };

	pSourceVoice->SetOutputMatrix(nullptr, sourceVoiceDetails.InputChannels, masterVoiceDetails.InputChannels, matrix);
	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->SetVolume(soundMap[soundName].volume);
	result = pSourceVoice->Start();
	AudioList list = { pSourceVoice, soundMap[soundName], buf, frameTime };
	audioList.push_back(list);
	Log("sound playing\n");
	// 指定したsourceVoiceよりも多くpush_backしたらassert
	assert(audioList.size() < maxSourceVoiceCount);
}

void Audio::Play3D(const std::string soundName, const Vector3 position, const bool loop)
{
	if (!soundMap.contains(soundName))
	{
		Log("this name is not registered\n");
		return;
	}

	HRESULT result;

	// 波形フォーマットをもとにSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundMap[soundName].wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.AudioBytes = soundMap[soundName].bufferSize;
	buf.pAudioData = soundMap[soundName].pBuffer;
	buf.Flags = XAUDIO2_END_OF_STREAM;
	if (loop)
	{
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	Vector3 pos = position;

	// 出力チャンネル数(ステレオなら2)
	const UINT32 outputChannels = 2;

	// 入力チャンネル数(モノラルなら1)
	const UINT32 inputChannels = 1;

	// 出力マトリクス(左右)
	float matrix[2] = { 1.0f, 0.0f };

	pSourceVoice->GetOutputMatrix(nullptr, inputChannels, outputChannels, matrix);
	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->SetVolume(soundMap[soundName].volume);
	result = pSourceVoice->Start();
	AudioList list = { pSourceVoice, soundMap[soundName], buf, frameTime };
	audioList.push_back(list);
	// 指定したsourceVoiceよりも多くpush_backしたらassert
	assert(audioList.size() < maxSourceVoiceCount);
}

bool Audio::LoadMP3(const std::string filePath, const std::string soundName, const float volume) {
	// 登録する名前が重複していたらfalseでreturn
	if (soundMap.contains(soundName))
	{
		return false;
		Log("this name already registered\n");
	}
	// 登録する音声が重複していたらfalseでreturn
	for (auto it = soundMap.begin(); it != soundMap.end(); ++it)
	{
		if (it->second.filePath == filePath)
		{
			Log("this file loaded\n");
			return false;
		}
	}
	// Media Foundationオブジェクト
	Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
	Microsoft::WRL::ComPtr<IMFMediaType> mediaType;

	// ファイルパスをワイド文字列に変換
	wchar_t wFilePath[MAX_PATH];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wFilePath, filePath.c_str(), MAX_PATH);

	// ソースリーダーの作成
	HRESULT hr = MFCreateSourceReaderFromURL(wFilePath, nullptr, sourceReader.GetAddressOf());
	if (FAILED(hr)) {
		return false;
	}

	// メディアタイプの取得
	hr = sourceReader->GetNativeMediaType(
		MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		0,
		mediaType.GetAddressOf());
	if (FAILED(hr)) {
		return false;
	}

	// PCMフォーマットに変換
	hr = MFCreateMediaType(mediaType.GetAddressOf());
	if (FAILED(hr)) {
		return false;
	}

	hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	if (FAILED(hr)) {
		return false;
	}

	hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	if (FAILED(hr)) {
		return false;
	}

	// メディアタイプの設定
	hr = sourceReader->SetCurrentMediaType(
		MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		nullptr,
		mediaType.Get());
	if (FAILED(hr)) {
		return false;
	}

	// 変換後のメディアタイプを取得
	hr = sourceReader->GetCurrentMediaType(
		MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		mediaType.GetAddressOf());
	if (FAILED(hr)) {
		return false;
	}

	// WAVEフォーマット情報を取得
	UINT32 formatSize = 0;
	WAVEFORMATEX* pWaveFormat = nullptr;
	hr = MFCreateWaveFormatExFromMFMediaType(
		mediaType.Get(),
		&pWaveFormat,
		&formatSize);
	if (FAILED(hr)) {
		return false;
	}

	// WAVEフォーマット
	//WAVEFORMATEX mp3waveFormat;

	// WAVEフォーマットをコピー
	memcpy(&mp3waveFormat, pWaveFormat, sizeof(WAVEFORMATEX));

	// WAVEフォーマットをコピー
	/*memcpy(&mp3WaveFormat, pWaveFormat, sizeof(WAVEFORMATEX));
	CoTaskMemFree(pWaveFormat);*/

	// オーディオデータの読み込み
	//mp3AudioData.clear();

	SoundData soundData;

	while (true) {
		// サンプルの読み込み
		Microsoft::WRL::ComPtr<IMFSample> sample;
		DWORD streamFlags = 0;

		hr = sourceReader->ReadSample(
			MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0,
			nullptr,
			&streamFlags,
			nullptr,
			sample.GetAddressOf());

		if (FAILED(hr) || (streamFlags & MF_SOURCE_READERF_ENDOFSTREAM)) {
			break;
		}

		if (sample == nullptr) {
			continue;
		}

		// サンプルからメディアバッファを取得
		Microsoft::WRL::ComPtr<IMFMediaBuffer> mediaBuffer;
		hr = sample->ConvertToContiguousBuffer(mediaBuffer.GetAddressOf());
		if (FAILED(hr)) {
			continue;
		}

		// メディアバッファからデータを取得
		BYTE* audioBuffer = nullptr;
		DWORD bufferSize = 0;

		hr = mediaBuffer->Lock(&audioBuffer, nullptr, &bufferSize);
		if (FAILED(hr)) {
			continue;
		}


		// データをコピー
		size_t offset = mp3AudioData[soundName].size();
		mp3AudioData[soundName].resize(offset + bufferSize);
		memcpy(mp3AudioData[soundName].data() + offset, audioBuffer, bufferSize);
		
		// バッファのロックを解除
		mediaBuffer->Unlock();
	}

	// 再生時間を計算する
	int time = static_cast<int>(mp3AudioData.size() / pWaveFormat->nAvgBytesPerSec);
	if (time <= 0)
	{
		time = 1;
	}
	CoTaskMemFree(pWaveFormat);
	// 音量を0.0f ~ 1.0fにclampする
	float vol = std::clamp(volume, 0.0f, 1.0f);

	soundData.wfex = mp3waveFormat;
	soundData.bufferSize = static_cast<UINT32>(mp3AudioData[soundName].size());
	soundData.pBuffer = mp3AudioData[soundName].data();
	soundData.filePath = filePath;
	soundData.volume = vol;
	soundData.playTime = float(time);

	/*mp3AudioData.clear(); */

	soundMap[soundName] = soundData;

	Log("Audio load\n");
	Log("音声のロード完了\n");

	mp3waveFormat = {};
	return true;
}

void Audio::SetVolume(const std::string soundName, const float volume) {
	// 0.0f ~ 1.0fにclampする
	float vol = std::clamp(volume, 0.0f, 1.0f);
	soundMap[soundName].volume = vol;
	for (AudioList list : audioList)
	{
		if (list.soundData.filePath == soundMap[soundName].filePath)
		{
			list.sourceVoice->SetVolume(soundMap[soundName].volume);
			return;
		}
	}
}

void Audio::SetMasterVolume(const float volume){
	// 0.0f ~ 1.0fにclampする
	float vol = std::clamp(volume, 0.0f, 1.0f);
	masterVoice->SetVolume(vol);
}

// 全ての音声停止
void Audio::StopAll() {
	// listに登録されているaudioSourceの全てを音声停止してlistをclearする
	for (AudioList list : audioList)
	{
		list.sourceVoice->Stop();
		list.sourceVoice->DestroyVoice();
	}
	audioList.clear();
}

// 音声停止
void Audio::Stop(const std::string soundName) {
	// listに登録されているaudioSourceの中から指定されたsoundDataのfilenameに一致するもの全てを音声停止して一致するものをlistからremoveする
	uint32_t index = 0;
	uint32_t eraseList[maxSourceVoiceCount] = { 0 };
	uint32_t eraseNum = 0;
	for (AudioList list : audioList)
	{
		if (list.soundData.filePath == soundMap[soundName].filePath)
		{
			list.sourceVoice->Stop();
			list.sourceVoice->DestroyVoice();
			eraseList[eraseNum] = index;
			eraseNum++;
		}
		index++;
	}
	for (uint32_t i = 0; i < eraseNum; i++)
	{
		audioList.erase(audioList.begin() + eraseList[i]);
		eraseList[i + 1] -= i + 1;
	}
}

void Audio::Pause(const std::string soundName) {
	for (AudioList list : audioList)
	{
		if (list.soundData.filePath == soundMap[soundName].filePath)
		{
			list.sourceVoice->Stop();
		}
	}
}

void Audio::Resume(const std::string soundName) {
	for (AudioList list : audioList)
	{
		if (list.soundData.filePath == soundMap[soundName].filePath)
		{
			list.sourceVoice->Start();
		}
	}
}



void Audio::Update() {
	// audioListのサイズが0なら早期return
	if (audioList.size() == 0) { 
		frameTime = 0;
		return;
	}
	uint32_t index = 0;
	for (AudioList list : audioList)
	{
		if (frameTime >= list.soundData.playTime * 60 + list.startFrameTime && list.buf.LoopCount != XAUDIO2_LOOP_INFINITE)
		{ 
			// 再生時間ごとに削除 
			// 前から再生時間が過ぎたら削除
			// listのbufferがループになっていないものを対象にする
			list.sourceVoice->Stop();
			list.sourceVoice->DestroyVoice();
			audioList.erase(audioList.begin() + index);
			break;
		}
		index++;
	}
	frameTime++;
}

// 音声データ解放
void Audio::SoundUnload(const std::string soundName) {
	// バッファのメモリを解放
	//delete[] soundMap[soundName].pBuffer;

	soundMap.erase(soundName);
	mp3AudioData.erase(soundName);
	Log("sound unloaded\n");
}

void Audio::Unload(SoundData* soundData) {
	// バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}
