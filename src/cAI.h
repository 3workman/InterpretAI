/************************************************************************
* @ AI���
* @ brief
	1����AIϵͳ��Ҫ�����������֣������麯���ĵݹ����(�����߼�ģ��)��AI�л�(��m_vecNext��ע��)
	2��AI����
		(1)Do��   ִ�л�������������·��������˯��
		(2)Base�� ��Do�����򵥰�װ�����������жϣ�������ߡ�Χ��ĳ�㡢����
		(3)Logic���������AI��Logic֮��Ҳ�ɻ�����ϣ����л��߼�

	3������Ϊ��������
		(1)�߼��жϡ���Ϊд����һ��
		(2)��Ϊ����ȡ��ͨ�ýڵ�(������֯�߼�)
			��Selector Node ѡ��ڵ㣬˳��ִ�У�һTrue�򷵻�True��ȫFalse�ŷ���False
			��Sequence Node ˳��ڵ㣬����ִ�У�һFalse�򷵻�False��ȫTrue�ŷ���True
			��Parallel Node ���нڵ㣬����ִ�������ӽڵ���ռ�������ٰ����Ը�������ֵ����Selector��Sequence����

	4��ͬ�ⲿϵͳ�Ľ���(OnEvent)
		(1)ͨ���¼����ͣ�ֱ�Ӹ���AI�л���Ϊ
		(2)AI�ڲ�ֻ�����Լ��ĳ������̣�����Ѳ�ߡ����������桭
		(3)�����������߼���ȫ��������¼�����
			�����¼��ĵط����൱�ھ��߲�
			����Ϊ����ľ��߲㣬�����������ṹ����Ϊ����װ����Ϊ�ڵ㣬��һ��С����
		(4)��Զ�����Ϊ����Condition Node�͵������ⲿ����
* @ author zhoumf
* @ date 2014-12-17
************************************************************************/
#pragma once
#include "iAI_Char.h"
#include <sysinfoapi.h> //ϵͳʱ��(����)GetTickCount
#include "..\tool\Mempool.h"


#define TimeNow_Msec GetTickCount()

enum class AI_Return{ // ǿ����ö�٣����Ὣö�ٳ�����¶�����������Ҳ������ʽת��Ϊ����
	None,
	SwapUrgent,
	CancelTarget,
	Exception,		//�쳣������Ŀ�ĵ���һ���赲
	Continue,		//�����ö���
	Next,			//��һ������
	GoBack,			//NPC����ս�����λ
};
enum class AI_Type{
	None,
	Attack,
	Sleep,
	Walk,
	Walk_Rand,   //����߶�
	Walk_Around, //�Ƶ�����߶�
	Walk_ToChar,
	Walk_Attack,
    AI_Chook,   //����
};

class cAI{ //AI���࣬����AI֮����л�������
public:
	virtual ~cAI(){ assert(!m_bRuning); ClearAI(); }
	cAI(iAI_Char* p, AI_Type eType);

	iAI_Char* const m_pChar;
	AI_Type const m_eAIType;

	bool m_bRuning = false; //���AIִ����ɾ��

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
	DWORD m_nTime = 0; //AI����ʱ��
	DWORD m_nTimeEnd = 0; //ʱ��(����)�������Զ���ֹͣ
public:
	bool IsTimeout(){ return m_nTime > 0 ? (TimeNow_Msec > m_nTimeEnd) : false; }
	void SetTimeout(DWORD time){ m_nTime = time; m_nTimeEnd = m_nTime + TimeNow_Msec; }
	void SetChildTimeout(DWORD time){ for (auto& v : m_vecAI) v->SetTimeout(time); }

//////////////////////////////////////////////////////////////////////
// AI�л�
//////////////////////////////////////////////////////////////////////
private: //��AI�б������������AI���漰��ģ�飩
	size_t m_nAcitveIndex = 0; //��ǰ�������AI��m_vecAI���±꣩
protected:
	std::vector<cAI*> m_vecAI;	   //������AI�Ĵ��б�Ϊ�գ�ֻ�����ܣ����л��߼�
	std::vector<size_t> m_vecNext; //�������ָ��m_nAcitveIndex�ɹ�����¸�AI����Сһ��ͬm_vecAI����nextIndex = m_vecNext[m_nAcitveIndex]
	std::vector<size_t> m_vecException; //�����쳣ʱ���л����������ָ��m_nAcitveIndexʧ�ܺ���¸�AI����Сһ��ͬm_vecAI��
										//��������ʵ���ϴ洢�ˣ�m_vecAI���±��<����, ��Ӧ����>
										//��map�����߼������ر�֤m_vecAI��m_vecNext��Сһ�£�ʵ��<������, ��Ӧ����> <�м�����, ��>
	cAI* GetActiveAI(){ return m_nAcitveIndex < m_vecAI.size() ? m_vecAI[m_nAcitveIndex] : NULL; }
	bool SetActiveIndex(size_t index){
		if (index < m_vecAI.size()){
			m_nAcitveIndex = index;
			m_vecAI[index]->Init();
			return true;
		}
		return false;
	}
	void ReverseAI(){ std::reverse(m_vecAI.begin(), m_vecAI.end()); } //��תAI��˳��
public:
	size_t GetAINum(){ return m_vecAI.size(); }
	void AddAI(cAI* p){ m_vecAI.push_back(p); }

//////////////////////////////////////////////////////////////////////
// ���߼�ģ��
//////////////////////////////////////////////////////////////////////
private:
	AI_Return OnReturn(AI_Return rt); //ֻ��Run()��OnEvent()��
protected:
	virtual AI_Return ExceptionActiveAI(); //������AI->Run()�쳣����������AI���漰�˽ӿ�
public:
	virtual AI_Return Run(); //��������AI��ȫ��дRun���߼������AI�������Լ���Run()����cAI::Run()
	virtual AI_Return OnEvent(const stAIEvent& event);

protected: // �����¼�����ʱ����AI��ǿ��ִ�У������ڷ��𹥻���
	cAI* m_pUrgent = NULL;
	void* m_pUrgentEndParam = NULL;
	void* m_pUrgentBeginParam = NULL;
	typedef void(*CallbackFun_vv)(void*);
	CallbackFun_vv m_pUergentEndFun = NULL;
	CallbackFun_vv m_pUergentBeginFun = NULL; //�����⣺���ڲ�ͬ�Ľ����¼�����ִ��ͬ���Ļص����������ǿ��������и��ġ�
	void UrgentEnd() // �������״̬(ͨ����ս��״̬)ʱ����
	{
        if (m_pUrgent) m_pUrgent->Release();
		m_pUrgent = NULL;
		if (m_pUergentEndFun) (*m_pUergentEndFun)(m_pUrgentEndParam);
	}
	void UrgentBegin() // �������״̬ʱ����
	{
		if (m_pUergentBeginFun) (*m_pUergentBeginFun)(m_pUrgentBeginParam);
	}
public:
	void SetUrgent(cAI* p)
	{
        if (m_pUrgent) m_pUrgent->Release(); //���⣺�ɽ���AI��ɾ����ûִ�иý���AI�Ľ����ص�
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
// ���ҿ���ģ��
//////////////////////////////////////////////////////////////////////
private: // AI��ʼ������ʱ�Ļص���������������¼��Ļص�ϵͳ��������AI����SelfCallbackFun��
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
// ȡ���ݽӿ�
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
	//������AI�У��⼸���ӿ�ֱ�ӷ������ݣ����AIһ�㲻��д
	virtual int GetDesID(){ cAI* p = GetActiveWithUrgent(); return p ? p->GetDesID() : -1; }
	virtual int GetSpeed(){ cAI* p = GetActiveWithUrgent(); return p ? p->GetSpeed() : -1; }
	virtual CPoint GetDesPos(){ cAI* p = GetActiveWithUrgent(); return p ? p->GetDesPos() : CPoint(-1, -1); }
	virtual AI_Type GetActiveType(){ cAI* p = GetActiveWithUrgent(); return p ? p->GetActiveType() : m_eAIType; }
};

/************************************************************************/
// ʾ��
iAI_Char* FindCharByIndex(int index); //�ڴ��ȡ��ɫ�Ľӿ�
DWORD Service_NPCAI(void* p); //ע���ServicePatch��ѭ����

/************************************************************************/
// ��һ����״AI���ȿ�ܣ������ڸ���Ϸ������ָ���¸�AI��ʲô
class cAI_{
protected:
	cAI_* m_pLast = NULL; // ��һ��AI������ʱ��
	cAI_* m_pNext = NULL; // ��һ��AI������ʱ��
public:
	virtual ~cAI_()
	{
		if (m_pLast) m_pLast->m_pNext = m_pNext; //�����Լ�������AI
		if (m_pNext) m_pNext->m_pLast = m_pLast;
	}
	void AddNext(cAI_* p) //������һAI����ʱָ���¸�AI���Ƚ�������ϸ����(�ݹ��л���)�׵���Щ
	{
		assert(p && !p->m_pNext); //���ֻ������(������)�������AI���ܴ��ڻ�״�У���������޵ݹ�

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
				s_vec.push_back(pTemp); //�ҵ�ͷ
			}
			pTemp = pTemp->m_pLast;
			if (++i >= 10) break; //Ϊ��ȡ��״AI�����������ʮ��(�������AI��û��ô��)
		}

		pTemp = s_vec[0]; i = 0;
		while (pTemp)
		{
			s_vec.push_back(pTemp);
			pTemp = pTemp->m_pNext;
			if (++i >= 10) break; //Ϊ��ȡ��״AI�����������ʮ��
		}
		return s_vec;
	}
};