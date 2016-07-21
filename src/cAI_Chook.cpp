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

        pActivity->SetTimeout( (rand() % 6 + 3)*1000 ); //攻击持续3-8秒
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
        m_setHitMe.insert(event.pChar); //缓存打自己的人

        if (m_eStatus != ChookFlee) //非逃跑状态
        {
            const size_t nHitMe = m_setHitMe.size();
            if (nHitMe > 1)
            {
                BYTE nFlee = 1; //确保nFlee不为0
                switch (nHitMe){
                case 2: nFlee = 4; break;   //25%逃跑
                case 3: nFlee = 2; break;   //50%逃跑
                default: nFlee = 1; break;  //100%逃跑
                }
                if (rand() % nFlee) return AI_Return::Continue;

                cAI_Walk* p = new cAI_Walk(m_pChar, false);
                CPoint pt = m_pChar->GetCharPos();
                pt.x += (rand() % 3 - 1) * 9;
                pt.y += (rand() % 3 - 1) * 9;
                p->SetDes(pt, m_nSpeed + 1, 1,0,0); //逃跑时移动速度+1格
                p->SetTimeout( (rand() % 5 + 5)*1000 ); //逃跑持续5-9秒
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
    /*  GetTickcount函数：它返回从操作系统启动到当前所经过的毫秒数，常常用来判断某个方法执行的时间
        返回值以32位的双字类型DWORD存储，因此可以存储的最大值是(2^32-1) ms约为49.71天
        【因此若系统运行时间超过49.71天时，这个数就会归0】
        因此，如果是编写服务器端程序，此处一定要万分注意，避免引起意外的状况
        特别注意：这个函数并非实时发送，而是由系统每18ms发送一次，因此其最小精度为18ms。当需要有小于18ms的精度计算时，应使用StopWatch方法进行 */

    if (timeNow > m_timeClearHitMe) //1秒清一次打自己的人
    {
        m_timeClearHitMe = timeNow + 1000;

        m_setHitMe.size() ? m_secNoBeAttack = 0 : ++m_secNoBeAttack; // 几秒内没被打
        m_setHitMe.clear();

        if (m_eStatus == ChookFlee && m_secNoBeAttack >= 3){ //逃跑时，3秒没被打，参战
                                                    // CChookFightActivityMgr::Instance().FindEnemyChook(m_pChar)
            if (iAI_Char* p = m_pChar->FindEnemy()) // 由于斗鸡只打斗鸡，此处的FindEnemy可以不走通用逻辑，直接在斗鸡列表里找(斗鸡管理器中)
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
    else if (m_pUrgent == NULL) //无紧急事件，找鸡打
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
    if (ChookFlee == m_eStatus && ChookFlee != status) //脱离逃跑状态
    {
        m_pChar->ClearBuff(265);//Buff_ChookTaoPao
    }
    else if (ChookFlee == status && ChookFlee != m_eStatus) //进入逃跑状态
    {
        m_pChar->AddBuff(265, 9, 0, 0, 0);//Buff_ChookTaoPao
    }

    m_eStatus = status;
}