#include "UIElement.h"
#include "UISelectionGroup.h"
#pragma once

namespace UI {
    class Container
    {
    public:
        Container();
        ~Container();

        // セレクショングループの追加関数
        void Add(std::shared_ptr<UI::SelectionGroup> group) { m_selectionGroups.push_back(group); }

        // 更新
        void Update();
        // 描画
        void Draw();

    private:
        std::vector<std::shared_ptr<UI::SelectionGroup>> m_selectionGroups; // セレクショングループのリスト
    };

}
