
#include <stdio.h>
#include <stdlib.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
#include <tuple>
#include <type_traits>

#include "Signal.h"
#include "SignalStub.h"
#include "Functional.h"

enum signal_slots {
    OnValueChange,
    OnScrollChange
};

template <typename R, typename ...Args>
inline bool operator == (std::function<R(Args...)> const & func1, std::function<R(Args...)> const & func2) _NOEXCEPT {
    R (* const * ptr1)(Args...) = func1.template target<R(*)(Args...)>();
    R (* const * ptr2)(Args...) = func2.template target<R(*)(Args...)>();
    return ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
}

template <typename R, typename ...Args>
inline bool operator == (std::function<R(Args...)> && func1, std::function<R(Args...)> && func2) _NOEXCEPT {
    return true;
}

void _onValueChange(int index) {
    std::cout << "_onValueChange(int index): index = " << index << std::endl;
}

void _onValueChange2(int x, int y) {
    std::cout << "_onValueChange(int x, int y): x = " << x << ", y = " << y << std::endl;
}

void _onValueChange3(int index, int x, int y) {
    std::cout << "_onValueChange3(int index, int x, int y): index = " << index
        << ", x = " << x << ", y = " << y << std::endl;
}

class A {
public:
    void onValueChange(int index) {
        std::cout << "A::onValueChange(int index): index = " << index << std::endl;
    }

    void onValueChange2(int x, int y) {
        std::cout << "A::onValueChange2(int x, int y): x = " << x << ", y = " << y << std::endl;
    }

    void onValueChange3(int index, int x, int y) {
        std::cout << "A::onValueChange3(int index, int x, int y): index = " << index
            << ", x = " << x << ", y = " << y << std::endl;
    }
};

class B {
public:
    void onValueChange(int index) {
        std::cout << "B::onValueChange(int index): index = " << index << std::endl;
    }

    void onValueChange2(int x, int y) {
        std::cout << "B::onValueChange2(int x, int y): x = " << x << ", y = " << y << std::endl;
    }

    void onValueChange3(int index, int x, int y) {
        std::cout << "B::onValueChange3(int index, int x, int y): index = " << index
            << ", x = " << x << ", y = " << y << std::endl;
    }
};

void test_signal()
{
    using namespace std::placeholders;

    A a; B b;
    auto binder1  = jimi::bind(&A::onValueChange, &a, _1);
    auto binder1_ = jimi::bind(&A::onValueChange, &a, _1);
    auto binder2  = jimi::bind(&B::onValueChange, &b, _1);
}

void test_signal_stub()
{
    using namespace std::placeholders;
    typedef jimi::signal_stub<> signal_0;
    signal_0 & signal_0_inst = signal_0::get();

    signal_0_inst.connect(signal_slots::OnValueChange, []() { std::cout << "lambda::onValueChange()." << std::endl; });
    signal_0_inst.emit(signal_slots::OnValueChange);
    signal_0_inst.disconnect(signal_slots::OnValueChange);

    typedef jimi::signal_stub<int> signal_int;
    signal_int & signal_int_inst = signal_int::get();

    A a; B b;
    //std::function<void(int)> memfunc_a1 = std::bind(&A::onValueChange, &a, _1);
    //std::function<void(int)> memfunc_a2 = std::bind(&A::onValueChange, &a, _1);
    std::function<void(int)> memfunc_b = std::bind(&B::onValueChange, &b, _1);
    std::function<void(int)> memfunc_a1 = std::bind(&_onValueChange, _1);
    std::function<void(int)> memfunc_a2 = std::bind(&_onValueChange, _1);
    //std::function<void(int)> memfunc_b = std::bind(&_onValueChange, _1);
    printf("memfunc_a1.target_type().name() = %s\n\n", memfunc_a1.target_type().name());
    printf("jimi::getAddressOf(memfunc_a1) = 0x%p\n", (void *)jimi::getAddressOf(memfunc_a1));
    printf("jimi::getAddressOf(memfunc_a2) = 0x%p\n", (void *)jimi::getAddressOf(memfunc_a2));
    printf("\n");
    //std::function<void(A *, int)> memfunc_a(A::onValueChange, &a, _1);
    //printf("(memfunc_a1 == memfunc_a2) ? %d\n", (int)(memfunc_a1 == memfunc_a2));
    printf("(memfunc_a1 == memfunc_a2) ? %d\n", (int)jimi::function_equal<A>(memfunc_a1, memfunc_a2));
    signal_int_inst.connect(signal_slots::OnValueChange, memfunc_a1);
    signal_int_inst.connect(signal_slots::OnValueChange, memfunc_b);
    signal_int_inst.emit(signal_slots::OnValueChange, 100);
    signal_int_inst.disconnect(signal_slots::OnValueChange);

    std::function<void(int, int, int)> memfunc_a1_ = std::bind(&A::onValueChange3, &a, _1, _2, _3);
    std::function<void(int, int, int)> memfunc_a2_ = std::bind(&A::onValueChange3, &a, _1, _2, _3);

    printf("memfunc_a1_.target_type().name() = %s\n\n", memfunc_a1_.target_type().name());
    printf("(memfunc_a1_ == memfunc_a2_) ? %d\n", (int)jimi::function_equal<A>(memfunc_a1_, memfunc_a2_));
}

void run_unittest()
{
    // TODO:
}

void test_atomic()
{
	using semaphore_type = semaphore;

	semaphore_type sema_1{ 1 };
	semaphore_type sema_2{ 1 };
	semaphore_type end_sema{ 2 };

	//int x, y, r1, r2;
	std::atomic<int> x, y, r1, r2;
    std::atomic<bool> exit_flag = true;

    printf("--------------------------------------------------------------------\n");
    printf("test_atomic():\n\n");

	std::thread t1
	{
		[&]()
		{
			while (exit_flag.load())
			{
				//sema_1.wait();
				while (std::rand() % 8 != 0);

				// transaction thread 1
				x.store(1);
				r1.store(y.load());
				///////////////////////

				//end_sema.signal();
			}
		}
	};

    std::thread t2
    {
	    [&]()
	    {
		    while (exit_flag.load())
		    {
			    //sema_2.wait();
			    while (std::rand() % 8 != 0);

			    // transaction thread 2
			    y.store(1);
			    r2.store(x.load());
			    ///////////////////////

			    //end_sema.signal();
		    }
	    }
    };

    static const size_t max_iterations = 500000;
	size_t detected_0_0 = 0, detected_0_1 = 0, detected_1_0 = 0, detected_1_1 = 0;
    size_t detected_other = 0, detected_total = 0;
	for (auto iterations = 0; iterations < max_iterations; ++iterations)
	{
		x = 0; y = 0; r1 = 0; r2 = 0;
		//sema_1.signal();
		//sema_2.signal();
		//end_sema.wait();
		//end_sema.wait();

        detected_total++;
		if (r1 == 0 && r2 == 0)
			detected_0_0++;
		else if (r1 == 0 && r2 == 1)
			detected_0_1++;
		else if (r1 == 1 && r2 == 0)
			detected_1_0++;
		else if (r1 == 1 && r2 == 1)
			detected_1_1++;
        else
            detected_other++;   // Other unknown result.
	}

    exit_flag.store(false);
	//sema_1.signal();
	//sema_2.signal();
	//end_sema.wait();
	//end_sema.wait();

    printf("[r1, r2] order result [0, 0]: %8zu / %zu, %5.1f %% detected.\n",
        detected_0_0, detected_total, ((double)detected_0_0  * 100.0/ detected_total));
    printf("[r1, r2] order result [0, 1]: %8zu / %zu, %5.1f %% detected.\n",
        detected_0_1, detected_total, ((double)detected_0_1  * 100.0/ detected_total));
    printf("[r1, r2] order result [1, 0]: %8zu / %zu, %5.1f %% detected.\n",
        detected_1_0, detected_total, ((double)detected_1_0  * 100.0/ detected_total));
    printf("[r1, r2] order result [1, 1]: %8zu / %zu, %5.1f %% detected.\n",
        detected_1_1, detected_total, ((double)detected_1_1  * 100.0/ detected_total));
    printf("\n");
    printf("[r1, r2] order result [x, x]: %8zu / %zu, %5.1f %% detected.\n",
        detected_other, detected_total, ((double)detected_other  * 100.0/ detected_total));
    printf("\n");

    if (t1.joinable())
        t1.join();
    if (t2.joinable())
        t2.join();
}

void test_semaphore()
{
	using semaphore_type = semaphore;

	semaphore_type sema_1{ 1 };
	semaphore_type sema_2{ 1 };
	semaphore_type end_sema{ 2 };

	//int x, y, r1, r2;
	std::atomic<int> x, y, r1, r2;
    std::atomic<bool> exit_flag = true;

    printf("--------------------------------------------------------------------\n");
    printf("test_semaphore():\n\n");

	std::thread t1
	{
		[&]()
		{
			while (exit_flag.load())
			{
				sema_1.wait();
				while (std::rand() % 8 != 0);

				// transaction thread 1
				x.store(1);
				r1.store(y.load());
				///////////////////////

				end_sema.signal();
			}
		}
	};

    std::thread t2
    {
	    [&]()
	    {
		    while (exit_flag.load())
		    {
			    sema_2.wait();
			    while (std::rand() % 8 != 0);

			    // transaction thread 2
			    y.store(1);
			    r2.store(x.load());
			    ///////////////////////

			    end_sema.signal();
		    }
	    }
    };

    static const size_t max_iterations = 500000;
	size_t detected_0_0 = 0, detected_0_1 = 0, detected_1_0 = 0, detected_1_1 = 0;
    size_t detected_other = 0, detected_total = 0;
	for (auto iterations = 0; iterations < max_iterations; ++iterations)
	{
		x = 0; y = 0; r1 = 0; r2 = 0;
		sema_1.signal();
		sema_2.signal();
		end_sema.wait();
		end_sema.wait();

        detected_total++;
		if (r1 == 0 && r2 == 0)
			detected_0_0++;
		else if (r1 == 0 && r2 == 1)
			detected_0_1++;
		else if (r1 == 1 && r2 == 0)
			detected_1_0++;
		else if (r1 == 1 && r2 == 1)
			detected_1_1++;
        else
            detected_other++;   // Other unknown result.
	}

    exit_flag.store(false);
	sema_1.signal();
	sema_2.signal();
	end_sema.wait();
	end_sema.wait();

    printf("[r1, r2] order result [0, 0]: %8zu / %zu, %5.1f %% detected.\n",
        detected_0_0, detected_total, ((double)detected_0_0  * 100.0/ detected_total));
    printf("[r1, r2] order result [0, 1]: %8zu / %zu, %5.1f %% detected.\n",
        detected_0_1, detected_total, ((double)detected_0_1  * 100.0/ detected_total));
    printf("[r1, r2] order result [1, 0]: %8zu / %zu, %5.1f %% detected.\n",
        detected_1_0, detected_total, ((double)detected_1_0  * 100.0/ detected_total));
    printf("[r1, r2] order result [1, 1]: %8zu / %zu, %5.1f %% detected.\n",
        detected_1_1, detected_total, ((double)detected_1_1  * 100.0/ detected_total));
    printf("\n");
    printf("[r1, r2] order result [x, x]: %8zu / %zu, %5.1f %% detected.\n",
        detected_other, detected_total, ((double)detected_other  * 100.0/ detected_total));
    printf("\n");

    if (t1.joinable())
        t1.join();
    if (t2.joinable())
        t2.join();
}

int main(int argn, char * argv[])
{
#if !defined(NDEBUG)
    run_unittest();
#endif

    test_atomic();
    test_semaphore();

#if 0
    test_signal();
    test_signal_stub();
#endif

#if defined(_WIN32) && (defined(NDEBUG) || !defined(NDEBUG))
    ::system("pause");
#endif
    return 0;
}
