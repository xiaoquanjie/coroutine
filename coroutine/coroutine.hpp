#ifndef M_COROUTINE_COROUTINE_INCLUDE
#define M_COROUTINE_COROUTINE_INCLUDE

#include "coroutine/config.hpp"
#include "coroutine/slist.hpp"
M_COROUTINE_NAMESPACE_BEGIN

typedef void(*_coroutine_func_)(void*ud);
#define COROUTINE_READY   (1)
#define COROUTINE_RUNNING (2)
#define COROUTINE_SUSPEND (3)
#define COROUTINE_DEAD	  (4)

#define DEFAULT_COROUTINE (1024)

#ifdef M_PLATFORM_WIN
struct _coroutine_ {
	int _id;
	int _status;
	LPVOID _ctx;
	void* _data;
	_coroutine_func_ _function;
};
typedef std::map<int, _coroutine_*> CoroutineMap;
struct _schedule_ {
	_schedule_() {
		_cap = 0;
		_nco = 0;
		_curco = 0;
		_ctx = 0;
		_co = 0;
	}
	int _cap;
	int _nco;
	LPVOID _ctx;
	_coroutine_** _co;
	_coroutine_* _curco;
	slist<int> _freeid;
};
#else
#define M_COROUTINE_STACK_SIZE  4*1024*1024
struct _coroutine_ {
	int			_id;
	int			_status;
	char*		_stack;
	int			_size;
	int			_cap;
	ucontext_t	_ctx;
	void*		_data;
	_coroutine_func_ _function;
};
typedef std::map<int, _coroutine_*> CoroutineMap;
struct _schedule_ {
	_schedule_() {
		_cap = 0;
		_nco = 0;
		_curco = 0;
		_co = 0;
		_pri_stack = false;
	}
	int _cap;
	int _nco;
	bool _pri_stack;
	ucontext_t _ctx;
	_coroutine_** _co;
	_coroutine_* _curco;
	slist<int> _freeid;
	char _stack[M_COROUTINE_STACK_SIZE];
};
#endif

#ifdef M_PLATFORM_WIN
template<typename T, int N = 0>
struct _tlsdata_ {
	struct _init_ {
		DWORD _tkey;
		_init_() {
			_tkey = TlsAlloc();
		}
		~_init_() {
			delete((T*)TlsGetValue(_tkey));
			TlsFree(_tkey);
		}
	};
	static T& data() {
		T* pv = 0;
		if (0 == (pv = (T*)TlsGetValue(_data._tkey))) {
			pv = new T;
			TlsSetValue(_data._tkey, (void*)pv);
		}
		return *pv;
	}

protected:
	static _init_ _data;
};
#else
template<typename T, int N = 0>
struct _tlsdata_ {
	struct _init_ {
		pthread_key_t _tkey;
		_init_() {
			pthread_key_create(&_tkey, 0);
		}
		~_init_() {
			delete((T*)pthread_getspecific(_tkey));
			pthread_key_delete(_tkey);
		}
	};
	static T& data() {
		T* pv = 0;
		if (0 == (pv = (T*)pthread_getspecific(_data._tkey))) {
			pv = new T;
			pthread_setspecific(_data._tkey, (void*)pv);
		}
		return *pv;
	}

protected:
	static _init_ _data;
};
#endif

template<typename T, int N>
typename _tlsdata_<T, N>::_init_ _tlsdata_<T, N>::_data;

#define gschedule _tlsdata_<_schedule_>::data()
#define gpristacksize _tlsdata_<unsigned int>::data()

template<typename T>
class basecoroutine {
public:
	// init environment
	static bool initEnv(unsigned int stack_size = 128 * 1204, bool pri_stack = false);
	// create one new coroutine
	static int create(_coroutine_func_ routine, void* data);
	// close
	static void close();
	// resume
	static void resume(int co_id);
	// yield
	static void yield();
	// current coroutine id
	static unsigned int curid() {
		_schedule_& schedule = gschedule;
		if (schedule._curco) {
			return schedule._curco->_id;
		}
		return -1;
	}
	static void destroy(int co_id);

private:
	static _coroutine_* _alloc_co_(_coroutine_func_ routine, void* data) {
		_schedule_& schedule = gschedule;
		_coroutine_* co = (_coroutine_*)malloc(sizeof(_coroutine_));
		co->_function = routine;
		co->_data = data;
		co->_status = COROUTINE_READY;
		if (!schedule._freeid.empty()) {
			int id = schedule._freeid.front();
			schedule._freeid.pop_front();
			schedule._co[id] = co;
			co->_id = id;
		}
		else {
			if (schedule._nco >= schedule._cap) {
				schedule._co = (_coroutine_**)realloc(schedule._co, schedule._cap * 2 * sizeof(_coroutine_*));
				memset(schedule._co + schedule._cap, 0, schedule._cap * sizeof(_coroutine_*));
				schedule._co[schedule._cap] = co;
				schedule._cap *= 2;
				co->_id = schedule._nco++;
			}
			else {
				int id = schedule._nco++;
				schedule._co[id] = co;
				co->_id = id;
			}
		}
		return co;
	}
};

// private stack is invalid for windows fiber
class Coroutine : public basecoroutine<Coroutine> {
public:
	// init environment
	static bool initEnv(unsigned int stack_size = 128 * 1204, bool pri_stack = false) {
		return basecoroutine::initEnv(stack_size, pri_stack);
	}
	// create one new coroutine
	static int create(_coroutine_func_ routine, void* data) {
		return basecoroutine::create(routine, data);
	}
	// close
	static void close() {
		return basecoroutine::close();
	}
	// resume
	static void resume(int co_id) {
		return basecoroutine::resume(co_id);
	}
	// yield
	static void yield() {
		return basecoroutine::yield();
	}
	static unsigned int curid() {
		return basecoroutine::curid();
	}
	static void destroy(int co_id) {
		basecoroutine::destroy(co_id);
	}
};

struct co_task {
	void* p;
	void(*func)(void*);
};

struct co_task_wrapper {
	int co_id;
	co_task** task;
};

typedef slist<co_task*> tasklist;
typedef slist<co_task_wrapper*> taskwrapperlist;

#define gfreetasklist _tlsdata_<tasklist,0>::data()
#define gworktasklist _tlsdata_<tasklist,1>::data()
#define gfreecolist	_tlsdata_<taskwrapperlist,0>::data()
#define gallcolist _tlsdata_<taskwrapperlist,1>::data()

class CoroutineTask {
public:
	static void doTask() {
		if (Coroutine::curid() == -1) {
			co_task* task = _get_task();
			if (task) {
				co_task_wrapper* wrapper = _get_co_task_wrapper();
				wrapper->task = &task;
				Coroutine::resume(wrapper->co_id);
			}
		}
	}

	static void doTask(void(*func)(void*), void*p) {
		if (Coroutine::curid() == -1) {
			co_task* task = 0;
			tasklist& tl = gfreetasklist;
			if (!tl.empty()) {
				task = tl.front();
				tl.pop_front();
			}
			else {
				task = (co_task*)malloc(sizeof(co_task));
			}
			task->func = func;
			task->p = p;
			co_task_wrapper* wrapper = _get_co_task_wrapper();
			wrapper->task = &task;
			Coroutine::resume(wrapper->co_id);
		}
	}

	static void addTask(void(*func)(void*), void*p) {
		co_task* task = 0;
		tasklist& tl = gfreetasklist;
		if (!tl.empty()) {
			task = tl.front();
			tl.pop_front();
		}
		else {
			task = (co_task*)malloc(sizeof(co_task));
		}
		task->func = func;
		task->p = p;
		gworktasklist.push_back(task);
	}

	static void clrTask() {
		if (Coroutine::curid() == -1) {
			tasklist& ftl = gfreetasklist;
			while (ftl.size()) {
				free(ftl.front());
				ftl.pop_front();
			}
			tasklist& wtl = gworktasklist;
			while (wtl.size()) {
				free(wtl.front());
				wtl.pop_front();
			}
			taskwrapperlist& wtw = gallcolist;
			while (wtw.size()) {
				co_task_wrapper* wrapper = wtw.front();
				Coroutine::destroy(wrapper->co_id);
				free(wrapper);
				wtw.pop_front();
			}
		}
	}

private:

	static co_task* _get_task() {
		tasklist& tl = gworktasklist;
		if (tl.empty()) {
			return 0;
		}
		else {
			co_task* task = tl.front();
			tl.pop_front();
			return task;
		}
	}

	static co_task_wrapper* _get_co_task_wrapper() {
		co_task_wrapper* wrapper = 0;
		taskwrapperlist& tw = gfreecolist;
		if (!tw.empty()) {
			wrapper = tw.front();
			tw.pop_front();
		}
		else {
			wrapper = (co_task_wrapper*)malloc(sizeof(co_task_wrapper));
			wrapper->co_id = Coroutine::create(_co_task_func_, wrapper);
			gallcolist.push_back(wrapper);
		}
		wrapper->task = 0;
		return wrapper;
	}

	static void _co_task_func_(void* p) {
		co_task_wrapper* wrapper = (co_task_wrapper*)p;
		tasklist& tl = gfreetasklist;
		taskwrapperlist& ftl = gfreecolist;
		co_task* task = 0;
		while (wrapper->task) {
			task = *(wrapper->task);
			if (task) {
				task->func(task->p);
				tl.push_back(task);
			}
			ftl.push_back(wrapper);
			Coroutine::yield();
		}
		free(wrapper);
	}
};

M_COROUTINE_NAMESPACE_END
#endif // end for M_COROUTINE_COROUTINE_INCLUDE

#include "coroutine/co_linux_impl.hpp"
#include "coroutine/co_win_impl.hpp"