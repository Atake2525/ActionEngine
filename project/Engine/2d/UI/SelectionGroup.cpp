#include "SelectionGroup.h"

using namespace UI;

SelectionGroup::SelectionGroup()
{}

SelectionGroup::~SelectionGroup()
{}

void SelectionGroup::Update() {
	if (m_moveUpBinding.Check(*m_input))
	{
        selectedIndex--;
	}
	if (m_moveDownBinding.Check(*m_input))
	{
        selectedIndex++;
	}
}