#pragma once

#include <any>
#include <memory>

#include <array>
#include <cassert>

template <typename RetVal, typename ...Args>
class IDelegateBase
{
public:
    virtual ~IDelegateBase() = default;

public:
    virtual RetVal Invoke(Args&& ...args) const = 0;
    virtual bool Equal(const IDelegateBase<RetVal, Args...>& delegate) const = 0;
};

template <typename RetVal, typename ...Args>
class StaticDelegate : public IDelegateBase<RetVal, Args...>
{
public:
    using SelfClass = StaticDelegate<RetVal, Args...>;

    StaticDelegate(RetVal(*function)(Args&& ...args));
    StaticDelegate(const SelfClass& delegate) noexcept;
    StaticDelegate<RetVal, Args...>& operator=(const SelfClass& delegate) noexcept;

public:
    virtual RetVal Invoke(Args&& ...args) const override;
    virtual bool Equal(const IDelegateBase<RetVal, Args...>& delegate) const;

private:
    RetVal(*function)(Args&&... args);
};

template <typename RetVal, typename ...Args>
class MemberDelegate : public IDelegateBase<RetVal, Args...>
{
public:
    typedef MemberDelegate<RetVal, Args...> SelfClass;

    template <typename Object>
    MemberDelegate(RetVal(Object::* function)(Args&& ...args), Object* target);

    MemberDelegate(const SelfClass& delegate) noexcept;
    MemberDelegate<RetVal, Args...>& operator=(const SelfClass& delegate) noexcept;

public:
    virtual RetVal Invoke(Args&& ...args) const override;
    virtual bool Equal(const IDelegateBase<RetVal, Args...>& delegate) const;

private:
    void* function;
    void* target;

    RetVal(*call)(void* fnObject, void* target, Args&& ...args);
    bool(*equalTest)(const void* afunction, const void* atarget, const void* bfunction, const void* btarget);
};

template <typename RetVal, typename ...Args>
class LambdaDelegate : public IDelegateBase<RetVal, Args...>
{
public:

    typedef LambdaDelegate<RetVal, Args...> SelfClass;

    template <typename Fn>
    LambdaDelegate(Fn&& lambda);
    LambdaDelegate(SelfClass&& delegate) noexcept;
    LambdaDelegate<RetVal, Args...>& operator=(SelfClass&& delegate) noexcept;

    virtual RetVal Invoke(Args&& ...args) const override;
    virtual bool Equal(const IDelegateBase<RetVal, Args...>& delegate) const;

private:
    std::any function;
    RetVal(*call)(std::any& fnObject, Args&& ...args);
    bool(*equalTest)(const std::any& a, const std::any& b);
};

template <typename BaseType>
class DelegateStorage
{
public:
    DelegateStorage() = default;
    DelegateStorage(const DelegateStorage& delegate) = default;
    DelegateStorage& operator=(const DelegateStorage& delegate) = default;

public:
    template <typename DelegateType, typename ...Args>
    void Reset(Args&& ...args);

    template <typename LambdaDelegateType, typename Lambda>
    void ResetLambda(Lambda&& lambda)
    {
        using WrapperType = std::shared_ptr<LambdaDelegateType>;

        // wrap lambda with copy constructible object
        anyStorage = std::make_shared<LambdaDelegateType>(std::forward<Lambda>(lambda));
        getter = [](std::any& storage) -> BaseType&
        {
            return *std::any_cast<WrapperType&>(storage);
        };
    }

    void Clear();

    BaseType* Get()
    {
        return &getter(const_cast<std::any&>(anyStorage));
    }

    const BaseType* Get() const
    {
        return &getter(const_cast<std::any&>(anyStorage));
    }

    BaseType* operator->()
    {
        return Get();
    }
    const BaseType* operator->() const
    {
        return Get();
    }

    bool IsValid() const
    {
        return anyStorage.has_value();
    }

private:
    std::any anyStorage;
    BaseType& (*getter)(std::any& storage);
};

template <typename>
class TDelegate;

template <typename RetVal, typename ...Args>
class TDelegate<RetVal(Args ...)>
{
public:
    using StaticFunction = RetVal(*)(Args&& ...args);

    TDelegate() = default;
    TDelegate(const TDelegate<RetVal(Args&& ...args)>& delegate) :
        storage(delegate.storage)
    {
    }

    TDelegate<RetVal(Args&& ...args)>& operator=(const TDelegate<RetVal(Args&& ...args)>& delegate)
    {
        storage = delegate.storage;
        return *this;
    }

    TDelegate(StaticFunction function)
    {
        using ThisStaticDelegate = StaticDelegate<RetVal, Args...>;
        storage.Reset<ThisStaticDelegate>(function);
    }

    template <typename Object>
    TDelegate(RetVal(Object::* function)(Args&& ...args), Object* target)
    {
        using ThisMemberDelegate = MemberDelegate<RetVal, Args...>;

        storage.Reset<ThisMemberDelegate>(function, target);
    }

    template <typename Lambda>
    TDelegate(Lambda&& lambda)
    {
        storage.ResetLambda<LambdaDelegate<RetVal, Args...>>(std::forward<Lambda>(lambda));
    }

    RetVal operator()(Args&& ...args) const
    {
        assert(IsValid());

        return storage->Invoke(std::forward<Args>(args)...);
    }

    RetVal Invoke(Args&& ...args) const
    {
        assert(IsValid());

        return storage->Invoke(std::forward<Args>(args)...);
    }

    void Clear()
    {
        storage.Clear();
    }

    bool IsValid() const
    {
        return storage.IsValid();
    }

    operator bool() const
    {
        return IsValid();
    }

    bool operator==(const TDelegate<RetVal(Args&& ...args)>& anyStorage) const
    {
        return storage->Equal(*anyStorage.storage.Get());
    }

private:
    using DelegateBase = IDelegateBase<RetVal, Args...>;
    DelegateStorage<DelegateBase> storage;
};

template<typename RetVal, typename ...Args>
inline StaticDelegate<RetVal, Args...>::StaticDelegate(RetVal(*function)(Args&& ...args)) :
    function(function)
{
}

template<typename RetVal, typename ...Args>
inline StaticDelegate<RetVal, Args...>::StaticDelegate(const SelfClass& delegate) noexcept :
    function(delegate.function)
{
}

template<typename RetVal, typename ...Args>
inline StaticDelegate<RetVal, Args...>& StaticDelegate<RetVal, Args...>::operator=(const SelfClass& delegate) noexcept
{
    function = delegate.function;
    return *this;
}

template<typename RetVal, typename ...Args>
inline RetVal StaticDelegate<RetVal, Args...>::Invoke(Args && ...args) const
{
    return function(std::forward<Args>(args)...);
}

template<typename RetVal, typename ...Args>
inline bool StaticDelegate<RetVal, Args...>::Equal(const IDelegateBase<RetVal, Args...>& delegate) const
{
    const SelfClass* staticDelegate = dynamic_cast<const SelfClass*>(&delegate);
    return staticDelegate && staticDelegate->function == function;
}

#define PACK_MEMBER_FUNCTION_POINTER(function) (* reinterpret_cast<void**>(&(function)))

template<typename RetVal, typename ...Args>
template<typename Object>
inline MemberDelegate<RetVal, Args...>::MemberDelegate(RetVal(Object::* function)(Args &&...args), Object* target) :
    function(PACK_MEMBER_FUNCTION_POINTER(function)),
    target(target)
{
    using MemberFn = RetVal(Object::*)(Args &&...args);

    call = [](void* fnObject, void* target, Args&& ...args)
    {
#define UNPACK_MEMBER_FUNCTION_POINTER(fnObject) *reinterpret_cast<MemberFn*>(&fnObject)
        Object* object = reinterpret_cast<Object*>(target);
        MemberFn function = UNPACK_MEMBER_FUNCTION_POINTER(fnObject);

        return (object->*function)(std::forward<Args>(args)...);
#undef UNPACK_MEMBER_FUNCTION_POINTER
    };

    equalTest = [](const void* afunction, const void* atarget, const void* bfunction, const void* btarget)
    {
        return afunction == bfunction && atarget == btarget;
    };
}

#undef PACK_MEMBER_FUNCTION_POINTER

template<typename RetVal, typename ...Args>
inline MemberDelegate<RetVal, Args...>::MemberDelegate(const SelfClass& delegate) noexcept :
    function(delegate.function),
    call(delegate.call),
    equalTest(delegate.equalTest)
{
    delegate.call = nullptr;
    delegate.equalTest = nullptr;
}

template<typename RetVal, typename ...Args>
inline MemberDelegate<RetVal, Args...>& MemberDelegate<RetVal, Args...>::operator=(const SelfClass& delegate) noexcept
{
    function = delegate.function;
    call = delegate.call;
    equalTest = delegate.equalTest;
    delegate.call = nullptr;
    delegate.equalTest = nullptr;

    return *this;
}

template<typename RetVal, typename ...Args>
inline RetVal MemberDelegate<RetVal, Args...>::Invoke(Args && ...args) const
{
    return call(const_cast<void*>(function), target, std::forward<Args>(args)...);
}

template<typename RetVal, typename ...Args>
inline bool MemberDelegate<RetVal, Args...>::Equal(const IDelegateBase<RetVal, Args...>& delegate) const
{
    const SelfClass* memberDelegate = dynamic_cast<const SelfClass*>(&delegate);
    return memberDelegate && equalTest(function, target, memberDelegate->function, memberDelegate->target);
}

template<typename RetVal, typename ...Args>
template<typename Fn>
inline LambdaDelegate<RetVal, Args...>::LambdaDelegate(Fn&& lambda) :
    function(lambda)
{
    call = [](std::any& fnObject, Args&& ...args)
    {
        return std::any_cast<Fn&>(fnObject)(std::forward<Args>(args)...);
    };

    equalTest = [](const std::any& a, const std::any& b)
    {
        try
        {
            const auto& fnA = std::any_cast<Fn>(a);
            const auto& fnB = std::any_cast<Fn>(b);

            return fnA == fnB;
        }
        catch (...)
        {
            return false;
        }
    };
}

template<typename RetVal, typename ...Args>
inline LambdaDelegate<RetVal, Args...>::LambdaDelegate(SelfClass&& delegate) noexcept :
    function(std::move(delegate.function)),
    call(delegate.call),
    equalTest(delegate.equalTest)
{
    delegate.call = nullptr;
    delegate.equalTest = nullptr;
}

template<typename RetVal, typename ...Args>
inline LambdaDelegate<RetVal, Args...>& LambdaDelegate<RetVal, Args...>::operator=(SelfClass&& delegate) noexcept
{
    function = std::move(delegate.function);
    call = delegate.call;
    equalTest = delegate.equalTest;
    delegate.call = nullptr;
    delegate.equalTest = nullptr;

    return *this;
}

template<typename RetVal, typename ...Args>
inline RetVal LambdaDelegate<RetVal, Args...>::Invoke(Args && ...args) const
{
    return call(const_cast<std::any&>(function), std::forward<Args>(args)...);
}

template<typename RetVal, typename ...Args>
inline bool LambdaDelegate<RetVal, Args...>::Equal(const IDelegateBase<RetVal, Args...>& delegate) const
{
    const SelfClass* lambdaDelegate = dynamic_cast<const SelfClass*>(&delegate);
    return lambdaDelegate && equalTest(function, lambdaDelegate->function) && lambdaDelegate->equalTest(function, lambdaDelegate->function);
}

template<typename BaseType>
inline void DelegateStorage<BaseType>::Clear()
{
    anyStorage.reset();
    getter = nullptr;
}

template<typename BaseType>
template<typename DelegateType, typename ...Args>
inline void DelegateStorage<BaseType>::Reset(Args && ...args)
{
    anyStorage = DelegateType(std::forward<Args>(args)...);
    getter = [](std::any& storage) -> BaseType&
    {
        return std::any_cast<DelegateType&>(storage);
    };
}
