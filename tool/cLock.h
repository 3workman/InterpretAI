/***********************************************************************
* @ ���������ٽ�����CRITICAL_SECTION��
* @ brief
	1���ٽ���������뾭��::InitializeCriticalSection()�ĳ�ʼ�������ʹ��
	2��ʹ�����Ҫ::DeleteCriticalSection()ɾ��
	3��::EnterCriticalSection()/::LeaveCriticalSection()��ʶ/�ͷ�һ���ٽ���
	4��������֮��Ĵ���μ��ụ����ʣ�cLock���װһ�㣬��֤�������Ĳ���
	5��::LeaveCriticalSection()һ��Ҫ��ִ�е�����������(һֱ�޷�����)����Ҫ��������ֱ��return/break�ȴ��
* @ author zhoumf
* @ date 2014-11-21
************************************************************************/
#pragma once
#include <windows.h>

class cLock{
	CRITICAL_SECTION& m_refLock; // �����ý�ʡ�пղ���(��ָ��)
public:
	cLock(CRITICAL_SECTION& lock) : m_refLock(lock) // ���󹹽��������ٽ���
	{
		::EnterCriticalSection(&m_refLock);
	}
	virtual ~cLock() { Unlock(); } // �����������˳��ٽ���
	void Unlock() { ::LeaveCriticalSection(&m_refLock); }
};