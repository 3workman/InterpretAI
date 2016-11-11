#pragma once
#include "cAI.h"

class cAI_Attack : public cAI{
    Pool_Index_Define(cAI_Attack, 32) // 声明内存池
	const bool m_kIsPlayerAutoKillNpc;
	int& m_refDesID;
	int& m_refSpeed;
	int& m_refSkillTID;
	CPoint& m_refDesPos;
	bool& m_refIsAttackPos;
public:
	cAI_Attack(iAI_Char* p, bool bPlayerAutoKillNpc, int nSpeed);
	Pool_Index_Define(cAI_Attack) // 声明内存池

	void SetAttack(iAI_Char* pChar, int nSkillTID)
	{
		m_refIsAttackPos = false;
		m_refSkillTID = nSkillTID;
		if (pChar) m_refDesID = pChar->GetID();
	}
	void SetAttack(CPoint point, int nSkillTID)
	{
		m_refIsAttackPos = true;
		m_refDesID = 0;
		m_refDesPos = point;
		m_refSkillTID = nSkillTID;
	}
	int GetSkillTID() { return m_refSkillTID; }

	AI_Return Run() override;
	int GetDesID() override { return m_refDesID; }
	int GetSpeed() override { return m_refSpeed; }
	CPoint GetDesPos() override { return CPoint(-1, -1); }
	AI_Type GetActiveType() override { return m_eAIType; }
};
