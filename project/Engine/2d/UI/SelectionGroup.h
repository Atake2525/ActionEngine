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
        void SetMoveUpBinding(const InputTrigger& trigger) { m_moveUpBinding.triggers.push_back(trigger); }
        void SetMoveDownBinding(const InputTrigger& trigger) { m_moveDownBinding.triggers.push_back(trigger); }

        // UI全体のインタラクトバインドを設定
        void SetAllInteractBinding(const InputTrigger& trigger);

        // グループにUI要素を追加する関数
        void Add(std::shared_ptr<UIElement> button) { m_uis.push_back(button); }

        // グループ内の全てのUI要素を更新する関数
        void Update();

        // グループ内の全てのUI要素を描画する関数
        void Draw();

    private:
        Input* m_input = nullptr;
        using UIElementVariant = std::variant<std::shared_ptr<Button>, std::shared_ptr<UISlider>>;
        std::vector<std::shared_ptr<UIElement>> m_uis;
        int selectedIndex = 0; // 現在選択されているUI要素のインデックス
        int selectedIndexPre = 0; // 前のフレームで選択されていたUI要素のインデックス

        using InteractType = std::variant<BYTE, int, Controller>;
        InputBinding m_moveUpBinding; // 上移動の入力バインディング
        InputBinding m_moveDownBinding; // 下移動の入力バインディング
    };
};
