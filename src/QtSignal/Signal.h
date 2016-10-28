#ifndef QT_SIGNAL_H
#define QT_SIGNAL_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "SignalImpl.h"

namespace jimi {

template <typename ...Args>
class signal
{
public:
    /// signals are default constructible
    signal() = default;

	template <typename Slot>
	connection connect(Slot && slot)
	{
		return signal_.connect(std::forward<T>(slot));
	}

    template <typename Slot>
	void disconnect(Slot && slot)
	{
		signal_.disconnect(std::forward<T>(slot));
	}

    template <typename Func>
	void disconnect(Func && func, Args && ...args)
	{
		signal_.disconnect(std::forward<Func>(func), std::forward<Args>(args)...);
	}

	void emit(Args && ...args)
	{
		signal_(key, std::forward<Args>(args)...);
	}

private:
    /// signals are not copy constructible
    signal(signal const &) = delete;
    /// signals are not move constructible
	signal(signal &&) = delete;
    /// signals are not copy assignable
    signal & operator = (signal const &) = delete;

	safe_signal_impl<void(Args...)> signal_;
};

template <typename ...Args>
class unsafe_signal
{
public:
    /// signals are default constructible
    unsafe_signal() = default;

	template <typename Slot>
	connection connect(Slot && slot)
	{
		return signal_.connect(std::forward<T>(slot));
	}

    template <typename Slot>
	void disconnect(Slot && slot)
	{
		signal_.disconnect(std::forward<T>(slot));
	}

	void emit(Args && ...args)
	{
		signal_(key, std::forward<Args>(args)...);
	}

private:
    /// signals are not copy constructible
    unsafe_signal(unsafe_signal const &) = delete;
    /// signals are not move constructible
	unsafe_signal(unsafe_signal &&) = delete;
    /// signals are not copy assignable
    unsafe_signal & operator = (unsafe_signal const &) = delete;

	unsafe_signal_impl<void(Args...)> signal_;
};

} // namespace jimi

#endif // !QT_SIGNAL_H
