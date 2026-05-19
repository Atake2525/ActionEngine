#include "SelectionGroup.h"

using namespace UI;

SelectionGroup::SelectionGroup()
{}

SelectionGroup::~SelectionGroup()
{}

void SelectionGroup::Update() {
    for (auto ui : m_uis)
    {
        if (ui->IsActivated())
        {
            return;
        }
    }
    if (m_moveUpBinding.CheckTrigger(*m_input))
    {
        selectedIndexPre = selectedIndex; // 前の選択インデックスを保存する
        selectedIndex--;
    }
    if (selectedIndex < 0)
    {
        selectedIndex = static_cast<int>(m_uis.size() - 1);
    }
    if (m_moveDownBinding.CheckTrigger(*m_input))
    {
        selectedIndexPre = selectedIndex; // 前の選択インデックスを保存する
        selectedIndex++;
    }
    if (selectedIndex == m_uis.size())
    {
        selectedIndex = 0;
    }

    if (selectedIndex != selectedIndexPre)
    {
        m_uis[selectedIndexPre]->SetSelected(false); // 前の選択されていたUI要素を非選択状態にする
        m_uis[selectedIndex]->SetSelected(true); // 現在選択されているUI要素を選択状態にする
    }
    for (auto ui : m_uis)
    {
        ui->Update(); // グループ内の全てのUI要素を更新する
    }
}

void SelectionGroup::Draw() {
    for (auto ui : m_uis)
    {
        ui->Draw(); // グループ内の全てのUI要素を描画する
    }
}

void SelectionGroup::SetAllInteractBinding(const InputTrigger& trigger) {
    for (auto ui : m_uis)
    {
        ui->AddInteractionBinding(trigger);
    }
}