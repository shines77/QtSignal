#ifndef QT_SIGNAL_STUB_IMPL_H
#define QT_SIGNAL_STUB_IMPL_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stddef.h>

#include <vector>       // std::vector
#include <functional>   // std::function
#include <mutex>        // std::mutex, std::lock_guard
#include <memory>       // std::shared_ptr, std::weak_ptr
#include <algorithm>    // std::find_if()
#include <cassert>      // assert()
#include <thread>       // std::this_thread::yield()
#include <type_traits>  // std::is_same
#include <iterator>     // std::back_inserter
#include <map>          // std::multimap<T>

#include "SignalCommon.h"
#include "Functional.h"

namespace jimi {

template <typename ThreadPolicy, typename Func>
class signal_stub_impl;

template <typename ThreadPolicy, typename Ret, typename ...Args>
class signal_stub_impl<ThreadPolicy, Ret(Args...)>
{
public:
    /// Type that will be used to store the slots for this signal type.
    using slot_type = std::function<Ret(Args...)>;
    /// Type that is used for counting the slots connected to this signal.
    using size_type = typename std::vector<slot_type>::size_type;
    /// Type of this signal class.
    using this_type = signal_stub_impl<ThreadPolicy, Ret(Args...)>;

private:
    /// Thread policy currently in use
    using thread_policy = ThreadPolicy;
    /// Type of mutex, provided by threading policy
    using mutex_type = typename thread_policy::mutex_type;
    /// Type of mutex lock, provided by threading policy
    using mutex_lock_type = typename thread_policy::mutex_lock_type;

    /// Forward declaration
    struct disconnector;

    /// Mutex to syncronize access to the slot vector
    mutable mutex_type mutex_;
    /// Vector of all connected slots
    std::multimap<std::size_t, slot_type> slots_;
    /// Number of connected slots
    size_type slot_count_;
    /// Disconnector operation, used for executing disconnection in a
    /// type erased manner.
    disconnector disconnector_;
    /// Shared pointer to the disconnector. All connection objects has a
    /// weak pointer to this pointer for performing disconnections.
    std::shared_ptr<detail::disconnector> shared_disconnector_;

public:
    /// signals are default constructible
    signal_stub_impl() : slot_count_(0) {}

    /// signals are not copy constructible
    signal_stub_impl(signal_stub_impl const &) = delete;
    /// signals are not copy assignable
    signal_stub_impl & operator = (signal_stub_impl const &) = delete;

    // Destruct the signal object.
    ~signal_stub_impl() {
        // If we are unlucky, some of the connected slots
        // might be in the process of disconnecting from other threads.
        // If this happens, we are risking to destruct the disconnector
        // object managed by our shared pointer before they are done
        // disconnecting. This would be bad. To solve this problem, we
        // discard the shared pointer (that is pointing to the disconnector
        // object within our own instance), but keep a weak pointer to that
        // instance. We then stall the destruction until all other weak
        // pointers have released their "lock" (indicated by the fact that
        // we will get a nullptr when locking our weak pointer).
        std::weak_ptr<detail::disconnector> weak_disconnector{ shared_disconnector_ };
        shared_disconnector_.reset();
        while (weak_disconnector.lock() != nullptr) {
            // we just yield here, allowing the OS to reschedule. We do
            // this until all threads has released the disconnector object.
            thread_policy::yield_thread();
        }
    }

    size_type slot_count() const {
        return slot_count_;
    }

    bool is_empty() const {
        return (slot_count() == 0);
    }

    void operator () (std::size_t key, Args const & ...args) const {
        for (auto const & it : copy_slots()) {
            if (it.second && it.first == key) {
                it.second(args...);
            }
        }
    }

    template <typename Slot>
    connection connect(std::size_t key, Slot && slot) {
        mutex_lock_type lock{ mutex_ };
        slots_.emplace(key, std::forward<Slot>(slot));
        std::size_t index = slots_.size() - 1;
        if (shared_disconnector_ == nullptr) {
            disconnector_ = disconnector{ this };
            shared_disconnector_ = std::shared_ptr<detail::disconnector>{ &disconnector_, detail::dont_delete };
        }
        ++slot_count_;
        return connection{ index, key, shared_disconnector_ };
    }

    void disconnect(std::size_t key) {
        mutex_lock_type lock(mutex_);
#if 1
        // Because when std::multimap earse a item, the iterator will be invalid.
        // So we record the delete iterators on the first run.
        std::vector<typename std::multimap<std::size_t, slot_type>::iterator> del_iters;
        for (auto iter = slots_.begin(); iter != slots_.end(); ++iter) {
            if (iter->first == key) {
                del_iters.push_back(iter);
            }
        }

        // And then we delete the iterators on the second run.
        assert(del_iters.size() <= slot_count_);
        for (unsigned i = 0; i < del_iters.size(); ++i) {
            slots_.erase(del_iters[i]);
            slot_count_--;
            if (slot_count_ < 0)
                break;
        }
#else
        bool done = false;
        while (!done) {
            done = true;
            for (auto iter = slots_.cbegin(); iter != slots_.cend(); ++iter) {
                if (iter->first == key) {
                    // Because when std::multimap earse a item, the iterator will be invalid,
                    // so we must rescan from begin.
                    slots_.erase(iter);
                    slot_count_--;
                    done = false;
                    break;
                }
            }
        }
#endif
    }

private:
    //template <typename ThreadPolicy, typename T, typename R, typename Args...>
    //friend class signal_accumulator;

    struct disconnector : detail::disconnector
    {
        disconnector() : signal_host_(nullptr) {}
        disconnector(this_type * ptr) : signal_host_(ptr) {}

        void operator () (std::size_t key) const override {
            if (signal_host_) {
                signal_host_->disconnect(key);
            }
        }

        /// Pointer to the current signal.
        this_type * signal_host_;
    };

    ///
    /// Retrieve a copy of the current slots
    ///
    /// It's useful and necessary to copy the slots so we don't need
    /// to hold the lock while calling the slots. If we hold the lock
    /// we prevent the called slots from modifying the slots vector.
    /// This simple "double buffering" will allow slots to disconnect
    /// themself or other slots and connect new slots.
    ///
    std::multimap<std::size_t, slot_type> copy_slots() const
    {
        mutex_lock_type lock{ mutex_ };
        return slots_;
    }
};

template <typename T>
using safe_signal_stub_impl = signal_stub_impl<multi_thread_policy, T>;

template <typename T>
using unsafe_signal_stub_impl = signal_stub_impl<singel_thread_policy, T>;

} // namespace jimi

#endif // !QT_SIGNAL_STUB_IMPL_H
