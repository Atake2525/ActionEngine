#include "UIContainer.h"

using namespace UI;

Container::Container() {}

Container::~Container() {}

void Container::Update() {
    for (auto& group : m_selectionGroups)
    {
        group->Update(); // セレクショングループの更新
    }
}

void Container::Draw() {
    for (auto& group : m_selectionGroups)
    {
        group->Draw(); // セレクショングループの描画
    }
}