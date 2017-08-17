#ifndef BASE_DELEGATE_HPP
#define BASE_DELEGATE_HPP

/**
 * @brief C++ delegate
 *
 * This object stores pointer to class and it's member.
 */
template <typename TReturn, typename... TParams> class Delegate
{
    typedef TReturn (*Type)(void* callee, TParams...);

  public:
    /**
     * @brief Default .ctor
     * @param[in] callee Pointer to object.
     * @param[in] function Method of object.
     */
    Delegate(void* callee, Type function) : callee(callee), function(function)
    {
    }

    /**
     * @brief Creates delegate.
     * @param[in] callee Pointer to object.
     * @tparam T Type of the object.
     * @return New delegate.
     */
    template <class T, TReturn (T::*TMethod)(TParams...)> static Delegate make_delegate(T* callee)
    {
        Delegate delegate(callee, &methodCaller<T, TMethod>);
        return delegate;
    }

    /**
     * @brief Executes delegate method on object.
     * @param[in] params Method parameters.
     * @return Method result.
     */
    TReturn operator()(TParams... params) const
    {
        return (*function)(callee, params...);
    }

  private:
    void* callee;

    Type function;

    template <class T, TReturn (T::*TMethod)(TParams...)> static TReturn methodCaller(void* callee, TParams... params)
    {
        T* object = static_cast<T*>(callee);
        return (object->*TMethod)(params...);
    }
};

#endif /* BASE_DELEGATE_HPP */
