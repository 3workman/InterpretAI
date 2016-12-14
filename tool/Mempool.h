/***********************************************************************
* @ �ڴ��
* @ brief
	1��Ԥ������һ����ڴ棬���̶���С��ҳ��ҳͷ��ַ�����ʹ��
	2�����漰operator new��operator delete

* @ Notice
    1��cPool_Index �ڴ����Ķ�����m_index���ݣ�ʵΪ�ڴ�����
    2������籣��ʱ�����ܶ����Ѿ������������ã���ôͨ�������idx�ҵ��ľ��Ǵ���ָ����
    3�����籣��NpcID���Ǹ�Npc�Ѿ��������ڴ�ǡ�ñ���Npc���ã���ʱͨ��NpcID�ҵ��ľͲ�֪����ɶ��
    4�������ڶ�����Ӹ��������������硰uint16 autoId;����ͬm_index�ϲ���uint32����Ψһid���������ֱ��ʹ��m_index

    void MyClass::_CreateUniqueId() //���ڶ����½�ʱ���ã������ط�ֱ���� m_unique_id
    {
        static uint16 s_auto_id = 0;

        m_unique_id = ((++s_auto_id) << 16) + m_index; //������16λ�͹��ã������Ļ����64λ��union��
    }
    MyClass* MyClass::FindGroup(uint32 uniqueId)
    {
        int idx = uniqueId & 0xFFFF;

        if (MyClass* ret = FindByIdx(idx))
            if (ret->m_unique_id == uniqueId)
                return ret;
        return NULL;
    }
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
	cMutex            m_csLock;
	const size_t	  m_pageSize;
	const size_t	  m_pageNum;
	std::queue<void*> m_queue;
public:
	cPool_Page(size_t pageSize, size_t pageNum) : m_pageSize(pageSize), m_pageNum(pageNum){
		Double();
	}
    bool Double() // ������Double��������
    {
        // �޳�ʼ�������Ҫoperator new�����new(ptr)
        char* p = (char*)malloc(m_pageSize * m_pageNum); // ������գ�m_pageSize * m_pageNum
        if (!p) return false;

        // �����дm_queue�Ͳ��ؼ����ˣ�Alloc���Ѿ��ӹ������⹹�캯���в��ؼ�
        for (size_t i = 0; i < m_pageNum; ++i) {
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
	cPool_Index(size_t num) : m_num(num){
		m_aObj = (T**)malloc(m_num * sizeof(T*));
		if (!m_aObj) return;

		//T* p = ::new T[m_num]; // ����ûoperator new������ȫ�ֵ�new(ȷ����ʼ��)
		T* p = (T*)malloc(m_num * sizeof(T));
		if (!p) return;	         // ����operator new���˴���new�����ε��ù��캯��

		for (size_t i = 0; i < m_num; ++i)
		{
			m_aObj[i] = p++;
			m_queue.push(i);
		}
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
	#define Pool_Index_Define(T, size) \
        static cPool_Index<T>& _Pool(){ static cPool_Index<T> pool(size); return pool; } \
        public: \
		int m_index; \
		void* operator new(size_t /*size*/){ return _Pool().Alloc(); }\
		void* operator new(size_t /*size*/, const char* file, int line){ return _Pool().Alloc(); }\
		void operator delete(void* p, const char* file, int line){ return _Pool().Dealloc((T*)p); }\
		void operator delete(void* p, size_t) { return _Pool().Dealloc((T*)p); }
#else
	#define Pool_Index_Define(T, size) \
        static cPool_Index<T>& _Pool(){ static cPool_Index<T> pool(size); return pool; } \
        public: \
		int m_index; \
		void* operator new(size_t /*size*/){ return _Pool().Alloc(); }\
		void* operator new(size_t /*size*/, const char* file, int line){ return _Pool().Alloc(); }\
		void operator delete(void* p, size_t /*size*/){ return _Pool().Dealloc((T*)p); }
#endif

template <class T>
class cPool_Obj{
	cPool_Page	m_pool;
public:
	cPool_Obj(size_t num) : m_pool(sizeof(T), num){}
	T* Alloc(){ return (T*)m_pool.Alloc(); }
	void Dealloc(T* p){ m_pool.Dealloc(p); }
};
#ifdef _DEBUG
#define Pool_Obj_Define(T, size) \
        static cPool_Obj<T>& _Pool(){ static cPool_Obj<T> pool(size); return pool; } \
        public: \
		void* operator new(size_t /*size*/){ return _Pool().Alloc(); }\
		void* operator new(size_t /*size*/, const char* file, int line){ return _Pool().Alloc(); }\
		void operator delete(void* p, const char* file, int line){ return _Pool().Dealloc((T*)p); }\
		void operator delete(void* p, size_t) { return _Pool().Dealloc((T*)p); }
#else
#define Pool_Obj_Define(T, size) \
        static cPool_Obj<T>& _Pool(){ static cPool_Obj<T> pool(size); return pool; } \
        public: \
		void* operator new(size_t /*size*/){ return _Pool().Alloc(); }\
		void* operator new(size_t /*size*/, const char* file, int line){ return _Pool().Alloc(); }\
		void operator delete(void* p, size_t /*size*/){ return _Pool().Dealloc((T*)p); }
#endif

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