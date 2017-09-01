
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

enum signal_slots {
    OnValueChange,
    OnScrollChange
};

template <typename R, typename ...Args>
inline bool operator == (std::function<R(Args...)> const & func1, std::function<R(Args...)> const & func2) JIMI_NOEXCEPT {
    R (* const * ptr1)(Args...) = func1.template target<R(*)(Args...)>();
    R (* const * ptr2)(Args...) = func2.template target<R(*)(Args...)>();
    return ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
}

template <typename R, typename ...Args>
inline bool operator == (std::function<R(Args...)> && func1, std::function<R(Args...)> && func2) JIMI_NOEXCEPT {
    return true;
}

void _onValueChange1(int index) {
    std::cout << "_onValueChange1(int index): index = " << index << std::endl;
}

void _onValueChange2(int x, int y) {
    std::cout << "_onValueChange2(int x, int y): x = " << x << ", y = " << y << std::endl;
}

void _onValueChange3(int index, int x, int y) {
    std::cout << "_onValueChange3(int index, int x, int y): index = " << index
        << ", x = " << x << ", y = " << y << std::endl;
}

class A {
public:
    void onValueChange1(int index) {
        std::cout << "A::onValueChange1(int index): index = " << index << std::endl;
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
    void onValueChange1(int index) {
        std::cout << "B::onValueChange1(int index): index = " << index << std::endl;
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
    auto binder1  = jimi::bind(&A::onValueChange1, &a, _1);
    auto binder1_ = jimi::bind(&A::onValueChange1, &a, _1);
    auto binder2  = jimi::bind(&B::onValueChange2, &b, _1, _2);

    jimi::function<void(int)>           _binder1  = jimi::bind(_onValueChange1, _1);
    jimi::function<void(int, int)>      _binder2  = jimi::bind(_onValueChange2, _1, _2);
    jimi::function<void(int, int, int)> _binder3  = jimi::bind(_onValueChange3, _1, _2, _3);

    _binder1(4);
    _binder2(4, 5);
    _binder3(3, 4, 5);

    jimi::function<void(int)> A_OnValueChange1 = jimi::bind(&A::onValueChange1, &a, _1);
    A_OnValueChange1(100);
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
    //std::function<void(int)> memfunc_a1 = std::bind(&A::onValueChange1, &a, _1);
    //std::function<void(int)> memfunc_a2 = std::bind(&A::onValueChange2, &a, _1);
    std::function<void(int)> memfunc_b = std::bind(&B::onValueChange1, &b, _1);
    std::function<void(int)> memfunc_a1 = std::bind(&_onValueChange1, _1);
    std::function<void(int)> memfunc_a2 = std::bind(&_onValueChange1, _1);
    //std::function<void(int)> memfunc_b = std::bind(&_onValueChange1, _1);
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

class FooA
{
public:
	FooA() {};
	~FooA() {};

	static VOID CALLBACK WaitOrTimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
	{
        if (lpParameter != NULL) {
		    ((FooA *)lpParameter)->OnWaitOrTimerCallback(TimerOrWaitFired);
        }
	}
protected:
	void OnWaitOrTimerCallback(BOOLEAN TimerOrWaitFired)
	{
        printf("call OnWaitOrTimerCallback()\n");
	}

public:
    void OnWaitOrTimerCallbackWrapper(BOOLEAN TimerOrWaitFired)
	{
        printf("call OnWaitOrTimerCallbackWrapper()\n");
	}
};

typedef struct _PARAM_DATA {
    std::function<void(BOOLEAN)> Callback;
    PVOID Host;
} PARAM_DATA, * PPARAM_DATA;

template <typename T>
class CallbackWrapper {
public:
	static VOID CALLBACK WaitOrTimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
	{
        PARAM_DATA * ParamData = (PARAM_DATA *)lpParameter;
        if (ParamData != NULL) {
            if (ParamData->Callback) {
                (ParamData->Callback)(TimerOrWaitFired);
            }
            else if (ParamData->Host != NULL) {
		        ((T *)ParamData->Host)->OnWaitOrTimerCallbackWrapper(TimerOrWaitFired);
            }
            else {
                // Unknown error
            }
        }
	}
};

template <typename T>
BOOL CreateTimerQueueTimerWrapper(
    _Outptr_ PHANDLE phNewTimer,
    _In_opt_ HANDLE TimerQueue,
    _In_ std::function<void(BOOLEAN)> const & Callback,
    _In_ T const & Host,
    _In_ PARAM_DATA & ParamData,
    _In_opt_ PVOID Parameter,
    _In_ DWORD DueTime,
    _In_ DWORD Period,
    _In_ ULONG Flags
    )
{
    ParamData.Callback = Callback;
    ParamData.Host = (PVOID)const_cast<T *>(&Host);
    return CreateTimerQueueTimer(phNewTimer, TimerQueue, &CallbackWrapper<T>::WaitOrTimerCallback,
                                 (PVOID)&ParamData, DueTime, Period, Flags);
}

void run_unittest()
{
    // TODO:
}

BOOL WINAPI CosonleHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType) {
    case CTRL_C_EVENT:
        printf("CosonleHandler::CTRL_C_EVENT\n");
        break;
    case CTRL_BREAK_EVENT:
        printf("CosonleHandler::CTRL_BREAK_EVENT\n");
        break;
    case CTRL_CLOSE_EVENT:
        printf("CosonleHandler::CTRL_CLOSE_EVENT\n");
        break;
    case CTRL_LOGOFF_EVENT:
        printf("CosonleHandler::CTRL_LOGOFF_EVENT\n");
        break;
    case CTRL_SHUTDOWN_EVENT:
        printf("CosonleHandler::CTRL_SHUTDOWN_EVENT\n");
        break;
    default:
        printf("CosonleHandler:: Unknown ctrl type: %u.\n", dwCtrlType);
        break;
    }
    return TRUE;
}

void integer_constant_test()
{
	int i1, i2, i3, i4, i5, i6;
	i1 = -2147483647;
	i2 = -2147483648;
	i3 = -2147483649;
	i4 = 2147483647;
	i5 = 2147483648;
	i6 = 2147483649;
	printf("integer_constant_test()\n");
}

int main(int argn, char * argv[])
{
    SetConsoleCtrlHandler(CosonleHandler, TRUE);

#if !defined(NDEBUG)
    run_unittest();
#endif

    integer_constant_test();

#if 1
    test_signal();
    test_signal_stub();
#endif

    int i = 0;
    for (;;) {
        i++;
    }
    printf("i = %d\n", i);

#if defined(_WIN32) && (defined(NDEBUG) || !defined(NDEBUG))
    ::system("pause");
#endif
    return 0;
}
