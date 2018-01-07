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

int gcoid1 = -1;
int gcoid2 = -1;
int gcoid3 = -1;
int gcoid4 = -1;

base::MutexLock gmutex;

void func(void*p) {
	{
		base::ScopedLock sc(gmutex);
		cout << "fun sub coroutine " << Coroutine::curid() << " " << base::thread::ctid() << endl;
	}
	if (gcoid1 == -1)
		gcoid1 = Coroutine::curid();
	else
		gcoid2 = Coroutine::curid();
	Coroutine::yield();
	{
		base::ScopedLock sc(gmutex);
		cout << "fun sub coroutine " << Coroutine::curid() << " " << base::thread::ctid() << endl;
	}
}

void func2(void*p) {
	{
		base::ScopedLock sc(gmutex);
		cout << "fun2 sub coroutine " << Coroutine::curid() << " " << base::thread::ctid() << endl;
	}
	if (gcoid3 == -1)
		gcoid3 = Coroutine::curid();
	else
		gcoid4 = Coroutine::curid();
	Coroutine::yield();
	{
		base::ScopedLock sc(gmutex);
		cout << "fun2 sub coroutine " << Coroutine::curid() << " " << base::thread::ctid() << endl;
	}
}

void thread_func(void*) {
	Coroutine::initEnv();
	CoroutineTask::doTask(func, 0);
	CoroutineTask::doTask(func, 0);
	while (true) {
		CoroutineTask::doThrResume();
		base::thread::sleep(2);
	}
}

void thread_func2(void*) {
	Coroutine::initEnv();
	CoroutineTask::doTask(func2, 0);
	CoroutineTask::doTask(func2, 0);
	while (true) {
		CoroutineTask::doThrResume();
		base::thread::sleep(2);
	}
}

int main() {

	base::thread thr(thread_func, 0);
	base::thread thr2(thread_func2, 0);
	int i = 0;
	cin >> i;
	CoroutineTask::addResume(thr.tid(), gcoid1);
	CoroutineTask::addResume(thr.tid(), gcoid2);
	CoroutineTask::addResume(thr2.tid(), gcoid3);
	CoroutineTask::addResume(thr2.tid(), gcoid4);
	cin >> i;
	return 0;
}