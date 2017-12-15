#include "coroutine.hpp"
#include <iostream>

using namespace std;
using namespace coroutine;

void coroutine_test(void* p) {
	static int i = 1;
	cout << "this is sub coroutine " << i << endl;
	++i;
	Coroutine::yield();
}

int main() {

	int co_count = 200;
	Coroutine::initEnv();
	for (int i = 0; i < co_count; ++i) {
		int id = Coroutine::create(coroutine_test, 0);
	}
	for (int i = 1; i <= co_count; ++i) {
		cout << "this is main coroutine" << endl;
		Coroutine::resume(i);
	}
	Coroutine::close();
	cout << "all finish" << endl;
	return 0;
}