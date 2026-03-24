#pragma once
#include <vector>

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

	struct Timer
	{
		float timer;
        float maxTime;
        bool isLoop;
        bool isFinished;
	};

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

	void SetDeltaPoint() { 
		Update();
		deltaTime = 0.0f;
	}

	void Update();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

#ifndef NDEBUG
	void DrawImGui();
#endif // !NDEBUG


	/// <summary>
    /// DeltaTimeの取得
	/// </summary>
	/// <returns>deltaTime</returns>
	const float& GetDeltaTime() const { return deltaTime; }

	/// <summary>
	/// TimeScaleを反映しない本来のDeltaTime
	/// </summary>
	/// <returns></returns>
	const float& GetUnscaledDeltaTime() const { return m_unScaledDeltaTime; }

	/// <summary>
    /// 固定フレームレート時間の取得
	/// </summary>
	/// <returns></returns>
	const float& GetUnFixedDeltaTime() const { return 1.0f / maxFPS; }

    /// <summary>
    /// CPU使用率の取得(PDH)
    /// </summary>
    /// <returns></returns>
    const float& GetCPUUsagePDH() const { return cpuUsage; }

    const float& GetGPUUsageNVML() const { return gpuUsage; }

	/// <summary>
	/// タイマーの進行速度を設定
	/// </summary>
	void SetTimeScale(const float& scale) { timeScale = scale; };

	const float& GetTimeScale() const { return timeScale; }

    /// <summary>
    /// タイマーの作成
    /// </summary>
    /// <param name="time">図りたい時間</param>
    /// <param name="loop">ループするか</param>
    /// <returns></returns>
    int CreateTimer(float time, bool loop = false);

	/// <summary>
    /// タイマーの進行度合いを取得
	/// </summary>
	/// <param name="index"></param>
	/// <returns></returns>
	float GetTimerProgress(int index) const {
		if (index < 0 || index >= timers.size()) return 0.0f;
		return timers[index].timer / timers[index].maxTime;
	}

private:
	float deltaTime = 0.0f;

    float cpuUsage = 0.0f;

    float updateUPUUsageTimer = 0.0f;

	bool initialized = false;

    float gpuUsage = 0.0f;

	float maxFPS = 60.0f;

	float timeScale = 1.0f;

	float m_unScaledDeltaTime = 0.0f;

	std::vector<Timer> timers;

	void UpdateDeltaTime();
    void UpdateCPUUsagePDH();
    //void UpdateGPUUsageNVML();

};