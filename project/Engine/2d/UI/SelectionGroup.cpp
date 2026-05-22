#include "SelectionGroup.h"

using namespace UI;

SelectionGroup::SelectionGroup()
{}

SelectionGroup::~SelectionGroup()
{}

void SelectionGroup::Update() {
    if (m_input != nullptr && !m_uis.empty())
    {
        if (m_firstUpdate)
        {
            m_uis[m_selectedIndex]->SetSelected(true); // 最初のフレームで最初のUI要素を選択状態にする
            m_firstUpdate = false; // 最初の更新が終わったのでフラグを下ろす
        }
        CheckUsable(); // UI要素の使用可能回数をチェックする
        if (m_moveUpBinding.CheckTrigger(*m_input))
        {
            m_selectedIndexPre = m_selectedIndex; // 前の選択インデックスを保存する
            m_selectedIndex--;
        }
        if (m_selectedIndex < 0)
        {
            m_selectedIndex = static_cast<int>(m_uis.size() - 1);
        }
        if (m_moveDownBinding.CheckTrigger(*m_input))
        {
            m_selectedIndexPre = m_selectedIndex; // 前の選択インデックスを保存する
            m_selectedIndex++;
        }
        if (m_selectedIndex == m_uis.size())
        {
            m_selectedIndex = 0;
        }

        if (m_selectedIndex != m_selectedIndexPre)
        {
            m_uis[m_selectedIndexPre]->SetSelected(false); // 前の選択されていたUI要素を非選択状態にする
            m_uis[m_selectedIndex]->SetSelected(true); // 現在選択されているUI要素を選択状態にする
        }
        for (auto ui : m_uis)
        {
            ui->Update(); // グループ内の全てのUI要素を更新する
        }
    }
}

void SelectionGroup::Draw() {
    for (auto ui : m_uis)
    {
        ui->Draw(); // グループ内の全てのUI要素を描画する
    }
}

void UI::SelectionGroup::SetUsableCount(int count) {
    m_usableCount = count;
    for (auto ui : m_uis)
    {
        ui->SetUsableCount(count); // グループ内の全てのUI要素の使用回数を設定する
    }
}

void SelectionGroup::SetAllInteractBinding(const InputTrigger& trigger) {
    for (auto ui : m_uis)
    {
        ui->AddInteractionBinding(trigger);
    }
}

void SelectionGroup::CheckUsable() {
    for (auto ui : m_uis)
    {
        if (ui->IsActivated())
        {
            if (m_usableCount > 0)
            {
                m_usableCount--; // 使用回数を減らす
            }
            if (m_usableCount == 0)
            {
                for (auto ui : m_uis)
                {
                    ui->SetUsableCount(0); // 全てのUI要素の使用回数を0にする
                };
                return;
            }
        }
    }
}