#ifndef QT_FUNCTIONAL_H
#define QT_FUNCTIONAL_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <functional>
#include <tuple>
#include <array>
#include <utility>
#include <type_traits>

#if defined(__GNUC__) || defined(__GNUG__) || defined(__clang__) || defined(__linux__)

#ifndef JIMI_STDCALL
#define JIMI_STDCALL    __attribute__((__stdcall__))
#endif

#ifndef JIMI_CDECL
#define JIMI_CDECL      __attribute__((__cdecl__))
#endif

#ifndef JIMI_NOEXCEPT
#define JIMI_NOEXCEPT   noexcept
#endif

#elif (defined(_MSC_VER) && (_MSC_VER != 0)) || defined(__ICL) || defined(__INTER_COMPILER)

#ifndef JIMI_STDCALL
#define JIMI_STDCALL    __stdcall
#endif

#ifndef JIMI_CDECL
#define JIMI_CDECL      __cdecl
#endif

#ifndef JIMI_NOEXCEPT
#define JIMI_NOEXCEPT   _NOEXCEPT
#endif

#endif // __GNUC__

namespace std {

#if !defined(_MSC_VER)

template <std::size_t N>
struct _Ph {};

template <class _Ret, class _Fx, class ..._Types>
class _Binder {};

#if 0
template <class _Ret, class _Fx, class T, class ..._Types>
//class _Binder : public std::_Binder<_Ret, _Fx, std::tuple<std::decay<_Types>::type...> >
class _Binder : public std::_Binder<_Ret, _Fx, _Types...>
{
    _Binder() {}
    ~_Binder() {}
};
#endif

struct _Unforced {};
#endif // !_MSC_VER

} // namespace std

namespace jimi {

#if 0
// Convert array into a tuple
template <typename Array, std::size_t ...I>
decltype(auto) make_args_tuple_impl(Array & arr, std::index_sequence<I...>)
{
    //static constexpr std::size_t M = I + 1;
    const std::_Ph<1> * arg = nullptr;
    //arr[I]... = (T)(void *)arg;
    return std::make_tuple(arr[I]...);
}

template <typename T, std::size_t N, typename Indices = std::make_index_sequence<N> >
decltype(auto) make_args_tuple(std::array<T, N> & arr)
{
    return make_args_tuple_impl(arr, Indices());
}
#endif

/*
template <std::size_t ...I, std::size_t N, typename T, typename ...Args>
struct make_args_list_impl2 : make_args_list_impl2<(I - 1)..., N, Args...>
{
};

template <std::size_t ...I, std::size_t N, typename ...Args>
struct make_args_list_impl : make_args_list_impl2<(I - 1)..., N, Args...>
{
};
//*/

#if 0
template <typename T, std::size_t N, std::size_t ...I>
struct make_args_list
{
    //typedef typename make_args_list_impl<I..., N, Args...>::type type;
    typedef typename std::_Ph<std::index_sequence<I...>::value>::type type;
};
#endif

namespace detail {

    template <typename FromFuncType, typename ToFuncType>
    static inline
    ToFuncType func_type_cast(const FromFuncType & from_func_ptr) {
        const FromFuncType * pFromFunc = reinterpret_cast<const FromFuncType *>(&from_func_ptr);
        const ToFuncType * pToFunc = reinterpret_cast<const ToFuncType *>(pFromFunc);
        ToFuncType to_func_ptr = (*const_cast<ToFuncType *>(pToFunc));
        return to_func_ptr;
    }

    ///////////////////////////////////////////////////////////////////////////

    // result type traits
    template <typename Func>
    struct result_traits_impl : result_traits_impl<decltype(&Func::operator())> {};

    template <typename T>
    struct result_traits_impl<T *> : result_traits_impl<T> {};

    /* check function */
    template <typename Ret, typename ...Args>
    struct result_traits_impl<Ret(*)(Args...)> {
        typedef Ret type;
        typedef typename std::conditional<std::is_void<Ret>::value, std::_Unforced, Ret>::type forced_type;
    };

    /* check member function */
    #ifndef RESULT_TRAITS_IMPL__
    #define RESULT_TRAITS_IMPL__(...) \
        template <typename Ret, typename Caller, typename ...Args> \
        struct result_traits_impl<Ret(Caller::*)(Args...) __VA_ARGS__> { \
            typedef Ret type; \
            typedef typename std::conditional<std::is_void<Ret>::value, std::_Unforced, Ret>::type forced_type; \
        };
    #endif

    RESULT_TRAITS_IMPL__()
    RESULT_TRAITS_IMPL__(const)
    RESULT_TRAITS_IMPL__(volatile)
    RESULT_TRAITS_IMPL__(const volatile)

    #undef RESULT_TRAITS_IMPL__

    template <typename Func>
    struct result_traits : result_traits_impl<typename std::decay<Func>::type> {};

    ///////////////////////////////////////////////////////////////////////////

    // member function type filter traits
    template <typename Func>
    struct func_traits_impl : func_traits_impl<decltype(&Func::operator())> {};

    template <typename T>
    struct func_traits_impl<T *> : func_traits_impl<T> {};

    /* check function */
    template <typename Ret, typename ...Args>
    struct func_traits_impl<Ret(*)(Args...)> {
        typedef void callable_type;
        typedef Ret(JIMI_CDECL *func_type)(Args...);
        typedef Ret(JIMI_CDECL *completed_func_type)(Args...);
        typedef Ret(JIMI_CDECL *flat_func_type)(void *, Args...);
        typedef Ret(JIMI_CDECL *callable_func_type)(Args...);
        typedef std::function<Ret(Args...)> std_func_type;
        typedef std::tuple<typename std::decay<Args>::type...> args_type;
        enum { is_member_func = false };
    };

    /* check member function */
    #ifndef FUNCTION_TRAITS_IMPL__
    #define FUNCTION_TRAITS_IMPL__(...) \
        template <typename Ret, typename Caller, typename ...Args> \
        struct func_traits_impl<Ret(Caller::*)(Args...) __VA_ARGS__> { \
            typedef Caller callable_type; \
            typedef Ret(JIMI_CDECL *func_type)(Args...); \
            typedef Ret(JIMI_CDECL *completed_func_type)(Caller *, Args...); \
            typedef Ret(JIMI_CDECL *flat_func_type)(void *, Args...); \
            typedef Ret(JIMI_CDECL callable_type::*callable_func_type)(Args...); \
            typedef std::function<Ret(Args...)> std_func_type; \
            typedef std::tuple<typename std::decay<Args>::type...> args_type; \
            enum { is_member_func = true }; \
        };
    #endif

    FUNCTION_TRAITS_IMPL__()
    FUNCTION_TRAITS_IMPL__(const)
    FUNCTION_TRAITS_IMPL__(volatile)
    FUNCTION_TRAITS_IMPL__(const volatile)

    #undef FUNCTION_TRAITS_IMPL__

    template <typename Func>
    struct func_traits : func_traits_impl<typename std::decay<Func>::type> {};

    ///////////////////////////////////////////////////////////////////////////

    struct empty_args_t {
        // Tag for no argument.
    };

    struct empty_tuple_t {
        // Tag for no argument.
    };

    template <typename Func, typename Ret>
    struct compressed_pair_no_args {
        typedef typename std::decay<Func>::type                     callable_type;
        typedef typename detail::func_traits<callable_type>::callable_type
                                                                    caller_type;
        typedef typename detail::empty_tuple_t                      args_type;
        typedef typename detail::empty_args_t                       first_type;
        typedef typename detail::empty_tuple_t                      second_type;
        typedef std::function<Ret()>                                slot_type;
    };

    template <typename Func, typename Ret, typename T, typename ...Args>
    struct compressed_pair_has_args {
        typedef typename std::decay<T>::type                        first_type;
        typedef std::function<Ret(first_type *, Args...)>           slot_type;
    };

#if 0
    template <typename Func, typename Ret, typename ...Args>
    struct compressed_pair;
#else
    template <typename Func, typename Ret, typename ...Args>
    struct compressed_pair {
        typedef typename detail::empty_args_t                       first_type;
        typedef std::function<Ret(Args...)>                         slot_type;
    };
#endif

    template <typename Func, typename Ret>
    struct compressed_pair<Func, Ret> : compressed_pair_no_args<Func, Ret> {
    };

    template <typename Func, typename Ret, typename T, typename ...Args>
    struct compressed_pair<Func, Ret, T, Args...> :
        compressed_pair_has_args<Func, Ret, T, Args...> {
        typedef typename std::decay<Func>::type                     callable_type;
        typedef typename detail::func_traits<Func>::callable_type
                                                                    caller_type;
        typedef std::tuple<typename std::decay<Args>::type...>      args_type;
        typedef typename std::decay<T>::type                        first_type;
        typedef std::tuple<typename std::decay<Args>::type...>      second_type;
        typedef std::function<Ret(first_type, Args...)>             full_slot_type;
        typedef std::function<Ret(Args...)>                         slot_type;
    };

    ///////////////////////////////////////////////////////////////////////////

    template <typename T, bool is_member_func, typename ...Args>
    struct tuple_traits {
        static T * get_first_value(Args && ...args) {
            return nullptr;
        } 
    };

    template <typename T, typename ...Args>
    struct tuple_traits<T, true, Args...> {
        typedef std::tuple<typename std::decay<Args>::type...> args_type;
        static T * get_first_value(Args && ...args) {
            args_type _tuples(std::forward<Args>(args)...);
            return static_cast<T *>(std::get<0>(_tuples));
        } 
    };

    ///////////////////////////////////////////////////////////////////////////

} // namespace detail

template <typename Func, typename Ret, typename ...Args>
class binder {
public:
    typedef std::tuple<typename std::decay<Args>::type...>      args_type;
    typedef typename std::decay<Func>::type                     callable_type;
    typedef typename detail::func_traits<Func>::callable_type   caller_type;
    typedef typename detail::result_traits<Func>::type          result_type;

    /// Type that will be used to store the slots for this signal type.
    typedef typename detail::compressed_pair<Func, Ret, Args...>::slot_type
                                                                slot_type2;
    typedef typename detail::func_traits<Func>::std_func_type
                                                                slot_type;
    typedef typename detail::func_traits<Func>::callable_func_type
                                                                slot_func_type;
    typedef typename detail::func_traits<Func>::completed_func_type
                                                                cdecl_func_type;
public:
    slot_type       slot_;
    callable_type   func_;
    cdecl_func_type cdecl_func_;
    caller_type *   caller_;
    args_type       args_;

public:
    template <typename Std_Func>
    binder(Std_Func && std_func, Func && func, Args && ...args)
        : slot_(std::forward<Std_Func>(std_func)),
          func_(std::forward<Func>(func)),
          args_(std::forward<Args>(args)...) {
#if 0
        if (binder<Func, Ret, Args...>::is_member_func()) {
            caller_ = detail::tuple_traits<caller_type, std::is_member_function_pointer<Func>::value, Args...>
                        ::get_first_value(std::forward<Args>(args)...);
            is_member_func_ = true;
        }
        else {
            caller_ = nullptr;
            is_member_func_ = false;
        }
#endif
        cdecl_func_ = detail::func_type_cast<callable_type, cdecl_func_type>(func_);
        caller_ = detail::tuple_traits<caller_type, std::is_member_function_pointer<Func>::value, Args...>
                        ::get_first_value(std::forward<Args>(args)...);
    }

    binder(binder const & src) {
        this->slot_ = src.slot_;
        this->func_ = src.func_;
        this->args_ = src.args_;
    }

    binder(binder && src) {
        this->slot_ = std::move(src.slot_);
        this->func_ = std::move(src.func_);
        this->args_ = std::move(src.args_);
    }

    binder & operator = (binder const & rhs) {
        this->slot_ = rhs.slot_;
        this->func_ = rhs.func_;
        this->args_ = rhs.args_;
        return (*this);
    }
};

template <typename Func>
class function
{
public:
    typedef Func                                                func_type;
    typedef typename std::decay<Func>::type                     callable_type;
    typedef typename detail::func_traits<Func>::callable_type   caller_type;
    typedef typename detail::result_traits<Func>::type          result_type;
    typedef typename detail::result_traits<Func>::forced_type   forced_result_type;
    typedef typename detail::func_traits<Func>::std_func_type
                                                                slot_type;
    typedef typename detail::func_traits<Func>::callable_func_type
                                                                slot_func_type;
    typedef typename detail::func_traits<Func>::flat_func_type
                                                                flat_func_type;
    typedef typename detail::func_traits<Func>::args_type       args_type;

private:
    slot_type       slot_;
    slot_func_type  func_;
    flat_func_type  flat_func_;
    void *          caller_;
    args_type       args_;

public:
    function(Func && func)
        : slot_(), func_(nullptr), args_(nullptr), caller_(nullptr) {}

#if 0
    template <typename ...Args>
    function(const binder<slot_func_type, forced_result_type, Args...> & rhs) {
        slot_ = rhs.slot_;
        func_ = rhs.func_;
    }
#endif

    template <typename Binder>
    function(const Binder & src) {
        slot_ = src.slot_;
        func_ = detail::func_type_cast<Binder::slot_func_type, slot_func_type>(src.func_);
        flat_func_ = detail::func_type_cast<Binder::slot_func_type, flat_func_type>(src.func_);
        caller_ = reinterpret_cast<void *>(src.caller_);
        //args_ = rhs.args_;
    }

    ~function() {}

    template <typename ...Args>
    result_type operator ()(Args ...args) {
        if (this->slot_)
            return this->slot_(std::forward<Args>(args)...);
        else
            return;
    }

#if 0
    template <typename ...Args>
    function & operator = (const binder<slot_func_type, forced_result_type, Args...> & rhs) {
        slot_ = rhs.slot_;
        func_ = rhs.func_;
        return (*this);
    }
#endif

    template <typename Binder>
    function & operator = (const Binder & rhs) {
        slot_ = rhs.slot_;
        func_ = detail::func_type_cast<Binder::slot_func_type, slot_func_type>(rhs.func_);
        flat_func_ = detail::func_type_cast<Binder::slot_func_type, flat_func_type>(rhs.func_);
        caller_ = reinterpret_cast<void *>(rhs.caller_);
        return (*this);
    }
};

//typedef std::_Unforced unforced;

template <typename Func, typename ...Args>
static inline binder<Func, std::_Unforced, Args...> bind(Func && func, Args && ...args) {
    // Bind a callable object with an implicit return type.
    return (binder<Func, std::_Unforced, Args...>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...),
		std::forward<Func>(func), std::forward<Args>(args)...));
}

template <typename Func, typename Ret, typename ...Args>
static inline binder<Func, Ret, Args...> bind(Func && func, Args && ...args) {
    // Bind a callable object with an explicit return type.
    return (binder<Func, Ret, Args...>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...),
		std::forward<Func>(func), std::forward<Args>(args)...));
}

//
// Comparing std::functions for equality?
// See: http://stackoverflow.com/questions/20833453/comparing-stdfunctions-for-equality
//
// C++ trying to get function address from a std::function
// See: http://stackoverflow.com/questions/18039723/c-trying-to-get-function-address-from-a-stdfunction
//
template <typename Ret, typename ...Args>
static inline size_t getAddressOf(std::function<Ret(Args...)> func) JIMI_NOEXCEPT {
    typedef Ret(funcType)(Args...);
    printf("Ret = %s\n", typeid(Ret).name());
    printf("funcType = %s\n", typeid(funcType).name());
    funcType ** funcPointer = func.template target<funcType *>();
    if (funcPointer != nullptr)
        return reinterpret_cast<size_t>(*funcPointer);
    else
        return 0;
}

template <typename Ret, typename ...Args>
static bool function_equal(std::function<Ret(Args...)> const & func1, std::function<Ret(Args...)> const & func2) JIMI_NOEXCEPT {
    typedef Ret(FuncType)(Args...);
    size_t sizeArgs = sizeof...(Args);
    //Ret(* const * ptr1)(Args...) = func1.template target<Ret(*)(Args...)>();
    //Ret(* const * ptr2)(Args...) = func2.template target<Ret(*)(Args...)>();
    FuncType ** ptr1 = func1.template target<FuncType *>();
    FuncType ** ptr2 = func2.template target<FuncType *>();
    bool is_equal = ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
    return is_equal;
}

template <typename T, typename ...Args>
static bool function_equal(std::function<void(Args...)> const & func1, std::function<void(Args...)> const & func2) JIMI_NOEXCEPT {
    bool is_equal = false;
    static constexpr size_t sizeArgs = sizeof...(Args);
    //typedef void(JIMI_CDECL T::*MemberFnType)(T *, Args...);
    //Ret(* const T::* ptr1)(T *, Args...) = func1.target<MemberFnType *>();
    //Ret(* const T::* ptr2)(T *, Args...) = func2.target<MemberFnType *>();
    {
        typedef void(JIMI_CDECL T::*MemberFnType)(T *, Args...);
        printf("MemberFnType = %s\n", typeid(MemberFnType).name());
        MemberFnType const * ptr1 = func1.template target<MemberFnType>();
        MemberFnType const * ptr2 = func2.template target<MemberFnType>();
        is_equal = ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
    }
    if (is_equal)
        return is_equal;
    {
        std::array<void *, sizeArgs> arr = { 0 };
        //auto tuple_list = make_args_tuple(arr);
        typedef void(JIMI_CDECL T::*MemberFnType)(Args...);
        // decltype(template make_args_tuple<sizeArgs>())
        typedef std::_Binder<std::_Unforced, MemberFnType, T *, std::tuple<Args...>> BindType;
        printf("BindType = %s\n", typeid(BindType).name());
        BindType const * ptr1 = func1.template target<BindType>();
        BindType const * ptr2 = func2.template target<BindType>();
        is_equal = ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
    }
    return is_equal;
}

template <typename T, typename Ret, typename ...Args>
static bool function_equal(std::function<Ret(Args...)> const & func1, std::function<Ret(Args...)> const & func2) JIMI_NOEXCEPT {
    bool is_equal = false;
    static constexpr size_t sizeArgs = sizeof...(Args);
    {
        typedef Ret(JIMI_CDECL T::*MemberFnType)(T *, Args...);
        printf("MemberFnType = %s\n", typeid(MemberFnType).name());
        MemberFnType const * ptr1 = func1.template target<MemberFnType>();
        MemberFnType const * ptr2 = func2.template target<MemberFnType>();
        is_equal = ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
    }
    if (is_equal)
        return is_equal;
    {
        std::array<void *, sizeArgs> arr = { 0 };
        //auto tuple_list = template make_args_tuple(arr);
        typedef Ret(JIMI_CDECL T::*MemberFnType)(Args...);
        typedef std::_Binder<Ret, MemberFnType, T *, Args...> BindType;
        printf("BindType = %s\n", typeid(BindType).name());
        BindType const * ptr1 = func1.template target<BindType>();
        BindType const * ptr2 = func2.template target<BindType>();
        is_equal = ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
    }
    return is_equal;
}

} // namespace jimi

#endif // !QT_FUNCTIONAL_H
