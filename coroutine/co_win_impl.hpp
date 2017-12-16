#ifndef M_COROUTINE_COWIN_IMPL_INCLUDE
#define M_COROUTINE_COWIN_IMPL_INCLUDE

#ifdef M_PLATFORM_WIN
M_COROUTINE_NAMESPACE_BEGIN

template<int N>
void __stdcall pub_coroutine(LPVOID p);

template<typename T>
bool basecoroutine<T>::initEnv(unsigned int stack_size) {
	if (_stack_size == 0)
		_stack_size = stack_size;
	_schedule_& schedule = gschedule;
	if (!schedule._mainctx) {
		LPVOID ctx = ::ConvertThreadToFiberEx(0, FIBER_FLAG_FLOAT_SWITCH);
		if (!ctx) {
			DWORD error = ::GetLastError();
			return false;
		}
		else {
			schedule._mainctx = ctx;
			schedule._index = 1;
			schedule._cur_co = 0;
		}
	}
	return true;
}

template<typename T>
int basecoroutine<T>::create(_coroutine_func_ routine, void* data) {
	LPVOID ctx = ::CreateFiberEx(_stack_size, 0, FIBER_FLAG_FLOAT_SWITCH, pub_coroutine<0>, 0);
	if (ctx) {
		_coroutine_* co = (_coroutine_*)malloc(sizeof(_coroutine_));
		if (co) {
			_schedule_& schedule = gschedule;
			int index = schedule._index++;
			co->_ctx = ctx;
			co->_function = routine;
			co->_data = data;
			co->_status = COROUTINE_READY;
			co->_id = index;
			schedule._co[index % 1024].insert(std::make_pair(index, co));
			return index;
		}
		return -1;
	}
	else {
		DWORD error = ::GetLastError();
		return -1;
	}
}

template<typename T>
void basecoroutine<T>::close() {
	_schedule_& schedule = gschedule;
	for (int i = 0; i < 1024; ++i) {
		for (CoroutineMap::iterator iter = schedule._co[i].begin();
			iter != schedule._co[i].end(); ++iter) {
			::DeleteFiber(iter->second->_ctx);
			free(iter->second);
		}
		schedule._co[i].clear();
	}
	schedule._cur_co = 0;
	schedule._index = 0;
	schedule._mainctx = 0;
	::ConvertFiberToThread();
}

template<typename T>
void basecoroutine<T>::resume(int co_id) {
	_schedule_& schedule = gschedule;
	if (schedule._cur_co) {
		return;
	}
	int mod = co_id % 1024;
	CoroutineMap::iterator iter = schedule._co[mod].find(co_id);
	if (iter != schedule._co[mod].end()) {
		switch (iter->second->_status) {
		case COROUTINE_READY:
		case COROUTINE_SUSPEND:
			iter->second->_status = COROUTINE_RUNNING;
			schedule._cur_co = iter->second;
			::SwitchToFiber(schedule._cur_co->_ctx);
			if (schedule._cur_co->_status == COROUTINE_DEAD) {
				schedule._co[mod].erase(co_id);
				free(schedule._cur_co);
			}
			schedule._cur_co = 0;
			break;
		}
	}
}

template<typename T>
void basecoroutine<T>::yield() {
	_schedule_& schedule = gschedule;
	if (schedule._cur_co) {
		schedule._cur_co->_status = COROUTINE_SUSPEND;
		::SwitchToFiber(schedule._mainctx);
	}
}

template<int N>
void __stdcall pub_coroutine(LPVOID p) {
	_schedule_& schedule = gschedule;
	if (schedule._cur_co) {
		(schedule._cur_co->_function)(schedule._cur_co->_data);
		schedule._cur_co->_status = COROUTINE_DEAD;
		::SwitchToFiber(schedule._mainctx);
	}
}

M_COROUTINE_NAMESPACE_END
#endif
#endif