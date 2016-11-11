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
// һЩ�����ýӿ�
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

	// ����Ŀ���
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

			// Ⱥ����buff����ֻ�Զ��ͷ�һ��
			return IsSingleSkill(pP) ? AI_Return::Continue : AI_Return::SwapUrgent;
		}else{
			return AI_Return::Exception;
		}
	}

	// ����ָ��Ŀ��
	iAI_Char* pDes = m_pChar->FindChar(m_refDesID);
	if (NULL == pDes || pDes->IsDie())
	{
		m_refDesID = 0;
		return AI_Return::Exception;
	}

	const bool bIsPlayer = m_pChar->IsPlayer();
	if (bIsPlayer && IsSpellSkill(m_refSkillTID))
	{
		// �ɼ�������������Ҫ���⴦��
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

	if (!bIsPlayer && pDes->IsPlayer() && pDes->IsHide()) //npc�����������
	{
		m_refDesID = 0;
		return AI_Return::Exception;
	}

	//Զ�̹����Ƿ������赲,���߾��벻����
	const bool bInRange = m_pChar->CanAttackRange(m_refSkillTID, pDes);
	static int s_nOutRange = 0;
	if (bIsPlayer && m_kIsPlayerAutoKillNpc)
	{
		if (bInRange){
			s_nOutRange = 0;
		}else{
			if (++s_nOutRange > 250) // ����250��û�ɹ��ͷ���
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

		if (bCanUseAoe)//����Ⱥ��
			m_refSkillTID = nAoeSkillTID;
		else if (bCanUseSingle)
			m_refSkillTID = nSingleSkillTID;
		else
			m_refSkillTID = m_pChar->GetSingleAttackSkill_MaxLv();

		//�Զ�ս����ȡ���ܣ�ָ��Ҫ��������
		pP = GetSkillPropertyByTID(m_refSkillTID);
		if (!pP) return AI_Return::Exception;
	}
	else //�����Զ�ս��
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
		// Ⱥ����buff����ֻ�Զ��ͷ�һ��
		if (bSucceedAttack && !IsSingleSkill(pP))
			return AI_Return::SwapUrgent;
		else
			return AI_Return::Continue;
	}
	else //�����Զ�ս��
	{
		if (!bSucceedAttack)
			return AI_Return::Continue;
		else if (pP->bContinueUse) //�Ƿ���������
			return AI_Return::Continue;
		else{
			// Npc�����ͷųɹ�����Ϊ�չ�
			if (!bIsPlayer) m_pChar->SetAINormalSkill();

			return AI_Return::SwapUrgent;
		}
	}

	return AI_Return::Continue;
}