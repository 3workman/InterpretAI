#pragma once
#include "cAI.h"

//׷�ٹ���character����ȥĳ�����һ������
class cAI_WalkAttack : public cAI{
	const bool m_bPlayerAutoKillNpc;
	const bool m_bCallPartner;
public:
	cAI_WalkAttack(iAI_Char* p, bool bCallPartner, bool bPlayerAutoKill);
	Pool_Index_Define(cAI_WalkAttack)

	void SetAttack(iAI_Char* p, int nSkillTID, int speed);	//����ĳ���˹���
	void SetAttack(CPoint ptDes, int nSkillTID, int speed);	//����ĳ���㹥��

	void CallPartner(iAI_Char* p);

	AI_Return ExceptionActiveAI() override;
};
