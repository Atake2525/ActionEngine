#include "GameTime.h"
#include <chrono>
#include <windows.h>
#include <pdh.h>
#include "DirectXBase.h"
#pragma comment(lib, "pdh.lib")

#ifndef NDEBUG
#include "ImGuiManager.h"
#endif // !NDEBUG


GameTime* GameTime::instance = nullptr;

GameTime* GameTime::GetInstance() {
    if (instance == nullptr) {
        instance = new GameTime;
    }
    return instance;
}

void GameTime::Finalize() {
    delete instance;
    instance = nullptr;
}

void GameTime::Initialize() {
    deltaTime = 0.0f;
    maxFPS = DirectXBase::GetInstance()->GetMaxFPS();
}

#ifndef NDEBUG
void GameTime::DrawImGui()
{
    ImGui::Begin("TimeScale");
    ImGui::DragFloat("TimeScale", &timeScale, 0.01f);
    ImGui::End();

}
#endif // !NDEBUG

int GameTime::CreateTimer(float time, bool loop)
{
    timers.push_back(Timer{ 0.0f, time, loop, false });
    return static_cast<int>(timers.size()) - 1;
}

void GameTime::UpdateDeltaTime() {
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> delta = currentTime - lastTime;



    lastTime = currentTime;
    deltaTime = std::min(delta.count(), 0.05f) * timeScale;
    m_unScaledDeltaTime = std::min(delta.count(), 0.05f);
}

void GameTime::UpdateCPUUsagePDH()
{
    static PDH_HQUERY query;
    static PDH_HCOUNTER counter;

    if (!initialized) {
        PdhOpenQuery(NULL, NULL, &query);
        PdhAddCounter(query, TEXT("\\Processor(_Total)\\% Processor Time"), NULL, &counter);
        PdhCollectQueryData(query);
        initialized = true;
        cpuUsage = 0.0f;
    }

    PdhCollectQueryData(query);
    PDH_FMT_COUNTERVALUE value;
    PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);
    cpuUsage = static_cast<float>(value.doubleValue);

}

void GameTime::Update()
{
    UpdateDeltaTime();

    updateUPUUsageTimer += deltaTime;
    if (updateUPUUsageTimer >= 1.0f) {
        UpdateCPUUsagePDH();
        updateUPUUsageTimer = 0.0f;
    }

    for (int i = 0; i < timers.size(); i++)
    {
        if (!timers[i].isFinished)
        {
            timers[i].timer += deltaTime;
            if (timers[i].timer >= timers[i].maxTime)
            {
                timers[i].isFinished = true;
                timers[i].timer = timers[i].maxTime;
            }
        }
    }

}