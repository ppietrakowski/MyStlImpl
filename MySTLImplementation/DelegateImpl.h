#pragma once

#include <cstdint>
#include <cmath>
#include <cassert>
#include <memory>

namespace impl
{
    class IDelegateBase
    {
    public:
        virtual ~IDelegateBase() noexcept = default;
        virtual bool Equals(const IDelegateBase& other) = 0;
    };

    template <typename RetValue, typename ...Args>
    class TDelegateBase : public IDelegateBase
    {
    public:
        using SelfClass = TDelegateBase<RetValue, Args...>;

        virtual ~TDelegateBase() noexcept = default;

        virtual RetValue Execute(Args&& ...args) const = 0;
        virtual bool Equals(const IDelegateBase& other) override
        {
            return false;
        }
    };

    template <typename RetValue, typename ...Args>
    class TStaticDelegate : public TDelegateBase<RetValue, Args...>
    {
    public:
        using StaticFunctionType = RetValue(*)(Args ...);
        using DelegateBaseClass = TDelegateBase<RetValue, Args...>;
        using SelfClass = TStaticDelegate<RetValue, Args...>;
        using Super = TDelegateBase<RetValue, Args...>;

        TStaticDelegate(StaticFunctionType function) :
            function(function)
        {
        }

        TStaticDelegate(const SelfClass* del) :
            function(del->function)
        {
        }

        RetValue Execute(Args&& ...args) const override
        {
            return function(std::forward<Args>(args)...);
        }

        bool Equals(const IDelegateBase& other) override
        {
            const SelfClass* del = dynamic_cast<const SelfClass*>(&other);
            return del && del->function == function;
        }

    private:
        StaticFunctionType function;
    };

    template <typename ObjectType, typename RetValue, typename ...Args>
    class TMemberDelegate : public TDelegateBase<RetValue, Args...>
    {
    public:
        using MemberFunctionType = RetValue(ObjectType::*)(Args ...);
        using DelegateBaseClass = TDelegateBase<RetValue, Args...>;
        using SelfClass = TMemberDelegate<ObjectType, RetValue, Args...>;
        using Super = TDelegateBase<RetValue, Args...>;

        TMemberDelegate(MemberFunctionType function, ObjectType& object) :
            object(&object),
            function(function)
        {
        }

        TMemberDelegate(const SelfClass* del) :
            object(del->object),
            function(del->function)
        {
        }

        RetValue Execute(Args&& ...args) const override
        {
            return (object->*function)(std::forward<Args>(args)...);
        }

        bool Equals(const IDelegateBase& other) override
        {
            const SelfClass* del = dynamic_cast<const SelfClass*>(&other);
            return del && del->function == function && del->object == object;
        }

    private:
        ObjectType* object;
        MemberFunctionType function;
    };

    template <typename LambdaType, typename RetValue, typename ...Args>
    class TLambdaDelegate : public TDelegateBase<RetValue, Args...>
    {
    public:
        using DelegateBaseClass = TDelegateBase<RetValue, Args...>;
        using Super = TDelegateBase<RetValue, Args...>;
        using SelfClass = TLambdaDelegate<LambdaType, RetValue, Args...>;

        TLambdaDelegate(LambdaType&& function) :
            lambda(std::make_shared<LambdaType>(std::forward<LambdaType>(function)))
        {
        }

        TLambdaDelegate(const SelfClass* del) :
            lambda(del->lambda)
        {
        }

        RetValue Execute(Args&& ...args) const override
        {
            return (*lambda)(std::forward<Args>(args)...);
        }

        bool Equals(const IDelegateBase& other) override
        {
            const SelfClass* del = dynamic_cast<const SelfClass*>(&other);
            return del && del->lambda.get() == lambda.get();
        }

    private:
        std::shared_ptr<LambdaType> lambda;
    };

    template <typename ObjectType, typename RetValue, typename ...Args>
    class TSPMemberDelegate : public TDelegateBase<RetValue, Args...>
    {
    public:
        using MemberFunctionType = RetValue(ObjectType::*)(Args ...);
        using DelegateBaseClass = TDelegateBase<RetValue, Args...>;
        using SelfClass = TSPMemberDelegate<ObjectType, RetValue, Args...>;
        using Super = TDelegateBase<RetValue, Args...>;

        TSPMemberDelegate(MemberFunctionType function, std::shared_ptr<ObjectType> object) :
            object(object),
            function(function)
        {
        }

        TSPMemberDelegate(const SelfClass* del) :
            object(del->object),
            function(del->function)
        {
        }

        RetValue Execute(Args&& ...args) const override
        {
            auto p = object.lock();

            if (p)
            {
                ObjectType* ptr = p.get();
                return (ptr->*function)(std::forward<Args>(args)...);
            }

            assert(p);
            return RetValue{};
        }

        bool Equals(const IDelegateBase& other) override
        {
            const SelfClass* del = dynamic_cast<const SelfClass*>(&other);
            auto p = object.lock();

            if (del && p)
            {
                ObjectType* ptr = p.get();
                ObjectType* ptrOther = del->object.lock().get();

                return del->function == function && (ptr == ptrOther);
            }

            return false;
        }

    private:
        std::weak_ptr<ObjectType> object;
        MemberFunctionType function;
    };

    inline void LambdaSizeCheck()
    {
    }

    using TestClass = TStaticDelegate<void>;

    constexpr int32_t MaxNumBytesAllocated = std::max<int32_t>({
        sizeof(TestClass),
        sizeof(TMemberDelegate<TestClass, void>),
        sizeof(TLambdaDelegate<decltype(LambdaSizeCheck), void>),
        sizeof(TSPMemberDelegate<TestClass, void>)
        });

    class DelegateStorage
    {
    public:
        DelegateStorage() :
            destroyFunc(nullptr),
            cloneFunc{nullptr}
        {
            Reset();
        }

        ~DelegateStorage() noexcept
        {
            ResetNoClear();
        }

        template <typename T, typename ...Args>
        T* NewDelegate(Args&& ...args)
        {
            T* p = NewDelegateRaw<T>();
            return new (p) T(std::forward<Args>(args)...);
        }

        template <typename T>
        T* NewDelegateRaw()
        {
            Reset();
            allocationSize = sizeof(T);

            destroyFunc = [](IDelegateBase* memory)
            {
                T* p = static_cast<T*>(memory);
                p->~T();
            };

            cloneFunc = [](IDelegateBase* destination, const IDelegateBase* src)
            {
                const T* p = static_cast<const T*>(src);
                new (destination) T(p);
            };

            return GetDelegatePtr<T>();
        }

        void Reset()
        {
            ResetNoClear();
            memset(&pad[0], 0, sizeof(pad));
            allocationSize = 0;
            destroyFunc = nullptr;
            cloneFunc = nullptr;
        }

        int32_t GetAllocationSize() const
        {
            return allocationSize;
        }

        void Clone(DelegateStorage& storage) const
        {
            assert(cloneFunc);
            storage.Reset();

            cloneFunc((IDelegateBase*)&storage.pad[0], (const IDelegateBase*)&pad[0]);
            storage.cloneFunc = cloneFunc;
            storage.allocationSize = allocationSize;
            storage.destroyFunc = destroyFunc;
        }

        template <typename T>
        T* GetDelegatePtr() const
        {
            return const_cast<T*>(reinterpret_cast<const T*>(&pad[0]));
        }

    private:
        uint8_t pad[MaxNumBytesAllocated];

        int32_t allocationSize{0};
        void (*destroyFunc)(IDelegateBase* memory);
        void (*cloneFunc)(IDelegateBase* destination, const IDelegateBase* src);

    private:
        void ResetNoClear()
        {
            if (destroyFunc)
            {
                destroyFunc((IDelegateBase*)&pad[0]);
            }
        }
    };
}