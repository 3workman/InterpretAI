#include "StdAfx.h"
#include "cAI_WalkAttack.h"
#include "cAI_WalkToChar.h"
#include "cAI_Attack.h"
#include "cAI_Walk.h"
#include <list>

cAI_WalkAttack::cAI_WalkAttack(iAI_Char* p, bool bCallPartner, bool bPlayerAutoKillNpc) : cAI(p, AI_Type::Walk_Attack)
, m_bPlayerAutoKillNpc(bPlayerAutoKillNpc)
, m_bCallPartner(bCallPartner)
{
	m_vecException.push_back(-1); // 0��AI(�߶�)�쳣�л���Ч���Է��� AI_Return::Exception;
	m_vecException.push_back(0);  // 1��AI(����)�쳣����Ϊ0�ţ����� AI_Return::Continue;
}

void cAI_WalkAttack::CallPartner(iAI_Char* p)
{
	std::list<iAI_Char*> listChar;
	m_pChar->FindParnterList(listChar);

	stAIEvent event;
	event.e = AIEvent_CallParnter;
	event.pChar = p;

	for (auto& v : listChar)
	{
		if (cAI* pAI = v->GetAI())
			pAI->OnEvent(event);
	}
}

void cAI_WalkAttack::SetAttack(iAI_Char* p, int nSkillTID, int speed)
{
	if (NULL == p) return;

	if (m_bCallPartner) CallPartner(p);

	ClearAI();
	{
		cAI_WalkToChar* pAI = new cAI_WalkToChar(m_pChar, m_bPlayerAutoKillNpc, nSkillTID);
		if (NULL == pAI) return;
		pAI->SetWalkTo(p, speed, m_pChar->GetSkillRadius(nSkillTID));
		AddAI(pAI);
	}{
		cAI_Attack* pAI = new cAI_Attack(m_pChar, m_bPlayerAutoKillNpc, speed);
		if (NULL == pAI) return;
		pAI->SetAttack(p, nSkillTID);
		AddAI(pAI);
	}
	SetActiveIndex(0);
}

void cAI_WalkAttack::SetAttack(CPoint ptDes, int nSkillTID, int speed)
{
	ClearAI();
	{
		cAI_Walk* pAI = new cAI_Walk(m_pChar, m_bPlayerAutoKillNpc);
		if (NULL == pAI) return;
		pAI->SetDes(ptDes, speed, 0, 0, 0);
		AddAI(pAI);
	}{
		cAI_Attack* pAI = new cAI_Attack(m_pChar, m_bPlayerAutoKillNpc, speed);
		if (NULL == pAI) return;
		pAI->SetAttack(ptDes, nSkillTID);
		AddAI(pAI);
	}
	SetActiveIndex(0);
}

AI_Return cAI_WalkAttack::ExceptionActiveAI()
{
	if (cAI* pAI = GetActiveAI()) //����GetActiveWithUrgent()��
	{
		const bool bAttacking = pAI->GetActiveType() == AI_Type::Attack;
		if (m_bPlayerAutoKillNpc && m_pChar->IsPlayer())
		{
			if (bAttacking){ //����ʱ�쳣
				SetActiveIndex(0); // ��Ϊ�߶�
				return AI_Return::Exception; //����ֱ�ӷ����쳣��Ϊʲô�أ�
											 //��0�ŵ��쳣�л���Ч����ֱ�� return cAI::Exception()��Ҳ����Ϊ�߶��������շ���Continue
			}else{
				SetActiveIndex(1); // ��Ϊ������������
				return AI_Return::Continue;
			}
		} 
		else if (bAttacking && pAI->GetDesID() <= 0)
		{
			return AI_Return::Exception;
		}
	}

	return cAI::ExceptionActiveAI();
};