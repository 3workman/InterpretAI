/***********************************************************************
* @ �ڴ��
* @ brief
	1��Ԥ������һ����ڴ棬���̶���С��ҳ��ҳͷ��ַ�����ʹ��
	2�����漰operator new��operator delete
* @ author zhoumf
* @ date 2014-11-21
************************************************************************/
#pragma once
#include <queue>
#include "cLock.h"

// ���һ���ڴ��Ƿ�Խ��(ͷβ����)
#define CHECKNU 6893    // ��0������ֵ
#define PRECHECK_FIELD(i) int __precheck##i;
#define POSCHECK_FIELD(i) int __poscheck##i;
#define INIT_CHECK(o, i) { (o)->__precheck##i = CHECKNU; (o)->__poscheck##i = CHECKNU; }
#define CHECK(o, i){\
if ((o)->__precheck##i != CHECKNU || (o)->__poscheck##i != CHECKNU){\
	printf("%s:%d, memory access out of range with checknu pre %d,pos %d", \
	__FILE__, __LINE__, (o)->__precheck##i, (o)->__poscheck##i);}\
}

class cPool_Page{
	CRITICAL_SECTION m_csLock; /*���⣺�ٽ���
								1���ٽ���������뾭��InitializeCriticalSection()�ĳ�ʼ�������ʹ��
								2��EnterCriticalSection()/LeaveCriticalSection()��ʶ/�ͷ�һ���ٽ���
								3��������֮��Ĵ���μ��ụ�����
								4��LeaveCriticalSection()һ��Ҫ��ִ�е�������������һֱ�޷�����(��Ҫ��������ֱ��return/break�ȴ��)
								5����÷�װһ��(cLock.h)����֤�������Ĳ���*/
	const size_t	  m_pageSize;
	const size_t	  m_pageNum;
	std::queue<void*> m_queue;
public:
	~cPool_Page() { ::DeleteCriticalSection(&m_csLock); }
	cPool_Page(size_t pageSize, size_t pageNum) : m_pageSize(pageSize), m_pageNum(pageNum){
		::InitializeCriticalSection(&m_csLock);
		Double();
	}
	bool Double(){ // ������Double��������
		const size_t size = m_pageSize * m_pageNum; // ������գ�m_pageSize * m_pageNum
		char* p = (char*)malloc(size); // �޳�ʼ�������Ҫoperator new�����new(ptr)
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
		if (m_queue.empty() && !Double()) return NULL; // ��STL������front()��pop()ֱ��崻�
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
class cPool_Index{ // �Զ���ţ����ڹ���(����Ҫ����m_index��������¼���ڴ�id)
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

		//T* p = ::new T[m_num]; // ����ûoperator new������ȫ�ֵ�new(ȷ����ʼ��)
		T* p = (T*)malloc(m_num * sizeof(T));
		if (!p) return false;	// ����operator new���˴���new�����ε��ù��캯��

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
		m_aObj[id]->m_index = id; // ����ʱ�����ڴ�id
		return m_aObj[id];
	}
	void Dealloc(T* p){
		m_queue.push(p->m_index);
		p->m_index = VOID_POOL_ID; // ���պ��ÿ��ڴ�id
	}
	T* GetByID(int id){
		if (id < 0 || id >= m_num) return NULL;
		return VOID_POOL_ID == m_aObj[id]->m_index ? NULL : m_aObj[id];
	}
};
#ifdef _DEBUG
	#define Pool_Index_Define(T) \
		static cPool_Index<T> m_alloc; /*��̬������Ҫ�������ʼ�������������ǣ�LinkError���˺þö���֪������Pool_Index_Ini��ʼ��*/ \
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
#define Pool_Index_Ini(T,num) cPool_Index<T> T::m_alloc(num); /*cpp���ã���ʼ����̬�ڴ��*/

/************************************************************************/
// ʾ��
#ifdef _MY_Test
	cPool_Page g_pool(4/*sizeof(cObj_Pool)*/, 2);
	class cObj_Pool{
		int _a = -1;
	public:
		cObj_Pool() { cout << "Ĭ�Ϲ��캯����" << _a << endl; }
		cObj_Pool(int a) : _a(a) { cout << "���ι��캯����" << _a << endl; }

		void* operator new(size_t){ return g_pool.Alloc(); }
		void* operator new(size_t, void* p){ return p; } // Ĭ�ϵ�new(ptr)�������ʵ��һ������return p;
	};

	void test_Mempool(){
		cout << "�����������������������������ڴ�ء���������������������������" << endl;

 		cObj_Pool* p = (cObj_Pool*)g_pool.Alloc(); // �޳�ʼ���ڴ��
		new(p)cObj_Pool(5);		// ���ذ汾������ptr��ָ��ַ�Ϲ���һ������(�����캯��)
		::new(p)cObj_Pool(10);	// ȫ�ְ汾������ptr��ָ��ַ�Ϲ���һ������(�����캯��)

		cObj_Pool* pp = new cObj_Pool;
	}
#endif