#ifndef M_COROUTINE_COROUTINE_INCLUDE
#define M_COROUTINE_COROUTINE_INCLUDE

#include "config.hpp"
M_COROUTINE_NAMESPACE_BEGIN

typedef void(*_coroutine_func_)(void*ud);
#define COROUTINE_READY   (1)
#define COROUTINE_RUNNING (2)
#define COROUTINE_SUSPEND (3)
#define COROUTINE_DEAD	  (4)

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
		_mainctx = 0;
		_index = 0;
		_cur_co = 0;
	}
	int _index;
	LPVOID _mainctx;
	_coroutine_* _cur_co;
	CoroutineMap _co[1024];
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
		_cur_co = 0;
		_index = -1;
	}
	int	_index;
	ucontext_t _mainctx;
	_coroutine_* _cur_co;
	char _stack[M_COROUTINE_STACK_SIZE];
	CoroutineMap _co[1024];
};
#endif

#ifdef M_PLATFORM_WIN
template<typename T>
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
template<typename T>
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

template<typename T>
typename _tlsdata_<T>::_init_ _tlsdata_<T>::_data;

#define gschedule _tlsdata_<_schedule_>::data()

template<typename T>
class basecoroutine {
public:
	// init environment
	static bool initEnv(unsigned int stack_size = 128 * 1204);
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
		if (schedule._cur_co) {
			return schedule._cur_co->_id;
		}
		return 0;
	}

protected:
	static unsigned int _stack_size;
};

template<typename T>
unsigned int basecoroutine<T>::_stack_size = 0;

class Coroutine : public basecoroutine<Coroutine>{
public:
	// init environment
	static bool initEnv(unsigned int stack_size = 128 * 1204) {
		return basecoroutine::initEnv(stack_size);
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
};

M_COROUTINE_NAMESPACE_END
#endif // end for M_COROUTINE_COROUTINE_INCLUDE

#include "co_linux_impl.hpp"
#include "co_win_impl.hpp"