#include "UIContainer.h"

using namespace UI;

Container::Container() {}

Container::~Container() {}

void Container::Update() {
    for (auto& group : m_selectionGroups)
    {
        group->Update(); // セレクショングループの更新
    }
    for (auto& element : m_uiElements)
    {
        element->Update(); // UI要素の更新
    }
}

void Container::Draw() {
    for (auto& group : m_selectionGroups)
    {
        group->Draw(); // セレクショングループの描画
    }
    for (auto& element : m_uiElements)
    {
        element->Draw(); // UI要素の描画
    }
}