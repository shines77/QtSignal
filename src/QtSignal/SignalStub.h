#ifndef QT_SIGNAL_STUB_H
#define QT_SIGNAL_STUB_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "SignalStubImpl.h"

namespace jimi {

template <typename ...Args>
class signal_stub
{
public:
	static signal_stub & get()
	{
		static signal_stub instance;
		return instance;
	}

	template <typename T>
	connection connect(int key, T && slot)
	{
		return signal_.connect(key, std::forward<T>(slot));
	}

	void disconnect(int key)
	{
		signal_.disconnect(key);
	}

	void emit(int key, Args && ...args)
	{
		signal_(key, std::forward<Args>(args)...);
	}

private:
	signal_stub() = default;
	signal_stub(const signal_stub &) = delete;
	signal_stub(signal_stub &&) = delete;

	safe_signal_stub_impl<void(Args...)> signal_;
};

template <typename ...Args>
class unsafe_signal_stub
{
public:
	static unsafe_signal_stub & get()
	{
		static unsafe_signal_stub instance;
		return instance;
	}

	template <typename T>
	connection connect(int key, T && slot)
	{
		return signal_.connect(key, std::forward<T>(slot));
	}

	void disconnect(int key)
	{
		signal_.disconnect(key);
	}

	void emit(int key, Args && ...args)
	{
		signal_(key, std::forward<Args>(args)...);
	}

private:
	unsafe_signal_stub() = default;
	unsafe_signal_stub(const unsafe_signal_stub &) = delete;
	unsafe_signal_stub(unsafe_signal_stub &&) = delete;

	unsafe_signal_stub_impl<void(Args...)> signal_;
};

} // namespace jimi

#endif // !QT_SIGNAL_STUB_H
