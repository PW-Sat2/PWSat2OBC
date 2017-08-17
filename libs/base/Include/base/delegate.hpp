#ifndef BASE_DELEGATE_HPP
#define BASE_DELEGATE_HPP

/**
 * @brief C++ delegate
 *
 * This object stores pointer to class and it's member.
 */
template <typename TReturn, typename... TParams> class Delegate
{
  public:
    /**
     * @brief Delegate type.
     */
    typedef TReturn (*Type)(void*, TParams...);

    /**
     * @brief Default .ctor
     * @param[in] callee Pointer to object.
     * @param[in] function Method of object.
     */
    Delegate(void* callee, Type function);

    /**
     * @brief Creates delegate.
     * @param[in] callee Pointer to object.
     * @tparam T Type of the object.
     * @return New delegate.
     */
    template <class T, TReturn (T::*TMethod)(TParams...)> static Delegate<TReturn, TParams...> make_delegate(T* callee);

    /**
     * @brief Executes delegate method on object.
     * @param[in] params Method parameters.
     * @return Method result.
     */
    TReturn operator()(TParams... params) const;

  private:
    template <class T, TReturn (T::*TMethod)(TParams...)> static TReturn MethodCaller(void* callee, TParams... params);

    void* callee;

    Type function;
};

template <typename TReturn, typename... TParams>
Delegate<TReturn, TParams...>::Delegate(void* callee, Type function) : callee(callee), function(function)
{
}

template <typename TReturn, typename... TParams>
template <class T, TReturn (T::*TMethod)(TParams...)>
Delegate<TReturn, TParams...> Delegate<TReturn, TParams...>::make_delegate(T* callee)
{
    Delegate<TReturn, TParams...> delegate(callee, &MethodCaller<T, TMethod>);
    return delegate;
}

template <typename TReturn, typename... TParams> inline TReturn Delegate<TReturn, TParams...>::operator()(TParams... params) const
{
    return (*function)(callee, std::move(params...));
}

template <typename TReturn, typename... TParams>
template <class T, TReturn (T::*TMethod)(TParams...)>
TReturn Delegate<TReturn, TParams...>::MethodCaller(void* callee, TParams... params)
{
    T* object = static_cast<T*>(callee);
    return (object->*TMethod)(std::move(params...));
}

#endif /* BASE_DELEGATE_HPP */
