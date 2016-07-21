#pragma once
#include <windows.h>
#include <atltypes.h> //CPoint

class cAI;
class iAI_Char{ //����AI��ɫ�Ļ�������
public:
	//Ϊ��AIͬ�����ܡ���������
	virtual cAI* GetAI(){ return NULL; }
	virtual void SetAI(cAI*){}
	virtual int& GetAISkillTID(){ static int s_nValue = 0; return s_nValue; }
	virtual int& GetAISpeed(){ static int s_nValue = 0; return s_nValue; }
	virtual int& GetAIAttackDes(){ static int s_nValue = 0; return s_nValue; }
	virtual bool& IsAIAttackPos(){ static bool s_nValue = false; return s_nValue; }
	virtual CPoint& GetAIAttackPos(){ static CPoint s_ptValue; return s_ptValue; }

	virtual void SetAINormalSkill(){}
	virtual int GetSkillLevel(int nSkillTID){ return 1; }

	virtual int GetID() const { return 0; }
	virtual int GetTrueID() const { return 0; }
	virtual bool IsDie() const { return false; }
	virtual bool IsHide() const { return false; }
	virtual bool IsPlayer() const { return true; }
	virtual CPoint GetCharPos(){ return CPoint(-1, -1); } //���ڵ�
	virtual CPoint GetInitPos(){ return CPoint(0, 0); }   //������
	virtual iAI_Char* FindChar(int id){ return NULL; }
    virtual iAI_Char* FindEnemy(){ return NULL; }
	virtual int FindPath(CPoint pos, CPoint Des, CPoint& ptNext, int& speed) { return 0; }
	virtual bool IsStaticObstacle(CPoint pos) { return true; }
    virtual bool HaveBuff(DWORD buffTID) { return false; }
    virtual bool AddBuff(DWORD buffTID, DWORD leftTime, WORD lv, float val1, float val2) { return false; }
    virtual bool ClearBuff(DWORD buffTID) { return false; }

	virtual DWORD GetCommonCD(){ return 0; } //����CD�湥���ٶ���߶�����
	virtual DWORD GetLastTimeUseSkill(int nSkillTID){ return 0; }

	virtual bool CharMove(int dir, int speed) { return true; }
	virtual bool Attack(int nSkillTID, iAI_Char* p) { return true; }
	virtual bool Attack(int nSkillTID, CPoint point) { return true; }
	virtual int GetFollowRange() { return 0; }	//׷����Χ��������������Χ
	virtual int GetFollowRadius() { return 0; }	//׷���뾶���������ڵ����Χ
	virtual int GetSkillRadius(int nSkillTID) { return 1; }
	virtual bool CanUseSkillAttack(int nSkillTID){ return true; }
	virtual bool CanAttackRange(int nSkillTID, iAI_Char* p){ return true; }
	virtual int GetAutoAttackAoeSkill(){ return 0; }
	virtual int GetAutoAttackSingleSkill(){ return 0; }
	virtual int GetSingleAttackSkill_MaxLv(){ return 0; }

	virtual bool IsSpelling(){ return false; } //�Ƿ���������
	virtual bool IsPauseAI() { return false; }

	virtual void FindParnterList(std::list<iAI_Char*>& list) { return; }
};

enum AIEventEnum{
	AIEvent_Null,
	AIEvent_BeAttack,	 //������
	AIEvent_Die,		 //char����
	AIEvent_MapChange,   //char��ͼ�ı�
	AIEvent_CallParnter, //����ͬ��
	AIEvent_ItemDrop,	 //���ϵ��˸���Ʒ
	AIEvent_InterAction, //�������Ի���
};
struct stAIEvent
{
	AIEventEnum e;
	iAI_Char* pChar;
	CPoint pos;
	long param;
	stAIEvent(){ memset(this, 0, sizeof(*this)); }
};

inline int Distance(const POINT& p1, const POINT& p2)
{
	return max(abs(p1.x - p2.x), abs(p1.y - p2.y));
}
inline bool InDistance(const POINT& p1, const POINT& p2, int d)
{
	return Distance(p1, p2) <= d;
}
inline bool IsOutOfFollowRange(iAI_Char* pSelf, iAI_Char* pTarget) //׷����Χ��������������Χ
{
	if (pSelf && pTarget)
	{
		return Distance(pSelf->GetInitPos(), pTarget->GetCharPos()) > pSelf->GetFollowRange();
	}
	return true;
}