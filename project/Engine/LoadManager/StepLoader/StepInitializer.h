#include <functional>
#include <vector>

#pragma once


class StepInitializer
{
public:
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize();
    
    /// <summary>
    /// 更新
    /// </summary>
    void Update();
    
    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    void AddStep(const std::function<void()>& step) {
        steps_.push_back(step);
        stepsTotal = static_cast<int>(steps_.size());
    }

    const bool IsFinished() const {
        if (stepCount >= stepsTotal)
        {
            return true;
        }
        return false;
    }

private:
    int stepCount = 0;
    int stepsTotal = 0;

    std::vector<std::function<void()>> steps_;

};

