/************************************************************************
* @ 围绕一点走动的AI
* @ brief
	1、由AI_WalkRand(随机走动)、AI_Sleep构成
	2、SetAround()接口：将具体动作子AI加入列表(有序) WalkRand-->Sleep
	3、构造函数指定AI切换顺序，编号0-->编号1、编号1-->编号0，即两个子AI互相切换
* @ author zhoumf
* @ date 2014-12-18
************************************************************************/
#pragma once
#include "cAI.h"

class cAI_WalkAround : public cAI{
public:
	cAI_WalkAround(iAI_Char* p) : cAI(p, AI_Type::Walk_Around)
	{
		m_vecNext.push_back(1);
		m_vecNext.push_back(0);
		m_vecException.push_back(1);
		m_vecException.push_back(0);
	}
	Pool_Index_Define(cAI_WalkAround)

	bool SetAround(CPoint pt, int radius, int speed);
};