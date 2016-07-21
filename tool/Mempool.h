/***********************************************************************
* @ 内存池
* @ brief
	1、预先申请一大块内存，按固定大小分页，页头地址给外界使用
	2、多涉及operator new、operator delete
* @ author zhoumf
* @ date 2014-11-21
************************************************************************/
#pragma once
#include <queue>
#include "cLock.h"

// 检查一段内存是否越界(头尾设标记)
#define CHECKNU 6893    // 除0外任意值
#define PRECHECK_FIELD(i) int __precheck##i;
#define POSCHECK_FIELD(i) int __poscheck##i;
#define INIT_CHECK(o, i) { (o)->__precheck##i = CHECKNU; (o)->__poscheck##i = CHECKNU; }
#define CHECK(o, i){\
if ((o)->__precheck##i != CHECKNU || (o)->__poscheck##i != CHECKNU){\
	printf("%s:%d, memory access out of range with checknu pre %d,pos %d", \
	__FILE__, __LINE__, (o)->__precheck##i, (o)->__poscheck##i);}\
}

class cPool_Page{
	CRITICAL_SECTION m_csLock; /*互斥：临界区
								1、临界区对象必须经过InitializeCriticalSection()的初始化后才能使用
								2、EnterCriticalSection()/LeaveCriticalSection()标识/释放一个临界区
								3、两函数之间的代码段即会互斥访问
								4、LeaveCriticalSection()一定要被执行到，否则死锁，一直无法访问(不要在两函数直接return/break等打断)
								5、最好封装一层(cLock.h)，保证两函数的并行*/
	const size_t	  m_pageSize;
	const size_t	  m_pageNum;
	std::queue<void*> m_queue;
public:
	~cPool_Page() { ::DeleteCriticalSection(&m_csLock); }
	cPool_Page(size_t pageSize, size_t pageNum) : m_pageSize(pageSize), m_pageNum(pageNum){
		::InitializeCriticalSection(&m_csLock);
		Double();
	}
	bool Double(){ // 可设置Double次数限制
		const size_t size = m_pageSize * m_pageNum; // 溢出风险：m_pageSize * m_pageNum
		char* p = (char*)malloc(size); // 无初始化，外界要operator new或调用new(ptr)
		if (!p) return false;

		for (size_t i = 0; i < size; ++i)
		{
			m_queue.push(p);
			p += m_pageSize;
		}
		return true;
	}
	void* Alloc(){
		cLock lock(m_csLock);
		if (m_queue.empty() && !Double()) return NULL; // 空STL容器调front()、pop()直接宕机
		void* p = m_queue.front();
		m_queue.pop();
		return p;
	}
	void Dealloc(void* p){
		cLock lock(m_csLock);
		m_queue.push(p);
	}
};

#define VOID_POOL_ID -1
template <class T>
class cPool_Index{ // 自动编号，便于管理(对象要含有m_index变量，记录其内存id)
	T**	   m_aObj;
	size_t m_num;
	std::queue<int> m_queue;
public:
	~cPool_Index(){}
	cPool_Index(size_t num, bool bIni = true) : m_num(num), m_aObj(NULL){ if (bIni) IniPool(); }
	bool IniPool()
	{
		if (m_aObj) return true;
		m_aObj = (T**)malloc(m_num * sizeof(T*));
		if (!m_aObj) return false;

		//T* p = ::new T[m_num]; // 若类没operator new，就用全局的new(确保初始化)
		T* p = (T*)malloc(m_num * sizeof(T));
		if (!p) return false;	// 若类operator new，此处用new，会多次调用构造函数

		for (size_t i = 0; i < m_num; ++i)
		{
			m_aObj[i] = p++;
			m_queue.push(i);
		}
		return true;
	}
	bool Double(){
		T** temp = (T**)malloc(m_num * 2 * sizeof(T*));
		if (!temp) return false;

		T* p = (T*)malloc(m_num * sizeof(T));
		if (!p) return false;

		memcpy(temp, m_aObj, m_num * sizeof(T*));
		free(m_aObj);	m_aObj = temp;

		for (size_t i = 0; i < m_num; ++i)
		{
			m_aObj[m_num + i] = p++;
			m_queue.push(m_num + i);
		}
		m_num *= 2;
		return true;
	}
	T* Alloc(){
		if (m_queue.empty() && !Double()) return NULL;
		int id = m_queue.front();
		m_queue.pop();
		m_aObj[id]->m_index = id; // 分配时设置内存id
		return m_aObj[id];
	}
	void Dealloc(T* p){
		m_queue.push(p->m_index);
		p->m_index = VOID_POOL_ID; // 回收后置空内存id
	}
	T* GetByID(int id){
		if (id < 0 || id >= m_num) return NULL;
		return VOID_POOL_ID == m_aObj[id]->m_index ? NULL : m_aObj[id];
	}
};
#ifdef _DEBUG
	#define Pool_Index_Define(T) \
		static cPool_Index<T> m_alloc; /*静态变量，要在类外初始化（好容易忘记，LinkError查了好久都不知道）用Pool_Index_Ini初始化*/ \
		int m_index; \
		void* operator new(size_t /*size*/){ return m_alloc.Alloc(); }\
		void* operator new(size_t /*size*/, const char* file, int line){ return m_alloc.Alloc(); }\
		void operator delete(void* p, const char* file, int line){ return m_alloc.Dealloc((T*)p); }\
		void operator delete(void* p, size_t) { return m_alloc.Dealloc((T*)p); }
#else
	#define Pool_Index_Define(T) \
		static cPool_Index<T> m_alloc; \
		int m_index; \
		void* operator new(size_t /*size*/){ return m_alloc.Alloc(); }\
		void* operator new(size_t /*size*/, const char* file, int line){ return m_alloc.Alloc(); }\
		void operator delete(void* p, size_t /*size*/){ return m_alloc.Dealloc((T*)p); }
#endif
#define Pool_Index_Ini(T,num) cPool_Index<T> T::m_alloc(num); /*cpp中用：初始化静态内存池*/

/************************************************************************/
// 示例
#ifdef _MY_Test
	cPool_Page g_pool(4/*sizeof(cObj_Pool)*/, 2);
	class cObj_Pool{
		int _a = -1;
	public:
		cObj_Pool() { cout << "默认构造函数：" << _a << endl; }
		cObj_Pool(int a) : _a(a) { cout << "含参构造函数：" << _a << endl; }

		void* operator new(size_t){ return g_pool.Alloc(); }
		void* operator new(size_t, void* p){ return p; } // 默认的new(ptr)跟这个的实现一样，仅return p;
	};

	void test_Mempool(){
		cout << "——————————————内存池——————————————" << endl;

 		cObj_Pool* p = (cObj_Pool*)g_pool.Alloc(); // 无初始化内存块
		new(p)cObj_Pool(5);		// 重载版本——在ptr所指地址上构建一个对象(调构造函数)
		::new(p)cObj_Pool(10);	// 全局版本——在ptr所指地址上构建一个对象(调构造函数)

		cObj_Pool* pp = new cObj_Pool;
	}
#endif