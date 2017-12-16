#include "coroutine.hpp"
#include <iostream>
#include <time.h>

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

void coroutine_test(void* p) {
	static int i = 1;
	//cout << "this is sub coroutine " << i << endl;
	++i;
	Coroutine::resume(2);
}

int main() {

	int co_count = 200000;
	Coroutine::initEnv();
	for (int i = 0; i < co_count; ++i) {
		int id = Coroutine::create(coroutine_test, 0);
	}
	print_clock(true);
	for (int i = 1; i <= co_count/*1000000*/; ++i) {
		//cout << "this is main coroutine" << endl;
		Coroutine::resume(i);
	}
	print_clock(false);
	Coroutine::close();
	cout << "all finish" << endl;

	//memcpy()

	return 0;
}