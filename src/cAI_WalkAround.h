/************************************************************************
* @ Χ��һ���߶���AI
* @ brief
	1����AI_WalkRand(����߶�)��AI_Sleep����
	2��SetAround()�ӿڣ������嶯����AI�����б�(����) WalkRand-->Sleep
	3�����캯��ָ��AI�л�˳�򣬱��0-->���1�����1-->���0����������AI�����л�
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