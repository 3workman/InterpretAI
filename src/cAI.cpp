#include "stdafx.h"
#include "cAI.h"

cAI::cAI(iAI_Char* p, AI_Type eType) : m_pChar(p), m_eAIType(eType)
{
	m_nAcitveIndex = 0;
	m_nTime = 0;
	m_nTimeEnd = 0;

	m_pUrgent = NULL;
	m_pUergentEndFun = NULL;
	m_pUergentBeginFun = NULL;
	m_pUrgentEndParam = NULL;
	m_pUrgentBeginParam = NULL;

	m_pSelfEndFun= NULL;
	m_pSelfBeginFun = NULL;
	m_pSelfEndParam = NULL;
	m_pSelfBeginParam = NULL;
}

AI_Return cAI::OnReturn(AI_Return rt)
{
	switch (rt){
	case AI_Return::Next:{
			size_t index = m_nAcitveIndex < m_vecNext.size() ? m_vecNext[m_nAcitveIndex] : m_nAcitveIndex + 1;
			if (SetActiveIndex(index)) return AI_Return::Continue;
			return AI_Return::Next;
		}break;
	case AI_Return::Exception:			return ExceptionActiveAI();
	case AI_Return::CancelTarget:		return AI_Return::Exception;
	case AI_Return::None: assert(0);	return AI_Return::Continue;
	default: return rt;
	}
}
AI_Return cAI::ExceptionActiveAI()//根据跳转表来处理异常
{
	if (m_nAcitveIndex < m_vecException.size() && SetActiveIndex(m_vecException[m_nAcitveIndex]))
		return AI_Return::Continue;
	return AI_Return::Exception;
}
AI_Return cAI::Run()
{
	if (m_pUrgent) //有紧急动作
	{
		const AI_Return rt = m_pUrgent->Run();
		if (rt != AI_Return::Continue && rt != AI_Return::SwapUrgent)
		{
			UrgentEnd();
		}
		return AI_Return::Continue;
	}

	if (IsTimeout()) return AI_Return::Exception;

	if (cAI* p = GetActiveAI())
	{
		return OnReturn( p->Run() );
	}
	return AI_Return::Next;
}
AI_Return cAI::OnEvent(const stAIEvent& event)
{
	if (cAI* p = GetActiveAI())
	{
		return OnReturn( p->OnEvent(event) );
	}
	return AI_Return::Continue;
}

/************************************************************************/
// 示例
iAI_Char* FindCharByIndex(int index){ return NULL; } //内存池取角色的接口

DWORD Service_NPCAI(void* p) //注册进ServicePatch，循环跑
{ 
	if (iAI_Char* pChar = FindCharByIndex((int)(long long)p)){
		if (cAI* pAI = pChar->GetAI())
		{
			pAI->m_bRuning = true;
			const AI_Return ret = pAI->Run();
			pAI->m_bRuning = false;

			if (ret != AI_Return::Continue){ //AI结果不是继续就清空AI
				pAI->ClearAI();
				pChar->SetAI(NULL);
			}
		}
	}
	return 0;
}