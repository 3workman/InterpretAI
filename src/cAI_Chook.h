#pragma once
#include "cAI.h"
#include <set>

class cAI_Chook : public cAI{
    int m_nSpeed;
    int m_nSkillTID;
    int m_nRadius;
    CPoint m_ptInit;
    DWORD m_idAttack;
    DWORD m_timeClearHitMe;
    DWORD m_secNoBeAttack; //几秒内没人打自己
    std::set<iAI_Char*> m_setHitMe;
public:
    cAI_Chook(iAI_Char* p);

    void SetAround(POINT pt,int radius,int nSpeed,int nSkillTID);
    void SetAttack(iAI_Char* p);

    enum NpcStatus{
        ChookFlee,
        ChookBack,
        ChookAttack,
        ChookNormal,
    };
    NpcStatus m_eStatus;
    void SetStatus(NpcStatus status);

    AI_Return OnEvent(const stAIEvent& event) override;
    AI_Return Run() override;
};