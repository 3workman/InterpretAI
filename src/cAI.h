/************************************************************************
* @ AI框架
* @ brief
	1、此AI系统主要包含两个部分：基于虚函数的递归调用(见主逻辑模块)、AI切换(见m_vecNext处注释)
	2、AI类型
		(1)Do：   执行基础动作，如走路、攻击、睡觉
		(2)Base： 对Do层做简单包装，加入条件判断，如随机走、围绕某点、跟随
		(3)Logic：组合任意AI，Logic之间也可互相组合，含切换逻辑

	3、与行为树的区别
		(1)逻辑判断、行为写在了一起
		(2)行为树提取了通用节点(易于组织逻辑)
			·Selector Node 选择节点，顺序执行，一True则返回True，全False才返回False
			·Sequence Node 顺序节点，依次执行，一False则返回False，全True才返回True
			·Parallel Node 并行节点，依次执行所有子节点后，收集结果，再按策略给出返回值，如Selector、Sequence策略

	4、同外部系统的交互(OnEvent)
		(1)通过事件类型，直接告诉AI切换行为
		(2)AI内部只关心自己的常规流程，比如巡逻、攻击、跟随…
		(3)特殊条件的逻辑，全交给外界事件处理：
			·发事件的地方，相当于决策层
			·行为树里的决策层，就是整个树结构，行为都封装在行为节点，或一棵小树里
		(4)相对而言行为树的Condition Node就得拉入外部数据
* @ author zhoumf
* @ date 2014-12-17
************************************************************************/
#pragma once
#include "iAI_Char.h"
#include <sysinfoapi.h> //系统时间(毫秒)GetTickCount
#include "..\tool\Mempool.h"


#define TimeNow_Msec GetTickCount()

enum class AI_Return{ // 强类型枚举，不会将枚举常量暴露到外层作用域，也不会隐式转换为整形
	None,
	SwapUrgent,
	CancelTarget,
	Exception,		//异常，例如目的点有一个阻挡
	Continue,		//继续该动作
	Next,			//下一个动作
	GoBack,			//NPC脱离战斗后归位
};
enum class AI_Type{
	None,
	Attack,
	Sleep,
	Walk,
	Walk_Rand,   //随机走动
	Walk_Around, //绕点随机走动
	Walk_ToChar,
	Walk_Attack,
    AI_Chook,   //斗鸡
};

class cAI{ //AI基类，负责AI之间的切换、管理
public:
	virtual ~cAI(){ assert(!m_bRuning); ClearAI(); }
	cAI(iAI_Char* p, AI_Type eType);

	iAI_Char* const m_pChar;
	AI_Type const m_eAIType;

	bool m_bRuning = false; //检测AI执行中删除

	virtual void Release(){ if (m_pSelfEndFun) (*m_pSelfEndFun)(m_pSelfEndParam); delete this; }
	virtual void Init()
	{
		m_nTimeEnd = m_nTime + TimeNow_Msec;
		m_nAcitveIndex = 0;
		for (auto& v : m_vecAI) v->Init();

		if (m_pSelfBeginFun) (*m_pSelfBeginFun)(m_pSelfBeginParam);
	}
	void ClearAI()
	{
		for (auto& v : m_vecAI) v->Release();
		m_vecAI.clear();
		m_pUrgent->Release();
		m_pUrgent = NULL;
	}

private:
	DWORD m_nTime = 0; //AI持续时长
	DWORD m_nTimeEnd = 0; //时间(毫秒)超过，自动就停止
public:
	bool IsTimeout(){ return m_nTime > 0 ? (TimeNow_Msec > m_nTimeEnd) : false; }
	void SetTimeout(DWORD time){ m_nTime = time; m_nTimeEnd = m_nTime + TimeNow_Msec; }
	void SetChildTimeout(DWORD time){ for (auto& v : m_vecAI) v->SetTimeout(time); }

//////////////////////////////////////////////////////////////////////
// AI切换
//////////////////////////////////////////////////////////////////////
private: //子AI列表管理（基础功能AI不涉及此模块）
	size_t m_nAcitveIndex = 0; //当前被激活的AI（m_vecAI的下标）
protected:
	std::vector<cAI*> m_vecAI;	   //基础子AI的此列表为空，只负责功能，无切换逻辑
	std::vector<size_t> m_vecNext; //子类各自指定m_nAcitveIndex成功后的下个AI（大小一般同m_vecAI），nextIndex = m_vecNext[m_nAcitveIndex]
	std::vector<size_t> m_vecException; //出现异常时的切换，子类各自指定m_nAcitveIndex失败后的下个AI（大小一般同m_vecAI）
										//这两个表实际上存储了：m_vecAI的下标对<索引, 响应索引>
										//用map更合逻辑（不必保证m_vecAI、m_vecNext大小一致）实现<大索引, 响应索引> <中间索引, 无>
	cAI* GetActiveAI(){ return m_nAcitveIndex < m_vecAI.size() ? m_vecAI[m_nAcitveIndex] : NULL; }
	bool SetActiveIndex(size_t index){
		if (index < m_vecAI.size()){
			m_nAcitveIndex = index;
			m_vecAI[index]->Init();
			return true;
		}
		return false;
	}
	void ReverseAI(){ std::reverse(m_vecAI.begin(), m_vecAI.end()); } //倒转AI的顺序
public:
	size_t GetAINum(){ return m_vecAI.size(); }
	void AddAI(cAI* p){ m_vecAI.push_back(p); }

//////////////////////////////////////////////////////////////////////
// 主逻辑模块
//////////////////////////////////////////////////////////////////////
private:
	AI_Return OnReturn(AI_Return rt); //只在Run()、OnEvent()调
protected:
	virtual AI_Return ExceptionActiveAI(); //处理子AI->Run()异常，基础功能AI不涉及此接口
public:
	virtual AI_Return Run(); //基础功能AI完全重写Run，高级的组合AI可能在自己的Run()最后调cAI::Run()
	virtual AI_Return OnEvent(const stAIEvent& event);

protected: // 紧急事件：临时插入AI，强制执行（多用于发起攻击）
	cAI* m_pUrgent = NULL;
	void* m_pUrgentEndParam = NULL;
	void* m_pUrgentBeginParam = NULL;
	typedef void(*CallbackFun_vv)(void*);
	CallbackFun_vv m_pUergentEndFun = NULL;
	CallbackFun_vv m_pUergentBeginFun = NULL; //【问题：对于不同的紧急事件，会执行同样的回调函数，除非开发者自行更改】
	void UrgentEnd() // 脱离紧急状态(通常是战斗状态)时触发
	{
        if (m_pUrgent) m_pUrgent->Release();
		m_pUrgent = NULL;
		if (m_pUergentEndFun) (*m_pUergentEndFun)(m_pUrgentEndParam);
	}
	void UrgentBegin() // 进入紧急状态时触发
	{
		if (m_pUergentBeginFun) (*m_pUergentBeginFun)(m_pUrgentBeginParam);
	}
public:
	void SetUrgent(cAI* p)
	{
        if (m_pUrgent) m_pUrgent->Release(); //问题：旧紧急AI被删，但没执行该紧急AI的结束回调
		if (m_pUrgent = p) UrgentBegin();
	}
	cAI* GetUrgent(){ return m_pUrgent; }
	void SetUrgentEnd(CallbackFun_vv fun, void* param)
	{
		m_pUergentEndFun = fun;
		m_pUrgentEndParam = param;
	}
	void SetUrgentBegin(CallbackFun_vv fun, void* param)
	{
		m_pUergentBeginFun = fun;
		m_pUrgentBeginParam = param;
	}
	void* GetUrgentEndParam(){ return m_pUrgentEndParam; }
	void* GetUrgentBeginParam(){ return m_pUrgentBeginParam; }

//////////////////////////////////////////////////////////////////////
// 自我控制模块
//////////////////////////////////////////////////////////////////////
private: // AI开始、结束时的回调（可以替代紧急事件的回调系统，给紧急AI设置SelfCallbackFun）
	void* m_pSelfEndParam = NULL;
	void* m_pSelfBeginParam = NULL;
	CallbackFun_vv m_pSelfEndFun = NULL;
	CallbackFun_vv m_pSelfBeginFun = NULL;
public:
	void SetSelfEnd(CallbackFun_vv fun, void* param)
	{
		m_pSelfEndFun = fun;
		m_pSelfEndParam = param;
	}
	void SetSelfBegin(CallbackFun_vv fun, void* param)
	{
		m_pSelfBeginFun = fun;
		m_pSelfBeginParam = param;
	}
	void* GetSelfEndParam(){ return m_pSelfEndFun; }
	void* GetSelfBeginParam(){ return m_pSelfBeginParam; }

//////////////////////////////////////////////////////////////////////
// 取数据接口
//////////////////////////////////////////////////////////////////////
public:
	cAI* GetActiveWithUrgent()
	{
		if (cAI* p = GetActiveAI())
		{
			cAI* pUrgent = p->GetUrgent();
			return pUrgent ? pUrgent->GetActiveWithUrgent() : p->GetActiveWithUrgent();
		}
		return NULL;
	}
	//基础子AI中，这几个接口直接返回数据，组合AI一般不复写
	virtual int GetDesID(){ cAI* p = GetActiveWithUrgent(); return p ? p->GetDesID() : -1; }
	virtual int GetSpeed(){ cAI* p = GetActiveWithUrgent(); return p ? p->GetSpeed() : -1; }
	virtual CPoint GetDesPos(){ cAI* p = GetActiveWithUrgent(); return p ? p->GetDesPos() : CPoint(-1, -1); }
	virtual AI_Type GetActiveType(){ cAI* p = GetActiveWithUrgent(); return p ? p->GetActiveType() : m_eAIType; }
};

/************************************************************************/
// 示例
iAI_Char* FindCharByIndex(int index); //内存池取角色的接口
DWORD Service_NPCAI(void* p); //注册进ServicePatch，循环跑

/************************************************************************/
// 另一种链状AI调度框架：适用于格斗游戏，不断指定下个AI是什么
class cAI_{
protected:
	cAI_* m_pLast = NULL; // 上一个AI，析构时用
	cAI_* m_pNext = NULL; // 下一个AI，运行时用
public:
	virtual ~cAI_()
	{
		if (m_pLast) m_pLast->m_pNext = m_pNext; //链接自己的上下AI
		if (m_pNext) m_pNext->m_pLast = m_pLast;
	}
	void AddNext(cAI_* p) //可在任一AI运行时指定下个AI，比较灵活，相比上个框架(递归切换型)易调试些
	{
		assert(p && !p->m_pNext); //这个只是提醒(可能性)：加入的AI不能处于环状中，否则会无限递归

		if (m_pNext){
			p->AddNext(m_pNext);
			m_pNext = p;
		}else
			m_pNext = p;

		if (m_pNext) m_pNext->m_pLast = this;
	}
	const std::vector<cAI_*>& GetAIList()
	{
		static std::vector<cAI_*> s_vec; s_vec.clear();

		cAI_* pTemp = this; int i = 0;
		while (pTemp)
		{
			if (NULL == pTemp->m_pLast)
			{
				s_vec.push_back(pTemp); //找到头
			}
			pTemp = pTemp->m_pLast;
			if (++i >= 10) break; //为获取环状AI，最多向上找十个(正常情况AI链没这么长)
		}

		pTemp = s_vec[0]; i = 0;
		while (pTemp)
		{
			s_vec.push_back(pTemp);
			pTemp = pTemp->m_pNext;
			if (++i >= 10) break; //为获取环状AI，最多向下找十个
		}
		return s_vec;
	}
};