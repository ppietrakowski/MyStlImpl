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
            m_Function(function)
        {
        }

        TStaticDelegate(const SelfClass* del) :
            m_Function(del->m_Function)
        {
        }

        RetValue Execute(Args&& ...args) const override
        {
            return m_Function(std::forward<Args>(args)...);
        }

        bool Equals(const IDelegateBase& other) override
        {
            const SelfClass* del = dynamic_cast<const SelfClass*>(&other);
            return del && del->m_Function == m_Function;
        }

    private:
        StaticFunctionType m_Function;
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
            m_Object(&object),
            m_Function(function)
        {
        }

        TMemberDelegate(const SelfClass* del) :
            m_Object(del->m_Object),
            m_Function(del->m_Function)
        {
        }

        RetValue Execute(Args&& ...args) const override
        {
            return (m_Object->*m_Function)(std::forward<Args>(args)...);
        }

        bool Equals(const IDelegateBase& other) override
        {
            const SelfClass* del = dynamic_cast<const SelfClass*>(&other);
            return del && del->m_Function == m_Function && del->m_Object == m_Object;
        }

    private:
        ObjectType* m_Object;
        MemberFunctionType m_Function;
    };

    template <typename LambdaType, typename RetValue, typename ...Args>
    class TLambdaDelegate : public TDelegateBase<RetValue, Args...>
    {
    public:
        using DelegateBaseClass = TDelegateBase<RetValue, Args...>;
        using Super = TDelegateBase<RetValue, Args...>;
        using SelfClass = TLambdaDelegate<LambdaType, RetValue, Args...>;

        TLambdaDelegate(LambdaType&& function) :
            m_Lambda(std::make_shared<LambdaType>(std::forward<LambdaType>(function)))
        {
        }

        TLambdaDelegate(const SelfClass* del) :
            m_Lambda(del->m_Lambda)
        {
        }

        RetValue Execute(Args&& ...args) const override
        {
            return (*m_Lambda)(std::forward<Args>(args)...);
        }

    private:
        std::shared_ptr<LambdaType> m_Lambda;
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
            m_Object(object),
            m_Function(function)
        {
        }

        TSPMemberDelegate(const SelfClass* del) :
            m_Object(del->m_Object),
            m_Function(del->m_Function)
        {
        }

        RetValue Execute(Args&& ...args) const override
        {
            auto p = m_Object.lock();

            if (p)
            {
                ObjectType* ptr = p.get();
                return (ptr->*m_Function)(std::forward<Args>(args)...);
            }

            assert(p);
            return RetValue{};
        }

        bool Equals(const IDelegateBase& other) override
        {
            const SelfClass* del = dynamic_cast<const SelfClass*>(&other);
            auto p = m_Object.lock();

            if (del && p)
            {
                ObjectType* ptr = p.get();
                ObjectType* ptrOther = del->m_Object.lock().get();

                return del->m_Function == m_Function && (ptr == ptrOther);
            }

            return false;
        }

    private:
        std::weak_ptr<ObjectType> m_Object;
        MemberFunctionType m_Function;
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
            m_DestroyFunc(nullptr),
            m_CloneFunc{nullptr}
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
            m_AllocationSize = sizeof(T);

            m_DestroyFunc = [](IDelegateBase* memory)
            {
                T* p = static_cast<T*>(memory);
                p->~T();
            };

            m_CloneFunc = [](IDelegateBase* destination, const IDelegateBase* src)
            {
                const T* p = static_cast<const T*>(src);
                new (destination) T(p);
            };

            return GetDelegatePtr<T>();
        }

        void Reset()
        {
            ResetNoClear();
            memset(&m_Pad[0], 0, sizeof(m_Pad));
            m_AllocationSize = 0;
            m_DestroyFunc = nullptr;
            m_CloneFunc = nullptr;
        }

        int32_t GetAllocationSize() const
        {
            return m_AllocationSize;
        }

        void Clone(DelegateStorage& storage) const
        {
            assert(m_CloneFunc);
            storage.Reset();

            m_CloneFunc((IDelegateBase*)&storage.m_Pad[0], (const IDelegateBase*)&m_Pad[0]);
            storage.m_CloneFunc = m_CloneFunc;
            storage.m_AllocationSize = m_AllocationSize;
            storage.m_DestroyFunc = m_DestroyFunc;
        }

        template <typename T>
        T* GetDelegatePtr() const
        {
            return const_cast<T*>(reinterpret_cast<const T*>(&m_Pad[0]));
        }

    private:
        uint8_t m_Pad[MaxNumBytesAllocated];

        int32_t m_AllocationSize{0};
        void (*m_DestroyFunc)(IDelegateBase* memory);
        void (*m_CloneFunc)(IDelegateBase* destination, const IDelegateBase* src);

    private:
        void ResetNoClear()
        {
            if (m_DestroyFunc)
            {
                m_DestroyFunc((IDelegateBase*)&m_Pad[0]);
            }
        }
    };
}