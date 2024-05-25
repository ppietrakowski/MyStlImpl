#pragma once

#include "DelegateImpl.h"

template <typename>
class TDelegate;

template <typename RetValue, typename ...Args>
class TDelegate<RetValue(Args...)>
{
public:
    using DelegateBaseClass = impl::TDelegateBase<RetValue, Args...>;
    using SelfClass = TDelegate<RetValue(Args...)>;
    using StaticFunctionType = RetValue(*)(Args ...);

    TDelegate() = default;
    TDelegate(StaticFunctionType function) :
        delegate(storage.NewDelegate<impl::TStaticDelegate<RetValue, Args...>>(function))
    {
    }

    template <typename ObjectType>
    TDelegate(RetValue(ObjectType::* function)(Args... args), ObjectType& object) :
        delegate(storage.NewDelegate<impl::TMemberDelegate<ObjectType, RetValue, Args...>>(function, object))
    {
    }

    template <typename ObjectType>
    TDelegate(RetValue(ObjectType::* function)(Args... args), std::shared_ptr<ObjectType> object) :
        delegate(storage.NewDelegate<impl::TSPMemberDelegate<ObjectType, RetValue, Args...>>(function, object))
    {
    }

    template <typename LambdaType>
    SelfClass& BindLambda(LambdaType&& lambda)
    {
        delegate = storage.NewDelegate<impl::TLambdaDelegate<LambdaType, RetValue, Args...>>(std::forward<LambdaType>(lambda));
        return *this;
    }

    TDelegate(const SelfClass& copy)
    {
        copy.CloneTo(storage);
        delegate = storage.GetDelegatePtr<DelegateBaseClass>();
    }

    TDelegate& operator=(const SelfClass& copy)
    {
        copy.CloneTo(storage);
        delegate = storage.GetDelegatePtr<DelegateBaseClass>();
        return *this;
    }

    ~TDelegate() noexcept = default;

    RetValue Execute(Args&& ...args) const
    {
        assert(delegate);
        return delegate->Execute(std::forward<Args>(args)...);
    }

    RetValue ExecuteIfBound(Args&& ...args) const
    {
        if constexpr (std::is_void_v<RetValue>)
        {
            if (delegate)
            {
                delegate->Execute(std::forward<Args>(args)...);
            }
        }
        else
        {
            if (delegate)
            {
                return delegate->Execute(std::forward<Args>(args)...);
            }

            return RetValue{};
        }
    }

    RetValue operator()(Args&& ...args) const
    {
        assert(delegate);
        return delegate->Execute(std::forward<Args>(args)...);
    }

    bool Equals(const SelfClass& other) const
    {
        assert(delegate && other.delegate);
        return other.delegate->Equals(*delegate);
    }

    bool operator==(const SelfClass& other) const
    {
        return other.Equals(*this);
    }

    bool operator!=(const SelfClass& other) const
    {
        return !other.Equals(*this);
    }

    bool IsBound() const
    {
        return delegate != nullptr;
    }

    operator bool() const
    {
        return IsBound();
    }

    bool operator!() const
    {
        return !IsBound();
    }

private:
    /* Storage typed erase (stack size equal to max of sizes of all kinds of delegate types) */
    impl::DelegateStorage storage;

    /* Cached delegate for faster access to  */
    DelegateBaseClass* delegate{nullptr};

private:
    void CloneTo(impl::DelegateStorage& storage) const
    {
        storage.Clone(storage);
    }
};
