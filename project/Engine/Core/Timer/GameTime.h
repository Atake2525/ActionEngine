#pragma once

class GameTime
{
private:
	// シングルトンパターンを適用
	static GameTime* instance;

	// コンストラクタ、デストラクタの隠蔽
	GameTime() = default;
	~GameTime() = default;
	// コピーコンストラクタ、コピー代入演算子の封印
	GameTime(GameTime&) = delete;
	GameTime& operator=(GameTime&) = delete;

public:

	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns>Input* instance</returns>
	static GameTime* GetInstance();

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	void SetDeltaPoint() { deltaTime = 0.0f; }

	void Update();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	const float& GetDeltaTime() const { return deltaTime; }

    const float& GetCPUUsagePDH() const { return cpuUsage; }

    const float& GetGPUUsageNVML() const { return gpuUsage; }

private:
	float deltaTime = 0.0f;

    float cpuUsage = 0.0f;

    float updateUPUUsageTimer = 0.0f;

	bool initialized = false;

    float gpuUsage = 0.0f;



	void UpdateDeltaTime();
    void UpdateCPUUsagePDH();
    void UpdateGPUUsageNVML();

};