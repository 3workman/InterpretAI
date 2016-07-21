#include "StdAfx.h"
#include "cAI_Chook.h"
#include "cAI_Walk.h"
#include "cAI_Sleep.h"
#include "cAI_WalkAttack.h"


cAI_Chook::cAI_Chook(iAI_Char* p) : cAI(p, AI_Type::AI_Chook), m_eStatus(ChookNormal)
{
    m_vecNext.push_back(1);
    m_vecNext.push_back(0);
    m_vecException.push_back(1);
    m_vecException.push_back(0);
}

void cAI_Chook::SetAround(POINT pt,int radius,int nSpeed, int nSkillTID)
{
    m_nSpeed = nSpeed;
    m_nSkillTID = nSkillTID;
    m_ptInit = pt;
    m_nRadius = radius;
    m_timeClearHitMe = 0;
    m_secNoBeAttack = 0;
    ClearAI();

    {
        cAI_WalkRand* p1 = new cAI_WalkRand(m_pChar);
        if (NULL == p1) return;
        p1->SetAround(pt, radius, 1);
        AddAI(p1);
    }{
        cAI_Sleep* p2 = new cAI_Sleep(m_pChar);
        if(NULL == p2) return;
        p2->SetTimeout(3000);
        AddAI(p2);
    }
    SetActiveIndex(0);

    SetStatus(ChookNormal);
}

void cAI_Chook::SetAttack(iAI_Char* p)
{
    assert(p);
    if (p)
    {
        cAI_WalkAttack* pActivity = new cAI_WalkAttack(m_pChar, false, false);
        if(NULL == pActivity) return ;

        if (m_pChar->HaveBuff(259)) //Buff_ChookLongJia
            pActivity->SetAttack(p, m_nSkillTID, m_nSpeed + 1);
        else
            pActivity->SetAttack(p, m_nSkillTID, m_nSpeed);

        pActivity->SetTimeout( (rand() % 6 + 3)*1000 ); //��������3-8��
        pActivity->Init();
        SetUrgent(pActivity);

        m_timeClearHitMe = 0;
        m_secNoBeAttack = 0;
        m_idAttack = p->GetID();
        SetStatus(ChookAttack);
    }
}

AI_Return cAI_Chook::OnEvent(const stAIEvent& event)
{
    if (event.e == AIEvent_BeAttack)
    {
        m_setHitMe.insert(event.pChar); //������Լ�����

        if (m_eStatus != ChookFlee) //������״̬
        {
            const size_t nHitMe = m_setHitMe.size();
            if (nHitMe > 1)
            {
                BYTE nFlee = 1; //ȷ��nFlee��Ϊ0
                switch (nHitMe){
                case 2: nFlee = 4; break;   //25%����
                case 3: nFlee = 2; break;   //50%����
                default: nFlee = 1; break;  //100%����
                }
                if (rand() % nFlee) return AI_Return::Continue;

                cAI_Walk* p = new cAI_Walk(m_pChar, false);
                CPoint pt = m_pChar->GetCharPos();
                pt.x += (rand() % 3 - 1) * 9;
                pt.y += (rand() % 3 - 1) * 9;
                p->SetDes(pt, m_nSpeed + 1, 1,0,0); //����ʱ�ƶ��ٶ�+1��
                p->SetTimeout( (rand() % 5 + 5)*1000 ); //���ܳ���5-9��
                SetUrgent(p);
                SetStatus(ChookFlee);
                return AI_Return::Continue;
            }
        }
    }
    else if (event.e == AIEvent_Die)
    {
        SetStatus(ChookNormal);
        m_setHitMe.clear();
        UrgentEnd();
    }
    return AI_Return::Continue;
}

AI_Return cAI_Chook::Run()
{
    const DWORD timeNow = ::GetTickCount();
    /*  GetTickcount�����������شӲ���ϵͳ��������ǰ�������ĺ����������������ж�ĳ������ִ�е�ʱ��
        ����ֵ��32λ��˫������DWORD�洢����˿��Դ洢�����ֵ��(2^32-1) msԼΪ49.71��
        �������ϵͳ����ʱ�䳬��49.71��ʱ��������ͻ��0��
        ��ˣ�����Ǳ�д�������˳��򣬴˴�һ��Ҫ���ע�⣬�������������״��
        �ر�ע�⣺�����������ʵʱ���ͣ�������ϵͳÿ18ms����һ�Σ��������С����Ϊ18ms������Ҫ��С��18ms�ľ��ȼ���ʱ��Ӧʹ��StopWatch�������� */

    if (timeNow > m_timeClearHitMe) //1����һ�δ��Լ�����
    {
        m_timeClearHitMe = timeNow + 1000;

        m_setHitMe.size() ? m_secNoBeAttack = 0 : ++m_secNoBeAttack; // ������û����
        m_setHitMe.clear();

        if (m_eStatus == ChookFlee && m_secNoBeAttack >= 3){ //����ʱ��3��û���򣬲�ս
                                                    // CChookFightActivityMgr::Instance().FindEnemyChook(m_pChar)
            if (iAI_Char* p = m_pChar->FindEnemy()) // ���ڶ���ֻ�򶷼����˴���FindEnemy���Բ���ͨ���߼���ֱ���ڶ����б�����(������������)
                SetAttack(p);
        }
    }

    if (m_eStatus != ChookBack && !InDistance(m_pChar->GetCharPos(), m_ptInit, 30))
    {
        SetStatus(ChookBack);
        cAI_Walk* p = new cAI_Walk(m_pChar, false);
        p->SetDes(m_ptInit, m_nSpeed, 1,0,0);
        SetUrgent(p);
    }
    else if (m_pUrgent == NULL) //�޽����¼����Ҽ���
    {
        if (iAI_Char* p = m_pChar->FindEnemy()){
            SetAttack(p);
        }else{
            SetStatus(ChookNormal);
        }
    }

    return cAI::Run();
}

void cAI_Chook::SetStatus(NpcStatus status)
{
    if (ChookFlee == m_eStatus && ChookFlee != status) //��������״̬
    {
        m_pChar->ClearBuff(265);//Buff_ChookTaoPao
    }
    else if (ChookFlee == status && ChookFlee != m_eStatus) //��������״̬
    {
        m_pChar->AddBuff(265, 9, 0, 0, 0);//Buff_ChookTaoPao
    }

    m_eStatus = status;
}