/***********************************************************************
* @ 互斥锁、临界区（CRITICAL_SECTION）
* @ brief
	1、临界区对象必须经过::InitializeCriticalSection()的初始化后才能使用
	2、使用完毕要::DeleteCriticalSection()删除
	3、::EnterCriticalSection()/::LeaveCriticalSection()标识/释放一个临界区
	4、两函数之间的代码段即会互斥访问；cLock类封装一层，保证两函数的并行
	5、::LeaveCriticalSection()一定要被执行到，否则死锁(一直无法访问)，不要在两函数直接return/break等打断
* @ author zhoumf
* @ date 2014-11-21
************************************************************************/
#pragma once
#include <windows.h>

class cLock{
	CRITICAL_SECTION& m_refLock; // 存引用节省判空操作(比指针)
public:
	cLock(CRITICAL_SECTION& lock) : m_refLock(lock) // 对象构建，进入临界区
	{
		::EnterCriticalSection(&m_refLock);
	}
	virtual ~cLock() { Unlock(); } // 对象析构，退出临界区
	void Unlock() { ::LeaveCriticalSection(&m_refLock); }
};