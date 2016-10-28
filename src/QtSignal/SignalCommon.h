#ifndef QT_SIGNAL_COMMON_H
#define QT_SIGNAL_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

namespace jimi {

// Forward declaration
template <typename ThreadPolicy, typename Func>
class signal_impl;

// Forward declaration
template <typename ThreadPolicy, typename Func>
class signal_stub_impl;

// implementational details
namespace detail {

    /// Interface for type erasure when disconnecting slots.
    struct disconnector
    {
	    virtual void operator() (std::size_t index) const = 0;
    };

    /// Deleter that doesn't delete.
    static inline void dont_delete(disconnector * p) {
        // Do nothing!!
    };

} // namespace detail

struct singel_thread_policy
{
	/// Dummy mutex type that doesn't do anything
	struct mutex_type{};

	/// Dummy lock type, that doesn't do any locking.
	struct mutex_lock_type
	{
		/// A lock type must be constructible from a
		/// mutex type from the same thread policy.
		explicit mutex_lock_type(mutex_type const &) {
		}
	};

	/// Dummy implementation of thread yielding, that
	/// doesn't do any actual yielding.
	static void yield_thread() {
	}
};

struct multi_thread_policy
{
	using mutex_type = std::mutex;
	using mutex_lock_type = std::lock_guard<mutex_type>;

	/// Function that yields the current thread, allowing
	/// the OS to reschedule.
	static void yield_thread() {
		std::this_thread::yield();
	}
};

class connection
{
private:
	/// Slot index of the connected slot.
    std::size_t key_;
	/// Weak pointer to the current disconnector functor.
	std::weak_ptr<detail::disconnector> weak_disconnector_;

public:
	connection() : key_(0) {
    }
	connection(connection const & src) = delete;
	connection & operator =(connection const & src) = delete;

	connection(connection && other) : key_(other.key_),
        weak_disconnector_(std::move(other.weak_disconnector_)) {
    }

	connection & operator =(connection && other) {
		key_ = other.key_;
        weak_disconnector_ = std::move(other.weak_disconnector_);
		return (*this);
	}

	bool connected() const {
		return (!weak_disconnector_.expired());
	}

    void connect(int key, std::shared_ptr<detail::disconnector> const & shared_disconnector) {
        //
    }

    // Implementation of the disconnect operation of the connection class
	void disconnect() {
		auto ptr = weak_disconnector_.lock();
		if (ptr != nullptr) {
			(*ptr)(key_);
		}
		weak_disconnector_.reset();
    }

private:
	template <typename ThreadPolicy, typename Func>
    friend class signal_impl;

	template <typename ThreadPolicy, typename Func>
    friend class signal_stub_impl;

	connection(std::size_t index, std::size_t key,
               std::shared_ptr<detail::disconnector> const & shared_disconnector) :
        key_(key), weak_disconnector_(shared_disconnector) {
    }
};

class scoped_connection
{
private:
    /// Underlying connection object
    connection connection_;

public:
    scoped_connection() = default;
    scoped_connection(scoped_connection const &) = delete;
    scoped_connection & operator =(scoped_connection const &) = delete;
    scoped_connection(scoped_connection && other) :
        connection_(std::move(other.connection_)) {
    }

    scoped_connection(connection && conn) :
        connection_(std::forward<connection>(conn)) {
    }

    scoped_connection & operator =(connection && conn) {
        reset(std::forward<connection>(conn));
        return (*this);
    }

    ~scoped_connection() {
        disconnect();
    }

    void reset(connection && conn = {}) {
        disconnect();
        connection_ = std::move(conn);
    }

    connection release() {
        connection conn = std::move(connection_);
        connection_ = connection{};
        return conn;
    }

    bool connected() const {
        return connection_.connected();
    }

    void connect() {
        //connection_.connect();
    }

    void disconnect() {
        connection_.disconnect();
    }
};

} // namespace jimi

#endif // !QT_SIGNAL_COMMON_H
