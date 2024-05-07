#pragma once

#include "Delegate.h"
#include <vector>

template <typename>
class TMulticastDelegate;


template <typename TReturn, typename ...TArgs>
class TMulticastDelegate<TReturn(TArgs...)>
{
public:
    using DelegateType = TDelegate<TReturn(TArgs...)>;

    TMulticastDelegate();
    TMulticastDelegate(TMulticastDelegate&& delegate) noexcept;

    TMulticastDelegate& operator=(TMulticastDelegate&& delegate) noexcept;

    void Notify(TArgs&& ...args);

    void BindStaticFunction(TReturn(*fn)(TArgs ...args));
    void UnbindStaticFunction(TReturn(*fn)(TArgs ...args));

    template <typename TObject>
    void BindMemberFunction(TReturn(TObject::* fn)(TArgs...), TObject* target);

    template <typename TObject>
    void UnbindMemberFunction(TReturn(TObject::* fn)(TArgs...), TObject* target);

    template <typename TLambda>
    void BindLambdaFunction(TLambda&& Lambda);

    template <typename TLambda>
    void UnbindLambdaFunction(TLambda&& Lambda);

    void Each(TDelegate<bool(TDelegate<TReturn(TArgs...)>&)> fn);
    void EachRefDelegate(TDelegate<bool(TDelegate<TReturn(TArgs...)>&)>& fn);

    size_t GetNumOfDelegates() const;

private:
    std::vector<DelegateType> delegates;

    void RemoveDelegate(TDelegate<TReturn(TArgs...)>& del);
};

template <typename>
using Action = TMulticastDelegate<void()>;

template class TMulticastDelegate<void()>;

template <typename TReturn, typename ...TArgs>
TMulticastDelegate<TReturn(TArgs...)>::TMulticastDelegate()
{
    delegates.reserve(32);
}

template <typename TReturn, typename ...TArgs>
TMulticastDelegate<TReturn(TArgs...)>::TMulticastDelegate(TMulticastDelegate&& delegate) noexcept :
    delegates(std::move(delegate.delegates))
{
}

template <typename TReturn, typename ...TArgs>
TMulticastDelegate<TReturn(TArgs...)>& TMulticastDelegate<TReturn(TArgs...)>::operator=(TMulticastDelegate&& delegate) noexcept
{
    delegates = std::move(delegate.delegates);
    return *this;
}

template <typename TReturn, typename ...TArgs>
void TMulticastDelegate<TReturn(TArgs...)>::Notify(TArgs&& ...args)
{
    for (size_t i = 0; i < delegates.size(); i++)
    {
        delegates[i].Invoke(std::forward<Args>(args)...);
    }
}

template <typename TReturn, typename ...TArgs>
void TMulticastDelegate<TReturn(TArgs...)>::BindStaticFunction(TReturn(*fn)(TArgs ...args))
{
    TDelegate<TReturn(TArgs...)> del(fn);
    delegates.push_back(std::move(del));
}

template <typename TReturn, typename ...TArgs>
void TMulticastDelegate<TReturn(TArgs...)>::UnbindStaticFunction(TReturn(*fn)(TArgs ...args))
{
    TDelegate<TReturn(TArgs...)> del(fn);

    RemoveDelegate(del);
}

template <typename TReturn, typename ...TArgs>
template <typename TObject>
void TMulticastDelegate<TReturn(TArgs...)>::BindMemberFunction(TReturn(TObject::* fn)(TArgs...), TObject* target)
{
    TDelegate<TReturn(TArgs...)> del(fn, target);
    delegates.push_back(std::move(del));
}

template <typename TReturn, typename ...TArgs>
template <typename TObject>
void TMulticastDelegate<TReturn(TArgs...)>::UnbindMemberFunction(TReturn(TObject::* fn)(TArgs...), TObject* target)
{
    TDelegate<TReturn(TArgs...)> del(fn, target);
    RemoveDelegate(del);
}

template<typename TReturn, typename ...TArgs>
template<typename TLambda>
inline void TMulticastDelegate<TReturn(TArgs...)>::BindLambdaFunction(TLambda&& Lambda)
{
    TDelegate<TReturn(TArgs...)> del(Lambda);
    delegates.push_back(std::move(del));
}

template<typename TReturn, typename ...TArgs>
template<typename TLambda>
inline void TMulticastDelegate<TReturn(TArgs...)>::UnbindLambdaFunction(TLambda&& Lambda)
{
    TDelegate<TReturn(TArgs...)> del = Lambda;
    RemoveDelegate(del);
}

template<typename TReturn, typename ...TArgs>
inline void TMulticastDelegate<TReturn(TArgs...)>::Each(TDelegate<bool(TDelegate<TReturn(TArgs...)>&)> fn)
{
    for (DelegateType& delegate : delegates)
    {
        if (!fn(delegate))
        {
            return;
        }
    }
}

template<typename TReturn, typename ...TArgs>
inline void TMulticastDelegate<TReturn(TArgs...)>::EachRefDelegate(TDelegate<bool(TDelegate<TReturn(TArgs...)>&)>& fn)
{
    for (DelegateType& delegate : delegates)
    {
        if (!fn(delegate))
        {
            return;
        }
    }
}

template<typename TReturn, typename ...TArgs>
inline size_t TMulticastDelegate<TReturn(TArgs...)>::GetNumOfDelegates() const
{
    return delegates.size();
}

template<typename TReturn, typename ...TArgs>
inline void TMulticastDelegate<TReturn(TArgs...)>::RemoveDelegate(TDelegate<TReturn(TArgs...)>& del)
{
    auto it = std::remove(delegates.begin(), delegates.end(), del);

    if (it != delegates.end())
    {
        delegates.erase(it);
    }
}