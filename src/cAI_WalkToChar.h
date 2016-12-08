#pragma once
#include "cAI.h"

class cAI_WalkToChar : public cAI{
    Pool_Index_Define(cAI_WalkToChar, 32)
	const bool m_bPlayerAutoKillNpc = false;
	int&  m_refSkillTID;
	int	  m_nDesID = 0;
public:
	cAI_WalkToChar(iAI_Char* p, bool bPlayerAutoKillNpc, int nSkill);

	bool SetWalkTo(iAI_Char* p, int speed, int range);

	AI_Return Run() override;
};