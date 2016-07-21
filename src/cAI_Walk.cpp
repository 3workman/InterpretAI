#include "stdafx.h"
#include "cAI_Walk.h"

Pool_Index_Ini(cAI_Walk, 32)

cAI_Walk::cAI_Walk(iAI_Char* p, bool bPlayerAutoKill) : cAI(p, AI_Type::Walk)
, m_refSpeed(p->GetAISpeed())
, m_refSkillTID(p->GetAISkillTID())
, m_bPlayerAutoKillNpc(bPlayerAutoKill)
{}

AI_Return cAI_Walk::Run()
{
	if (IsTimeout()) return AI_Return::Exception;

	CPoint ptSelf = m_pChar->GetCharPos();
	if (m_pCheckStopFun && m_pCheckStopFun(ptSelf, m_ptDes))
	{
		if (m_pStopFun) m_pStopFun(m_pChar, m_nNpcID);

		return AI_Return::Exception;
	}

	static int s_nTryNum = 0;
	bool bInView = InDistance(m_ptDes, ptSelf, m_nRange);
	if (bInView)
	{
		if (m_nDesID > 0 && m_nRange > 2){
			iAI_Char* pChar = m_pChar->FindChar(m_nDesID);
			if (pChar == NULL) return AI_Return::Exception;

			if (m_pChar->CanAttackRange(m_refSkillTID, pChar))
			{
				s_nTryNum = 0;
				return AI_Return::Next;
			}
		}else{
			s_nTryNum = 0;
			return AI_Return::Next;
		}
		bInView = false;
	}

	static CPoint s_ptOut;
	int nspeed = m_refSpeed;
	bool bNeedAddTryNum = (!bInView && m_pChar->IsPlayer() && m_bPlayerAutoKillNpc);
	if (bNeedAddTryNum && s_nTryNum > 60)
	{
		if (s_nTryNum == 61){
			s_ptOut = ptSelf;
			int x = rand() % 5 + 20;
			int y = rand() % 5 + 20;
			m_ptDes.x > ptSelf.x ? s_ptOut.x -= x : s_ptOut.x += x;
			m_ptDes.y > ptSelf.y ? s_ptOut.y -= y : s_ptOut.y += y;
			return AI_Return::Continue;
		}else{
			if (ptSelf == s_ptOut)
			{
				s_nTryNum = 0;
				return AI_Return::Exception;
			}
			if (s_nTryNum > 120)
			{
				s_nTryNum = 0;
				return AI_Return::Exception;
			}
			CPoint temp;
			int dir = m_pChar->FindPath(ptSelf, s_ptOut, temp, nspeed);
			if (dir == -1) return AI_Return::Exception;
			m_pChar->CharMove(dir, nspeed);
			return AI_Return::Continue;
		}
	}

	CPoint temp;
	int dir = m_pChar->FindPath(ptSelf, m_ptDes, temp, nspeed);
	if (dir == -1) return AI_Return::Exception;
	if (bNeedAddTryNum && m_pChar->CharMove(dir, nspeed)) // 移动成功了才增加尝试次数
	{
		++s_nTryNum;
	}
	
	if (m_bChangeAI)
	{
		if (++m_nStepNum >= 1) // 多少步一切
		{
			m_nStepNum = 0;
			return AI_Return::Next;
		}
	}

	return AI_Return::Continue;
}