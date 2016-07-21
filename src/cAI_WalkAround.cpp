#include "StdAfx.h"
#include "cAI_WalkAround.h"
#include "cAI_Walk.h"
#include "cAI_Sleep.h"
#include <atltypes.h>

Pool_Index_Ini(cAI_WalkAround, 32)

bool cAI_WalkAround::SetAround(CPoint pt, int radius, int speed)
{
	ClearAI();
	{
		cAI_WalkRand* p = new cAI_WalkRand(m_pChar);
		if (NULL == p) return false;
		p->SetAround(pt, radius, speed);
		AddAI(p);
	}{
		cAI_Sleep* p = new cAI_Sleep(m_pChar);
		if (NULL == p) return false;
		p->SetTimeout(3000);
		AddAI(p);
	}
	SetActiveIndex(0);
	return true;
}