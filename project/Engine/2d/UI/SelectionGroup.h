#include "UIButton.h"
#include "UISlider.h"
#include <variant>
#include "Input.h"
#pragma once

namespace UI {
    class SelectionGroup
    {
    public:
        SelectionGroup();
        ~SelectionGroup();

        void SetInput(Input* input) { m_input = input; }

        // グループにUI要素を追加する関数
        void Add(const std::shared_ptr<Button>& button) { m_uis.push_back(button); }
        void AddSlider(const std::shared_ptr<UISlider>& slider) { m_uis.push_back(slider); }

        // グループ内の全てのUI要素を更新する関数
        void Update();

    private:
        Input* m_input = nullptr;
        using UIElementVariant = std::variant<std::shared_ptr<Button>, std::shared_ptr<UISlider>>;
        std::vector<UIElementVariant> m_uis;
        int selectedIndex = 0; // 現在選択されているUI要素のインデックス
        
        using InteractType = std::variant<BYTE, int, Controller>;
        InputBinding m_moveUpBinding; // 上移動の入力バインディング
        InputBinding m_moveDownBinding; // 下移動の入力バインディング
    };
};
