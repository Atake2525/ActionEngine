#include "StepInitializer.h"
#include "Logger.h"
#include <string>

using namespace Logger;
using namespace std;

void StepInitializer::Initialize() {
    stepCount = 0;
    stepsTotal = 0;
    steps_.clear();

}

void StepInitializer::Update() {
    for (int i = 0; i < steps_.size(); i++)
    {
        if (stepCount == i)
        {
            if (steps_[stepCount])
            {
                steps_[stepCount]();
                string str = "ステップ" + std::to_string(stepCount);
                str += "完了\n";
                Log(str);
                stepCount++;
            }
        }
    }
}

void StepInitializer::Draw() {
   
}