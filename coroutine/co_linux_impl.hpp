#ifndef M_COROUTINE_COLINUX_IMPL_INCLUDE
#define M_COROUTINE_COLINUX_IMPL_INCLUDE

#ifndef M_PLATFORM_WIN
M_COROUTINE_NAMESPACE_BEGIN

template<int N>
void pub_coroutine();

template<int N>
void save_stack(_coroutine_* c, char* top);

template<typename T>
bool basecoroutine<T>::initEnv(unsigned int stack_size) {
	if (_stack_size == 0)
		_stack_size = stack_size;
	_schedule_& schedule = gschedule;
	if (schedule._index == -1) {
		schedule._cur_co = 0;
		schedule._index = 1;
	}
}

template<typename T>
int basecoroutine<T>::create(_coroutine_func_ routine, void* data) {
	_schedule_& schedule = gschedule;
	if (schedule._index != -1) {
		int index = schedule._index++;
		_coroutine_* co = (_coroutine_*)malloc(sizeof(_coroutine_));
		co->_function = routine;
		co->_data = data;
		co->_status = COROUTINE_READY;
		co->_id = index;
		co->_stack = 0;
		co->_cap = 0;
		co->_size = 0;
		schedule._co[index % 1024].insert(std::make_pair(index, co));
		assert(getcontext(&co->_ctx) == 0);
		co->_ctx.uc_stack.ss_sp = schedule._stack;
		co->_ctx.uc_stack.ss_size = M_COROUTINE_STACK_SIZE;
		co->_ctx.uc_link = &schedule._mainctx;
		makecontext(&co->_ctx, pub_coroutine<0>, 0);
		return index;
	}
	return -1;
}

template<typename T>
void basecoroutine<T>::close() {
	_schedule_& schedule = gschedule;
	for (int i = 0; i < 1024; ++i) {
		for (CoroutineMap::iterator iter = schedule._co[i].begin();
			iter != schedule._co[i].end(); ++iter) {
			free(iter->second->_stack);
			free(iter->second);
		}
		schedule._co[i].clear();
	}
	schedule._cur_co = 0;
	schedule._index = 0;
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
		_coroutine_* co = iter->second;
		switch (iter->second->_status){
		case COROUTINE_SUSPEND:
			memcpy(schedule._stack + M_COROUTINE_STACK_SIZE - co->_size, co->_stack, co->_size);
		case COROUTINE_READY:
			iter->second->_status = COROUTINE_RUNNING;
			schedule._cur_co = co;
			swapcontext(&schedule._mainctx, &schedule._cur_co->_ctx);
			if (schedule._cur_co->_status == COROUTINE_DEAD) {
				schedule._co[mod].erase(co_id);
				free(schedule._cur_co->_stack);
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
		_coroutine_* co = schedule._cur_co;
		save_stack<0>(co, schedule._stack + M_COROUTINE_STACK_SIZE);
		co->_status = COROUTINE_SUSPEND;
		swapcontext(&co->_ctx, &schedule._mainctx);
	}
}

template<int N>
void pub_coroutine() {
	_schedule_& schedule = gschedule;
	if (schedule._cur_co) {
		(schedule._cur_co->_function)(schedule._cur_co->_data);
		schedule._cur_co->_status = COROUTINE_DEAD;
		swapcontext(&schedule._cur_co->_ctx, &schedule._mainctx);
	}
}

template<int N>
void save_stack(_coroutine_* c, char* top) {
	char dummy = 0;
	assert(top - &dummy <= M_COROUTINE_STACK_SIZE);
	c->_size = top - &dummy;
	if (c->_cap < c->_size) {
		free(c->_stack);
		c->_cap = c->_size;
		c->_stack = (char*)malloc(c->_cap);
	}
	memcpy(c->_stack, &dummy, c->_size);
}

M_COROUTINE_NAMESPACE_END
#endif
#endif