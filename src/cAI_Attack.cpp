#include "stdafx.h"
#include "cAI_Attack.h"

cAI_Attack::cAI_Attack(iAI_Char* p, bool bPlayerAutoKillNpc, int nSpeed) : cAI(p, AI_Type::Attack)
, m_refDesID(p->GetAIAttackDes())
, m_refSpeed(p->GetAISpeed())
, m_refDesPos(p->GetAIAttackPos())
, m_refSkillTID(p->GetAISkillTID())
, m_refIsAttackPos(p->IsAIAttackPos())
, m_kIsPlayerAutoKillNpc(bPlayerAutoKillNpc)
{
	m_refDesID = 0;
	m_refSpeed = nSpeed;
	m_refIsAttackPos = false;
}

/************************************************************************/
// 一些测试用接口
const int SKILL_TID_Boss60_Xp = 1;
const int SKILL_TID_ShiFoLuanWu = 2;
const int SKILL_TID_CaiJi = 3;
const int SKILL_TID_FaBaoChuZhan = 4;
struct SkillProperty{
	DWORD m_dwCD;
	DWORD m_dwCDCommon;
	DWORD bContinueUse;
};
SkillProperty* GetSkillPropertyByTID(int){ return NULL; }
bool IsSingleSkill(SkillProperty*){ return false; }
bool IsSpellSkill(int){ return false; }
/************************************************************************/

AI_Return cAI_Attack::Run()
{
	if (m_refSkillTID < 0) return AI_Return::None;

	SkillProperty* pP = GetSkillPropertyByTID(m_refSkillTID);
	if (NULL == pP) return AI_Return::Exception;

	bool bSucceedAttack = false;

	// 攻击目标点
	if (m_refIsAttackPos)
	{
		bSucceedAttack = m_pChar->Attack(m_refSkillTID, m_refDesPos);

		if (m_refSkillTID == SKILL_TID_Boss60_Xp ||
			m_refSkillTID == SKILL_TID_ShiFoLuanWu)
		{
			return AI_Return::Continue;
		}

		if (bSucceedAttack)
		{
			m_refIsAttackPos = false;

			// 群攻、buff技能只自动释放一次
			return IsSingleSkill(pP) ? AI_Return::Continue : AI_Return::SwapUrgent;
		}else{
			return AI_Return::Exception;
		}
	}

	// 攻击指定目标
	iAI_Char* pDes = m_pChar->FindChar(m_refDesID);
	if (NULL == pDes || pDes->IsDie())
	{
		m_refDesID = 0;
		return AI_Return::Exception;
	}

	const bool bIsPlayer = m_pChar->IsPlayer();
	if (bIsPlayer && IsSpellSkill(m_refSkillTID))
	{
		// 采集等吟唱技能需要特殊处理
		if (SKILL_TID_CaiJi == m_refSkillTID)
		{
			m_pChar->Attack(m_refSkillTID, pDes);
			return AI_Return::SwapUrgent;
		}
		else if (m_pChar->IsSpelling())
		{
			return AI_Return::Continue;
		}
	}

	if (!bIsPlayer && pDes->IsPlayer() && pDes->IsHide()) //npc不打隐身玩家
	{
		m_refDesID = 0;
		return AI_Return::Exception;
	}

	//远程攻击是否碰到阻挡,或者距离不够了
	const bool bInRange = m_pChar->CanAttackRange(m_refSkillTID, pDes);
	static int s_nOutRange = 0;
	if (bIsPlayer && m_kIsPlayerAutoKillNpc)
	{
		if (bInRange){
			s_nOutRange = 0;
		}else{
			if (++s_nOutRange > 250) // 走了250步没成功就放弃
			{
				s_nOutRange = 0;
				return AI_Return::Exception;
			}

			CPoint pA = pDes->GetCharPos();
			CPoint pB = m_pChar->GetCharPos();
			CPoint pt;
			int speed = m_refSpeed;
			int dir = m_pChar->FindPath(pB, pA, pt, speed);
			if (dir == -1) return AI_Return::Exception;
			m_pChar->CharMove(dir, speed);
			return AI_Return::Continue;
		}
	}else if (!bInRange)
		return AI_Return::Exception;

	if (bIsPlayer && m_kIsPlayerAutoKillNpc)
	{
		const int nAoeSkillTID = m_pChar->GetAutoAttackAoeSkill();
		const int nSingleSkillTID = m_pChar->GetAutoAttackSingleSkill();
		SkillProperty* pAoeSkill = GetSkillPropertyByTID(nAoeSkillTID);
		SkillProperty* pSingleSkill = GetSkillPropertyByTID(nSingleSkillTID);
		bool bCanUseAoe = pAoeSkill && m_pChar->CanUseSkillAttack(nAoeSkillTID);
		bool bCanUseSingle = pSingleSkill && m_pChar->CanUseSkillAttack(nAoeSkillTID);

		if (bCanUseAoe && pAoeSkill->m_dwCD)
		{
			DWORD dwTimePast = TimeNow_Msec - m_pChar->GetLastTimeUseSkill(nAoeSkillTID);
			if (dwTimePast < pAoeSkill->m_dwCD)
			{
				bCanUseAoe = false;
			}
		}

		if (bCanUseSingle && pSingleSkill->m_dwCD)
		{
			DWORD dwTimePast = TimeNow_Msec - m_pChar->GetLastTimeUseSkill(nSingleSkillTID);
			if (dwTimePast < pSingleSkill->m_dwCD)
			{
				bCanUseSingle = false;
			}
		}

		if (bCanUseAoe)//优先群攻
			m_refSkillTID = nAoeSkillTID;
		else if (bCanUseSingle)
			m_refSkillTID = nSingleSkillTID;
		else
			m_refSkillTID = m_pChar->GetSingleAttackSkill_MaxLv();

		//自动战斗获取技能，指针要重新生成
		pP = GetSkillPropertyByTID(m_refSkillTID);
		if (!pP) return AI_Return::Exception;
	}
	else //不是自动战斗
	{
		DWORD dwTimePast = TimeNow_Msec - m_pChar->GetLastTimeUseSkill(m_refSkillTID);
		if (pP->m_dwCDCommon && !bIsPlayer)
		{
			if (dwTimePast < m_pChar->GetCommonCD())
			{
				return AI_Return::Continue;
			}
		}
		if (dwTimePast < pP->m_dwCD)
		{
			return AI_Return::Continue;
		}
	}

	bSucceedAttack = m_pChar->Attack(m_refSkillTID, pDes);

	if (bIsPlayer && m_kIsPlayerAutoKillNpc)
	{
		// 群攻、buff技能只自动释放一次
		if (bSucceedAttack && !IsSingleSkill(pP))
			return AI_Return::SwapUrgent;
		else
			return AI_Return::Continue;
	}
	else //不是自动战斗
	{
		if (!bSucceedAttack)
			return AI_Return::Continue;
		else if (pP->bContinueUse) //是否连续技能
			return AI_Return::Continue;
		else{
			// Npc技能释放成功后，切为普攻
			if (!bIsPlayer) m_pChar->SetAINormalSkill();

			return AI_Return::SwapUrgent;
		}
	}

	return AI_Return::Continue;
}