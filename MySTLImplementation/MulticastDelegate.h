#pragma once
#include "Delegate.h"
#include "Array.h"

template <typename ...Args>
class TMulticastDelegate
{
public:
    using DelegateType = TDelegate<void(Args...)>;
    TMulticastDelegate() = default;

    TMulticastDelegate(const TMulticastDelegate&) = default;
    TMulticastDelegate& operator=(const TMulticastDelegate&) = default;

    TMulticastDelegate(TMulticastDelegate&&) noexcept = default;
    TMulticastDelegate& operator=(TMulticastDelegate&&) noexcept = default;

    void AddStatic(void(*function)(Args... args))
    {
        m_Delegates.EmplaceBack(function);
    }

    void RemoveStatic(void(*function)(Args... args))
    {
        DelegateType del(function);
        DeleteDelegate(del);
    }

    template <typename ObjectType>
    void AddObjectRaw(void(ObjectType::* function)(Args... args), ObjectType& object)
    {
        m_Delegates.EmplaceBack(function, object);
    }

    template <typename ObjectType>
    void RemoveObjectRaw(void(ObjectType::* function)(Args... args), ObjectType& object)
    {
        DelegateType del(function, object);
        DeleteDelegate(del);
    }

    template <typename ObjectType>
    void AddObjectSP(void(ObjectType::* function)(Args... args), std::shared_ptr<ObjectType> object)
    {
        m_Delegates.EmplaceBack(function, object);
    }

    template <typename ObjectType>
    void RemoveObjectSP(void(ObjectType::* function)(Args... args), std::shared_ptr<ObjectType> object)
    {
        DelegateType del(function, object);
        DeleteDelegate(del);
    }

    template <typename LambdaType>
    void AddLambda(LambdaType&& lambda)
    {
        m_Delegates.Add(DelegateType().BindLambda(std::forward<LambdaType>(lambda)));
    }

    template <typename LambdaType>
    void RemoveLambda(LambdaType&& lambda)
    {
        DelegateType del(std::forward<LambdaType>(lambda));
        DeleteDelegate(del);
    }

    void Broadcast(Args&& ...args)
    {
        for (auto& del : m_Delegates)
        {
            del.Execute(std::forward<Args>(args)...);
        }
    }

    int32_t GetNumDelegates() const
    {
        return m_Delegates.GetNumElements();
    }

private:
    TArray<DelegateType> m_Delegates;

private:
    void DeleteDelegate(const DelegateType& del)
    {
        m_Delegates.Remove(del);
    }
};