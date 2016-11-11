#pragma once
#include "cAI.h"

//追踪攻击character或跑去某个点放一个技能
class cAI_WalkAttack : public cAI{
    Pool_Index_Define(cAI_WalkAttack, 32)
	const bool m_bPlayerAutoKillNpc;
	const bool m_bCallPartner;
public:
	cAI_WalkAttack(iAI_Char* p, bool bCallPartner, bool bPlayerAutoKill);

	void SetAttack(iAI_Char* p, int nSkillTID, int speed);	//走向某个人攻击
	void SetAttack(CPoint ptDes, int nSkillTID, int speed);	//走向某个点攻击

	void CallPartner(iAI_Char* p);

	AI_Return ExceptionActiveAI() override;
};
