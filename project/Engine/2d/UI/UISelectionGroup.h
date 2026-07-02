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

        // 入力の設定関数
        void SetInput(Input* input) { m_input = input; }
        void SetMoveUpBinding(const InputTrigger& trigger) { m_moveUpBinding.triggers.push_back(trigger); }
        void SetMoveDownBinding(const InputTrigger& trigger) { m_moveDownBinding.triggers.push_back(trigger); }

        // 表示状態の設定関数
        void Show() { for (auto ui : m_uis) { ui->Show(); } }
        void Hide() { for (auto ui : m_uis) { ui->Hide(); } }
        void ShowThisFrame() { for (auto ui : m_uis) { ui->ShowThisFrame(); } }
        void HideThisFrame() { for (auto ui : m_uis) { ui->HideThisFrame(); } }

        // UI要素の使用回数を設定する
        void SetUsableCount(int count);
        // UI要素の使用回数を取得する
        const int GetUsableCount() const { return m_usableCount; }

        // UI全体のインタラクトバインドを設定
        void SetInteractBinding(const InputTrigger& trigger);

        // グループにUI要素を追加する関数
        void Add(std::shared_ptr<Element> button);
        void Add(std::unique_ptr<Element> button);

        // グループ内の全てのUI要素を更新する関数
        void Update();
        // グループ内の全てのUI要素を描画する関数
        void Draw();

    private:
        Input* m_input = nullptr;
        std::vector<std::shared_ptr<Element>> m_uis;
        int m_selectedIndex = 0; // 現在選択されているUI要素のインデックス
        int m_selectedIndexPre = 0; // 前のフレームで選択されていたUI要素のインデックス

        InputBinding m_interactBinding; // インタラクト用バインディング

        InputBinding m_moveUpBinding; // 上移動の入力バインディング
        InputBinding m_moveDownBinding; // 下移動の入力バインディング

        bool m_firstUpdate = true; // 最初の更新かどうかを示すフラグ

        int m_usableCount = -1; // 何回選択できるかを管理する変数(-1のときは無限に選択できる)
        void CheckUsable(); // UI要素のアクティベーションをチェックする関数
    };
};
