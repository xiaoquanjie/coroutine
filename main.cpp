#include "coroutine/coroutine.hpp"
#include <iostream>
#include <time.h>
#include <list>

using namespace std;
using namespace coroutine;

clock_t gbeg_time = 0;
clock_t gend_time = 0;
void print_clock(bool beg) {
	if (beg)
		gbeg_time = clock();
	else
		gend_time = clock();
	if (beg)
		cout << "begin : " << ((double)gbeg_time / CLOCKS_PER_SEC) << endl;
	else {
		cout << "end : " << ((double)gend_time / CLOCKS_PER_SEC) << endl;
		cout << "elapsed : " << ((double)(gend_time - gbeg_time) / CLOCKS_PER_SEC) << endl;
	}
}

template<typename T>
void print_c(const T& t) {
	for (typename T::const_iterator iter = t.begin(); iter != t.end(); ++iter) {
		cout << iter->first << " " << iter->second << endl;
	}
}

void coroutine_test(void* p) {
	int id = Coroutine::curid();
	cout << "this is sub coroutine " << id << endl;
	std::map<int, int> m1;
	m1[3] = 3;
	m1[1] = 1;
	m1[2] = 2;
	print_c(m1);
	Coroutine::yield();
	cout << "this is sub coroutine " << id << endl;
	m1[4] = 4;
	m1[5] = 5;
	print_c(m1);
}

enum {
	task_free,
	task_ready,
	task_over,
};

struct co_task {
	int id;
	int status;
	void* param;
	void(*fun)(void*);
};

void co_task_func(void* p) {
	co_task* task = (co_task*)p;
	while (task->fun) {
		task->fun(task->param);
		Coroutine::yield();
	}
}

void func(void*p) {
	cout << "nihao" << endl;
}

class CoroutinePool {
public:
	static void addTask(void(*fun)(void*), void*p) {
		
	}

protected:
	co_task* _getTask(void(*fun)(void*), void*p) {

	}

private:
	std::list<co_task*> _freelist;
	std::list<co_task*> _worklist;

	std::map<int, co_task*> _tasks;
	std::list<int> _idlist;
};

int main() {

	int co_count = 20;
	Coroutine::initEnv();
	
	std::list<int> idlist;
	int id = Coroutine::create(co_task_func, 0);
	idlist.push_back(id);
	id = Coroutine::create(co_task_func, 0);
	idlist.push_back(id);
	id = Coroutine::create(co_task_func, 0);
	idlist.push_back(id);

	Coroutine::close();
	return 0;
}