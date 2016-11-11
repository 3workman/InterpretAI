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

class cMutex {
    CRITICAL_SECTION _cs;
public:
    cMutex(){ ::InitializeCriticalSection(&_cs); }
    ~cMutex(){ ::DeleteCriticalSection(&_cs); }

    __forceinline CRITICAL_SECTION* operator&(){ return &_cs; }
    __forceinline operator CRITICAL_SECTION&(){ return _cs; }
};

class cLock {
	CRITICAL_SECTION& _refLock; // �����ý�ʡ�пղ���(��ָ��)
public:
    cLock(cMutex& mutex) : _refLock(mutex) // ���󹹽��������ٽ���
    {
        ::EnterCriticalSection(&_refLock);
    }
    ~cLock() { ::LeaveCriticalSection(&_refLock); } // �����������˳��ٽ���
};