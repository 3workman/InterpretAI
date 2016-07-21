#include "stdafx.h"
#include "cAI_WalkToChar.h"
#include "cAI_Walk.h"

Pool_Index_Ini(cAI_WalkToChar, 32)

cAI_WalkToChar::cAI_WalkToChar(iAI_Char* p, bool bPlayerAutoKillNpc, int nSkill) : cAI(p, AI_Type::Walk_ToChar)
, m_refSkillTID(p->GetAISkillTID())
, m_bPlayerAutoKillNpc(bPlayerAutoKillNpc)
{
	m_refSkillTID = nSkill;
}

bool cAI_WalkToChar::SetWalkTo(iAI_Char* p, int speed, int range)
{
	if (NULL == p) return false;
	ClearAI();
	m_nDesID = p->GetID();

	cAI_Walk* pAI = new cAI_Walk(m_pChar, m_bPlayerAutoKillNpc);
	if (NULL == pAI) return false;
	pAI->SetDes(p->GetCharPos(), speed, range, m_nDesID, m_refSkillTID);
	AddAI(pAI);
	SetActiveIndex(0);
	return true;
}

AI_Return cAI_WalkToChar::Run()
{
	iAI_Char* pDes = m_pChar->FindChar(m_nDesID);
	if (NULL == pDes) return AI_Return::Exception;

	//×·»÷°ë¾¶
	if (Distance(m_pChar->GetCharPos(), pDes->GetCharPos()) > m_pChar->GetFollowRadius())
		return AI_Return::GoBack;

	//×·»÷·¶Î§
	if (!m_pChar->IsPlayer()
		&& Distance(m_pChar->GetInitPos(), m_pChar->GetCharPos()) > m_pChar->GetFollowRange()
		)
	{
		return AI_Return::GoBack;
	}

	cAI_Walk* pWalkAI = dynamic_cast<cAI_Walk*>(GetActiveAI());
	if (NULL == pWalkAI) return AI_Return::Exception;
	const CPoint ptDes = pDes->GetCharPos();
	const CPoint ptWalk = pWalkAI->GetDesPos();
	if (ptDes != ptWalk) pWalkAI->ResetDes(ptDes);

	return cAI::Run();
}
