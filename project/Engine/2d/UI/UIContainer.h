#include "UIElement.h"
#include "SelectionGroup.h"
#pragma once

namespace UI {
    class Container
    {
    public:
        Container();
        ~Container();

        // UI要素の追加関数
        void AddElement(std::shared_ptr<UI::UIElement> element) { m_uiElements.push_back(element); }
        // セレクショングループの追加関数
        void AddSelectionGroup(std::shared_ptr<UI::SelectionGroup> group) { m_selectionGroups.push_back(group); }

        // 更新
        void Update();
        // 描画
        void Draw();

    private:
        std::vector<std::shared_ptr<UI::UIElement>> m_uiElements; // UI要素のリスト
        std::vector<std::shared_ptr<UI::SelectionGroup>> m_selectionGroups; // セレクショングループのリスト
    };

}
