#pragma once
#include <windows.h>
#include <atltypes.h> //CPoint

class cAI;
class iAI_Char{ //定义AI角色的基本操作
public:
	//为了AI同步技能、攻击数据
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
	virtual CPoint GetCharPos(){ return CPoint(-1, -1); } //所在点
	virtual CPoint GetInitPos(){ return CPoint(0, 0); }   //出生点
	virtual iAI_Char* FindChar(int id){ return NULL; }
    virtual iAI_Char* FindEnemy(){ return NULL; }
	virtual int FindPath(CPoint pos, CPoint Des, CPoint& ptNext, int& speed) { return 0; }
	virtual bool IsStaticObstacle(CPoint pos) { return true; }
    virtual bool HaveBuff(DWORD buffTID) { return false; }
    virtual bool AddBuff(DWORD buffTID, DWORD leftTime, WORD lv, float val1, float val2) { return false; }
    virtual bool ClearBuff(DWORD buffTID) { return false; }

	virtual DWORD GetCommonCD(){ return 0; } //公共CD随攻击速度提高而缩短
	virtual DWORD GetLastTimeUseSkill(int nSkillTID){ return 0; }

	virtual bool CharMove(int dir, int speed) { return true; }
	virtual bool Attack(int nSkillTID, iAI_Char* p) { return true; }
	virtual bool Attack(int nSkillTID, CPoint point) { return true; }
	virtual int GetFollowRange() { return 0; }	//追击范围：距离出生点最大范围
	virtual int GetFollowRadius() { return 0; }	//追击半径：距离所在点最大范围
	virtual int GetSkillRadius(int nSkillTID) { return 1; }
	virtual bool CanUseSkillAttack(int nSkillTID){ return true; }
	virtual bool CanAttackRange(int nSkillTID, iAI_Char* p){ return true; }
	virtual int GetAutoAttackAoeSkill(){ return 0; }
	virtual int GetAutoAttackSingleSkill(){ return 0; }
	virtual int GetSingleAttackSkill_MaxLv(){ return 0; }

	virtual bool IsSpelling(){ return false; } //是否在吟唱中
	virtual bool IsPauseAI() { return false; }

	virtual void FindParnterList(std::list<iAI_Char*>& list) { return; }
};

enum AIEventEnum{
	AIEvent_Null,
	AIEvent_BeAttack,	 //被攻击
	AIEvent_Die,		 //char死亡
	AIEvent_MapChange,   //char地图改变
	AIEvent_CallParnter, //呼唤同伴
	AIEvent_ItemDrop,	 //地上掉了个物品
	AIEvent_InterAction, //有人来对话了
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
inline bool IsOutOfFollowRange(iAI_Char* pSelf, iAI_Char* pTarget) //追击范围：距离出生点最大范围
{
	if (pSelf && pTarget)
	{
		return Distance(pSelf->GetInitPos(), pTarget->GetCharPos()) > pSelf->GetFollowRange();
	}
	return true;
}