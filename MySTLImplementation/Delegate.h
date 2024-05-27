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
        m_Delegate(m_Storage.NewDelegate<impl::TStaticDelegate<RetValue, Args...>>(function))
    {
    }

    template <typename ObjectType>
    TDelegate(RetValue(ObjectType::* function)(Args... args), ObjectType& object) :
        m_Delegate(m_Storage.NewDelegate<impl::TMemberDelegate<ObjectType, RetValue, Args...>>(function, object))
    {
    }

    template <typename ObjectType>
    TDelegate(RetValue(ObjectType::* function)(Args... args), std::shared_ptr<ObjectType> object) :
        m_Delegate(m_Storage.NewDelegate<impl::TSPMemberDelegate<ObjectType, RetValue, Args...>>(function, object))
    {
    }

    template <typename LambdaType>
    SelfClass& BindLambda(LambdaType&& lambda)
    {
        m_Delegate = m_Storage.NewDelegate<impl::TLambdaDelegate<LambdaType, RetValue, Args...>>(std::forward<LambdaType>(lambda));
        return *this;
    }

    TDelegate(const SelfClass& copy)
    {
        copy.CloneTo(m_Storage);
        m_Delegate = m_Storage.GetDelegatePtr<DelegateBaseClass>();
    }

    TDelegate& operator=(const SelfClass& copy)
    {
        copy.CloneTo(m_Storage);
        m_Delegate = m_Storage.GetDelegatePtr<DelegateBaseClass>();
        return *this;
    }

    ~TDelegate() noexcept = default;

    RetValue Execute(Args&& ...args) const
    {
        assert(m_Delegate);
        return m_Delegate->Execute(std::forward<Args>(args)...);
    }

    RetValue ExecuteIfBound(Args&& ...args) const
    {
        if constexpr (std::is_void_v<RetValue>)
        {
            if (m_Delegate)
            {
                m_Delegate->Execute(std::forward<Args>(args)...);
            }
        }
        else
        {
            if (m_Delegate)
            {
                return m_Delegate->Execute(std::forward<Args>(args)...);
            }

            return RetValue{};
        }
    }

    RetValue operator()(Args&& ...args) const
    {
        assert(m_Delegate);
        return m_Delegate->Execute(std::forward<Args>(args)...);
    }

    bool Equals(const SelfClass& other) const
    {
        assert(m_Delegate && other.m_Delegate);
        return other.m_Delegate->Equals(*m_Delegate);
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
        return m_Delegate != nullptr;
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
    impl::DelegateStorage m_Storage;

    /* Cached delegate for faster access to  */
    DelegateBaseClass* m_Delegate{nullptr};

private:
    void CloneTo(impl::DelegateStorage& storage) const
    {
        storage.Clone(storage);
    }
};
