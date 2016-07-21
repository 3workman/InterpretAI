#pragma once
#include "cAI.h"

class cAI_Sleep : public cAI{
public:
	cAI_Sleep(iAI_Char* p) : cAI(p, AI_Type::Sleep){}

	AI_Return Run() override { return IsTimeout() ? AI_Return::Next : AI_Return::Continue; }
	int GetDesID() override { return 0; }
	int GetSpeed() override { return 0; }
	CPoint GetDesPos() override { return CPoint(0, 0); }
	AI_Type GetActiveType() override { return m_eAIType; }
};