#pragma once
#include "cAI.h"

//走向某一点
class cAI_Walk : public cAI{
protected:
	int m_nDesID = 0;
	int m_nNpcID = 0;
	int m_nRange = 0;
	CPoint m_ptDes; //不是直接的目的地，而可能是很远的一个地点
	int& m_refSpeed;
	int& m_refSkillTID;
	int  m_nStepNum = 0;
	bool m_bChangeAI = false;
	const bool m_bPlayerAutoKillNpc;
	typedef bool(*StopCheck)(CPoint, CPoint);
	typedef void(*StopCallback)(iAI_Char*, int);
	StopCallback m_pStopFun = NULL;
	StopCheck m_pCheckStopFun = NULL;
public:
	cAI_Walk(iAI_Char* p, bool bPlayerAutoKill);
	Pool_Index_Define(cAI_Walk)

	void SetDes(CPoint ptDes, int speed, int range, int id, int nSkill)
	{
		m_ptDes = ptDes;
		m_nRange = range;
		m_nDesID = id;
		m_refSpeed = speed;
		m_refSkillTID = nSkill;
	}
	void ResetDes(CPoint ptDes){ m_ptDes = ptDes; Init(); }

	void SetStopCheck(StopCheck p){ m_pCheckStopFun = p; }
	void SetStopCallback(StopCallback p){ m_pStopFun = p; }

	void SetDesNpcID(int id){ m_nNpcID = id; }
	void OpenChangeAI(){ m_bChangeAI = true; }

	AI_Return Run() override;
	void Init() override { m_nStepNum = 0; }
	int GetDesID() override { return m_nDesID; }
	int GetSpeed() override { return m_refSpeed; }
	CPoint GetDesPos() override { return m_ptDes; }
	AI_Type GetActiveType() override { return m_eAIType; }
};

class cAI_WalkRand : public cAI_Walk{
protected:
	CPoint m_ptAround;
	DWORD  m_nRadius = 0;
public:
	cAI_WalkRand(iAI_Char* p) : cAI_Walk(p, false){ const_cast<AI_Type>(m_eAIType) = AI_Type::Walk_Rand; }

	void SetAround(CPoint pt, DWORD radius, int speed)
	{
		m_ptAround = pt;
		m_nRadius = radius;
		m_refSpeed = speed;
	}
	void Init() override
	{
		cAI_Walk::Init();
		CPoint pt = m_ptAround;
		do{
			pt.x += rand() % (2 * m_nRadius) - m_nRadius;
			pt.y += rand() % (2 * m_nRadius) - m_nRadius;
		} while (m_pChar->IsStaticObstacle(pt));
		SetDes(m_ptAround, m_refSpeed, 1, m_nDesID, m_refSkillTID);
	}
};