#include "PhysicsCorrection.h"

using namespace PhysicsEngine;

void PhysicsCorrection::Update()
{
	for (size_t i = 0; i < m_correctionObjects.size(); i++)
	{
		CorrectionObject& obj = m_correctionObjects[i];
		if (obj.useCCD) // 連続的衝突検出を使用する場合の処理
		{

		}
        else // 通常の衝突検出
		{

		}
	}
}

void PhysicsCorrection::UpdateObject()
{
}
