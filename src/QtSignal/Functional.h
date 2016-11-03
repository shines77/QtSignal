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

namespace std {

#if !defined(_MSC_VER)

#define _NOEXCEPT noexcept
#ifndef cdecl
#define cdecl
#endif

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
#endif

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

    ///////////////////////////////////////////////////////////////////////////

    // result type traits
    template <typename Func>
    struct result_traits : result_traits<decltype(&Func::operator())> {};

    template <typename T>
    struct result_traits<T *> : result_traits<T> {};

    /* check function */
    template <typename Ret, typename ...Args>
    struct result_traits<Ret(*)(Args...)> { typedef Ret type; };

    /* check member function */
    #ifndef RESULT_TRAITS__
    #define RESULT_TRAITS__(...) \
        template <typename Ret, typename Caller, typename ...Args> \
        struct result_traits<Ret(Caller::*)(Args...) __VA_ARGS__> { typedef Ret type; };
    #endif

    RESULT_TRAITS__()
    RESULT_TRAITS__(const)
    RESULT_TRAITS__(volatile)
    RESULT_TRAITS__(const volatile)

    #undef RESULT_TRAITS__

    ///////////////////////////////////////////////////////////////////////////

    // member function type filter traits
    template <typename Func>
    struct func_traits : func_traits<decltype(&Func::operator())> {};

    template <typename T>
    struct func_traits<T *> : func_traits<T> {};

    /* check function */
    template <typename Ret, typename ...Args>
    struct func_traits<Ret(*)(Args...)> {
        typedef Ret(cdecl * func_type)(Args...);
        typedef std::function<Ret(Args...)> type;
        typedef std::nullptr_t callable_type;
    };

    /* check member function */
    #ifndef FUNCTION_TRAITS__
    #define FUNCTION_TRAITS__(...) \
        template <typename Ret, typename Caller, typename ...Args> \
        struct func_traits<Ret(Caller::*)(Args...) __VA_ARGS__> { \
            typedef Ret(cdecl * func_type)(Caller *, Args...); \
            typedef std::function<Ret(Args...)> type; \
            typedef Caller callable_type; \
        };
    #endif

    FUNCTION_TRAITS__()
    FUNCTION_TRAITS__(const)
    FUNCTION_TRAITS__(volatile)
    FUNCTION_TRAITS__(const volatile)

    #undef FUNCTION_TRAITS__

    ///////////////////////////////////////////////////////////////////////////

    struct noargs_t {
        // Tag for no argument.
    };

    struct noargs_tuple_t {
        // Tag for no argument.
    };

    template <typename Func, typename Ret>
    struct compressed_pair_no_args {
        typedef typename std::decay<Func>::type                     callable_type;
        typedef typename detail::func_traits<callable_type>::callable_type
                                                                    caller_type;
        typedef typename detail::noargs_tuple_t                     args_type;
        typedef typename detail::noargs_t                           first_type;
        typedef typename detail::noargs_tuple_t                     second_type;
        typedef std::function<Ret()>                                slot_type;
    };

    template <typename Func, typename Ret, typename T, typename ...Args>
    struct compressed_pair_one_more_args {
        typedef typename std::decay<T>::type                        first_type;
        typedef std::function<Ret(first_type *, Args...)>           slot_type;
    };

#if 0
    template <typename Func, typename Ret, typename ...Args>
    struct compressed_pair;
#else
    template <typename Func, typename Ret, typename ...Args>
    struct compressed_pair {
        typedef typename detail::noargs_t                           first_type;
        typedef std::function<Ret(Args...)>                         slot_type;
    };
#endif

    template <typename Func, typename Ret>
    struct compressed_pair<Func, Ret> : compressed_pair_no_args<Func, Ret> {
    };

    ///*
    template <typename Func, typename Ret, typename T, typename ...Args>
    struct compressed_pair<Func, Ret, T, Args...> :
        compressed_pair_one_more_args<Func, Ret, T, Args...> {
        typedef typename std::decay<Func>::type                     callable_type;
        typedef typename detail::func_traits<callable_type>::callable_type
                                                                    caller_type;
        typedef std::tuple<typename std::decay<Args>::type...>      args_type;
        typedef typename std::decay<T>::type                        first_type;
        typedef std::tuple<typename std::decay<Args>::type...>      second_type;
        typedef std::function<Ret(first_type, Args...)>             slot_type;
    };
    //*/

    ///////////////////////////////////////////////////////////////////////////

} // namespace detail

template <typename Func, typename Ret, typename ...Args>
class binder {
public:
    typedef std::tuple<typename std::decay<Args>::type...>              args_type;
    typedef typename std::decay<Func>::type                             callable_type;
    typedef typename detail::func_traits<callable_type>::callable_type  caller_type;
    typedef typename detail::result_traits<callable_type>::type         result_type;

    /// Type that will be used to store the slots for this signal type.
    typedef typename detail::compressed_pair<Func, Ret, Args...>::slot_type
                                                                        slot_type;
    typedef Ret(cdecl caller_type::*slot_func_type)(Args...);

private:
    callable_type func_;
    args_type     args_;

    slot_type     slot_;

public:
    template <typename Std_Func>
    binder(Std_Func && std_func, Func && func, Args && ...args)
        : func_(std::forward<Func>(func)),
          args_(std::forward<Args>(args)...) /*,
          slot_(std::bind(std::forward<Func>(func), std::forward<Args>(args)...))*/ {
        detail::func_traits<Func>::callable_type * pThis = std::get<0>(args_);
        //detail::func_traits<Func>::type func_t = (*reinterpret_cast<detail::func_traits<Func>::type *>(&func));
        detail::func_traits<Func>::func_type * pFunc = reinterpret_cast<detail::func_traits<Func>::func_type *>(&func);
        //pFunc = offsetof(detail::func_traits<Func>::caller_type, func);
        
        auto slot = std::bind(func_, std::forward<Args>(args)...);
        //slot_ = slot;
        //slot_ = std::bind(std::move(*pFunc), std::forward<Args>(args)...);
        slot_func_type * slot_func = reinterpret_cast<slot_func_type *>(&func_);
        detail::func_traits<Func>::func_type * pFunc2 = reinterpret_cast<detail::func_traits<Func>::func_type *>(&slot_func);
        //slot_ = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
        //slot_ = std::bind(*pFunc2, std::forward<Args>(args)...);
        auto slot_2 = std::bind(*pFunc2, std::forward<Args>(args)...);
        //slot_ = slot_2;
        //if (pFunc2 != nullptr) {
        //    (*pFunc2)(std::forward<Args>(args)...);
        //}
        //slot_ = std_func;
    }

    binder(binder const & src) {
        this->func_ = src.func_;
        this->args_ = src.args_;
        this->slot_ = src.slot_;
    }

    binder(binder && src) {
        this->func_ = std::move(src.func_);
        this->args_ = std::move(src.args_);
        this->slot_ = std::move(src.slot_);
    }

    binder & operator = (binder const & rhs) {
        this->func_ = rhs.func_;
        this->args_ = rhs.args_;
        this->slot_ = src.slot_;
        return (*this);
    }

    void init() {
    }

};

template <typename Func, typename Ret, typename ...Args>
class function: public binder<Func, Ret, Args...>
{
private:
    //
public:
    function(Func && func, Args && ...args) {}

};

//typedef std::_Unforced unforced;

template <typename Func, typename ...Args>
static inline binder<Func, std::_Unforced, Args...> bind(Func && func, Args && ...args) {
    // Bind a callable object with an implicit return type.
    std::function<void(int)> std_func = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
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
static inline size_t getAddressOf(std::function<Ret(Args...)> func) _NOEXCEPT {
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
static bool function_equal(std::function<Ret(Args...)> const & func1, std::function<Ret(Args...)> const & func2) _NOEXCEPT {
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
static bool function_equal(std::function<void(Args...)> const & func1, std::function<void(Args...)> const & func2) _NOEXCEPT {
    bool is_equal = false;
    static constexpr size_t sizeArgs = sizeof...(Args);
    //typedef void(cdecl T::*MemberFnType)(T *, Args...);
    //Ret(* const T::* ptr1)(T *, Args...) = func1.target<MemberFnType *>();
    //Ret(* const T::* ptr2)(T *, Args...) = func2.target<MemberFnType *>();
    {
        typedef void(cdecl T::*MemberFnType)(T *, Args...);
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
        typedef void(cdecl T::*MemberFnType)(Args...);
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
static bool function_equal(std::function<Ret(Args...)> const & func1, std::function<Ret(Args...)> const & func2) _NOEXCEPT {
    bool is_equal = false;
    static constexpr size_t sizeArgs = sizeof...(Args);
    {
        typedef Ret(cdecl T::*MemberFnType)(T *, Args...);
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
        typedef Ret(cdecl T::*MemberFnType)(Args...);
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
