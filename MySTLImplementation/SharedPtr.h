#pragma once

#include <cstdint>
#include <atomic>
#include <cassert>
#include "InlineStorage.h"

#define FORCE_MULTITHREAD_MODE 0

namespace impl
{
    template <typename PointerType>
    class TSingleThreadSafeRefCounter
    {
    public:
        TSingleThreadSafeRefCounter() = default;

        void Release()
        {
            int32_t numRefs = --this->m_NumSharedRefs;

            if (numRefs == 0)
            {
                DestroyObject();

                if (m_NumWeakRefs == 0)
                {
                    delete this;
                }
            }
        }

        void AddRef()
        {
            m_NumSharedRefs++;
        }

        void ReleaseWeakRef()
        {
            int32_t numRefs = --m_NumWeakRefs;

            if (numRefs == 0)
            {
                if (this->m_NumSharedRefs == 0)
                {
                    delete this;
                }
            }
        }

        void AddWeakRef()
        {
            m_NumWeakRefs++;
        }

        virtual PointerType* GetPointer() const = 0;

        int32_t GetUseCount() const
        {
            return m_NumSharedRefs;
        }

    protected:
        ~TSingleThreadSafeRefCounter() noexcept = default;

        virtual void DestroyObject() = 0;

    private:
        int32_t m_NumSharedRefs{1};
        int32_t m_NumWeakRefs{0};
    };

    template <typename PointerType>
    class TMultiThreadSafeRefCounter
    {
    public:
        TMultiThreadSafeRefCounter() = default;

        void Release()
        {
            --m_NumRefs;

            if (m_NumRefs == 0)
            {
                DestroyObject();

                if (m_NumWeakRefs == 0)
                {
                    delete this;
                }
            }
        }

        void AddRef()
        {
            m_NumRefs++;
        }

        void ReleaseWeakRef()
        {
            --m_NumWeakRefs;

            if (m_NumWeakRefs == 0)
            {
                if (m_NumRefs == 0)
                {
                    delete this;
                }
            }
        }

        void AddWeakRef()
        {
            m_NumWeakRefs++;
        }

        virtual PointerType* GetPointer() const = 0;

        int32_t GetUseCount() const
        {
            return m_NumRefs;
        }

    protected:
        ~TMultiThreadSafeRefCounter() noexcept = default;

        virtual void DestroyObject() = 0;

    private:
        std::atomic_int32_t m_NumRefs{1};
        std::atomic_int32_t m_NumWeakRefs{0};
    };

    template <typename PointerType>
    class TDefaultSpRefCounter : public impl::TSingleThreadSafeRefCounter<PointerType>
    {
    public:
        typedef impl::TSingleThreadSafeRefCounter<PointerType> Super;

        TDefaultSpRefCounter(PointerType* p) :
            m_Pointer(p)
        {
        }

        virtual PointerType* GetPointer() const override
        {
            return m_Pointer;
        }

    protected:
        ~TDefaultSpRefCounter() noexcept = default;

        virtual void DestroyObject() override
        {
            delete m_Pointer;
            m_Pointer = nullptr;
        }

    private:
        PointerType* m_Pointer;
    };

    template <typename PointerType>
    class TInlineSpRefCounter : public impl::TSingleThreadSafeRefCounter<PointerType>
    {
    public:
        typedef impl::TSingleThreadSafeRefCounter<PointerType> Super;

        template <typename ...Args>
        TInlineSpRefCounter(Args&& ...args)
        {
            m_Pad.Replace(std::forward<Args>(args)...);
            m_bSet = true;
        }

        virtual PointerType* GetPointer() const override
        {
            return m_bSet ? const_cast<PointerType*>(m_Pad.GetValue()) : NULL;
        }

    protected:
        ~TInlineSpRefCounter() noexcept = default;

        virtual void DestroyObject() override
        {
            if constexpr (!std::is_trivially_destructible_v<PointerType>)
            {
                m_Pad.Destroy();
            }

            m_bSet = false;
        }

    private:
        TInlineStorage<PointerType> m_Pad;
        bool m_bSet{false};
    };

    template <typename PointerType>
    class TDefaultMpRefCounter : public impl::TMultiThreadSafeRefCounter<PointerType>
    {
    public:
        typedef impl::TMultiThreadSafeRefCounter<PointerType> Super;

        TDefaultMpRefCounter(PointerType* p) :
            m_Pointer(p)
        {
        }

        virtual PointerType* GetPointer() const override
        {
            return m_Pointer;
        }

    protected:
        ~TDefaultMpRefCounter() noexcept = default;

        virtual void DestroyObject() override
        {
            delete m_Pointer;
            m_Pointer = nullptr;
        }

    private:
        PointerType* m_Pointer;
    };

    template <typename PointerType>
    class TInlineMpRefCounter : public impl::TMultiThreadSafeRefCounter<PointerType>
    {
    public:
        typedef impl::TMultiThreadSafeRefCounter<PointerType> Super;

        template <typename ...Args>
        TInlineMpRefCounter(Args&& ...args)
        {
            m_Pad.Replace(std::forward<Args>(args)...);
            m_bSet = true;
        }

        virtual PointerType* GetPointer() const override
        {
            return m_bSet ? const_cast<PointerType*>(m_Pad.GetValue()) : NULL;
        }

    protected:
        ~TInlineMpRefCounter() noexcept = default;

        virtual void DestroyObject() override
        {
            assert(m_bSet);

            if constexpr (!std::is_trivially_destructible_v<PointerType>)
            {
                m_Pad.Destroy();
            }

            m_bSet = false;
        }

    private:
        TInlineStorage<PointerType> m_Pad;
        bool m_bSet{false};
    };
}

enum class EThreadMode
{
    SingleThread,
    MultiThread,
    Auto = FORCE_MULTITHREAD_MODE ? MultiThread : SingleThread
};

template <typename PointerType, EThreadMode ThreadMode = EThreadMode::Auto>
class TSharedPtr
{
    template <typename OtherPointerType, EThreadMode OtherThreadMode>
    friend class TWeakPtr;

    template <typename OtherPointerType, EThreadMode OtherThreadMode>
    friend class TSharedPtr;

    template <typename To, EThreadMode ThreadMode, typename From>
    friend TSharedPtr<To, ThreadMode> DynamicCast(const TSharedPtr<From, ThreadMode>& from);

    template <typename To, EThreadMode ThreadMode, typename From>
    friend TSharedPtr<To, ThreadMode> ReinterpretCast(const TSharedPtr<From, ThreadMode>& from);

public:
    using RefCounter = std::conditional_t<ThreadMode == EThreadMode::SingleThread, impl::TSingleThreadSafeRefCounter<PointerType>, impl::TMultiThreadSafeRefCounter<PointerType>>;

    using DefaultRefCounter = std::conditional_t<ThreadMode == EThreadMode::SingleThread, impl::TDefaultSpRefCounter<PointerType>, impl::TDefaultMpRefCounter<PointerType>>;

    using SelfSharedPtr = TSharedPtr<PointerType, ThreadMode>;

    TSharedPtr() :
        m_RefCounter{nullptr},
        m_Pointer{nullptr}
    {
    }

    TSharedPtr(RefCounter* counter) :
        m_RefCounter{counter},
        m_Pointer{nullptr}
    {
        if (m_RefCounter)
        {
            m_Pointer = static_cast<PointerType*>(m_RefCounter->GetPointer());
        }
    }

    explicit TSharedPtr(PointerType* p) :
        m_RefCounter{new DefaultRefCounter(static_cast<PointerType*>(p))},
        m_Pointer{static_cast<PointerType*>(p)}
    {
    }

    template <typename OtherPointerType>
    TSharedPtr(const TSharedPtr<OtherPointerType, ThreadMode>& p) :
        m_RefCounter{(RefCounter*)p.m_RefCounter},
        m_Pointer{static_cast<PointerType*>(p.m_Pointer)}
    {
        if (m_RefCounter)
        {
            m_RefCounter->AddRef();
        }
    }

    template <typename OtherPointerType>
    SelfSharedPtr& operator=(const TSharedPtr<OtherPointerType, ThreadMode>& p)
    {
        Clear();
        m_RefCounter = (RefCounter*)p.m_RefCounter;
        m_Pointer = static_cast<PointerType*>(p.m_Pointer);

        if (m_RefCounter)
        {
            m_RefCounter->AddRef();
        }

        return *this;
    }

    TSharedPtr(const SelfSharedPtr& p) :
        m_RefCounter{p.m_RefCounter},
        m_Pointer{p.m_Pointer}
    {
        if (m_RefCounter)
        {
            m_RefCounter->AddRef();
        }
    }

    SelfSharedPtr& operator=(const SelfSharedPtr& p)
    {
        Clear();
        m_RefCounter = p.m_RefCounter;
        m_Pointer = p.m_Pointer;

        if (m_RefCounter)
        {
            m_RefCounter->AddRef();
        }

        return *this;
    }

    TSharedPtr(SelfSharedPtr&& p) noexcept:
        m_RefCounter{nullptr},
        m_Pointer{nullptr}
    {
        *this = std::move(p);
    }

    SelfSharedPtr& operator=(SelfSharedPtr&& p) noexcept
    {
        Clear();
        m_RefCounter = std::exchange(p.m_RefCounter, nullptr);
        m_Pointer = std::exchange(p.m_Pointer, nullptr);
        return *this;
    }

    template <typename OtherPointerType>
    TSharedPtr(TSharedPtr<OtherPointerType, ThreadMode>&& p) noexcept :
        m_RefCounter{nullptr},
        m_Pointer{nullptr}
    {
        *this = std::move(p);
    }

    template <typename OtherPointerType>
    SelfSharedPtr& operator=(TSharedPtr<OtherPointerType, ThreadMode>&& p) noexcept
    {
        Clear();
        m_RefCounter = (RefCounter*)std::exchange(p.m_RefCounter, nullptr);
        m_Pointer = static_cast<PointerType*>(std::exchange(p.m_Pointer, nullptr));
        return *this;
    }

    ~TSharedPtr() noexcept
    {
        Clear();
    }

public:

    PointerType* Get() const
    {
        return m_Pointer;
    }

    void Reset(PointerType* p = nullptr)
    {
        TSharedPtr<PointerType, ThreadMode>{}.swap(*this);

        if (p)
        {
            m_Pointer = p;
            m_RefCounter = new DefaultRefCounter(p);
        }
    }

    void swap(TSharedPtr<PointerType, ThreadMode>& p)
    {
        std::swap(m_Pointer, p.m_Pointer);
        std::swap(m_RefCounter, p.m_RefCounter);
    }

    PointerType* operator->() const
    {
        return m_Pointer;
    }

    PointerType& operator*() const
    {
        return *m_Pointer;
    }

    int32_t GetUseCount() const
    {
        return m_RefCounter->GetUseCount();
    }

    bool IsValid() const
    {
        return m_RefCounter;
    }

    operator bool() const
    {
        return IsValid();
    }

    bool operator!() const
    {
        return !IsValid();
    }

private:
    RefCounter* m_RefCounter{nullptr};
    PointerType* m_Pointer{nullptr};

private:
    void Clear()
    {
        if (m_RefCounter)
        {
            m_RefCounter->Release();
        }

        m_RefCounter = nullptr;
    }
};

template <typename PointerType, EThreadMode ThreadMode = EThreadMode::Auto>
class TWeakPtr
{
    template <typename To, EThreadMode ThreadMode, typename From>
    friend TWeakPtr<To, ThreadMode> DynamicCast(const TWeakPtr<From, ThreadMode>& from);

    template <typename To, EThreadMode ThreadMode, typename From>
    friend TWeakPtr<To, ThreadMode> ReinterpretCast(const TWeakPtr<From, ThreadMode>& from);

public:
    using SharedPtr = TSharedPtr<PointerType, ThreadMode>;
    using RefCounter = SharedPtr::RefCounter;

    TWeakPtr() = default;

    template <typename OtherPointerType>
    TWeakPtr(const TSharedPtr<OtherPointerType, ThreadMode>& p) :
        m_RefCounter((RefCounter*)p.m_RefCounter)
    {
        static_assert(std::is_convertible_v<OtherPointerType*, PointerType*>);
        if (m_RefCounter)
        {
            m_RefCounter->AddWeakRef();
        }
    }

    TWeakPtr(const TSharedPtr<PointerType, ThreadMode>& p) :
        m_RefCounter((RefCounter*)p.m_RefCounter)
    {
        if (m_RefCounter)
        {
            m_RefCounter->AddWeakRef();
        }
    }

    TWeakPtr(const TWeakPtr<PointerType, ThreadMode>& p) :
        m_RefCounter((RefCounter*)p.m_RefCounter)
    {
        if (m_RefCounter)
        {
            m_RefCounter->AddWeakRef();
        }
    }

    TWeakPtr& operator=(const TWeakPtr<PointerType, ThreadMode>& p)
    {
        Clear();
        m_RefCounter = p.m_RefCounter;

        if (m_RefCounter)
        {
            m_RefCounter->AddWeakRef();
        }

        return *this;
    }

    template <typename OtherPointerType>
    TWeakPtr& operator=(const TSharedPtr<OtherPointerType, ThreadMode>& p)
    {
        static_assert(std::is_assignable_v<PointerType*, OtherPointerType*>);
        Clear();

        m_RefCounter = (RefCounter*)p.m_RefCounter;

        if (m_RefCounter)
        {
            m_RefCounter->AddWeakRef();
        }

        return *this;
    }

    TWeakPtr& operator=(const TSharedPtr<PointerType, ThreadMode>& p)
    {
        Clear();

        m_RefCounter = (RefCounter*)p.m_RefCounter;

        if (m_RefCounter)
        {
            m_RefCounter->AddWeakRef();
        }

        return *this;
    }

    template <typename OtherPointerType>
    TWeakPtr(const TWeakPtr<OtherPointerType, ThreadMode>& p)
    {
        static_assert(std::is_assignable_v<PointerType*, OtherPointerType*>);
        m_RefCounter = (RefCounter*)p.m_RefCounter;

        if (m_RefCounter)
        {
            m_RefCounter->AddWeakRef();
        }
    }

    ~TWeakPtr() noexcept
    {
        Clear();
    }

public:

    bool IsValid() const
    {
        return m_RefCounter;
    }

    PointerType* Get() const
    {
        return m_RefCounter->GetPointer();
    }

    PointerType* operator->() const
    {
        PointerType* pointer = m_RefCounter->GetPointer();
        assert(pointer != nullptr && "Retrieving dead object");
        return pointer;
    }

    PointerType& operator*() const
    {
        PointerType* pointer = m_RefCounter->GetPointer();
        assert(pointer != nullptr);
        return *pointer;
    }

    TSharedPtr<PointerType, ThreadMode> AsShared() const
    {
        if (m_RefCounter)
        {
            m_RefCounter->AddRef();
        }

        return TSharedPtr<PointerType, ThreadMode>{m_RefCounter};
    }

private:
    RefCounter* m_RefCounter{nullptr};

private:
    void Clear()
    {
        if (m_RefCounter)
        {
            m_RefCounter->ReleaseWeakRef();
        }
    }
};

template <typename PointerType, typename ...Args>
TSharedPtr<PointerType, EThreadMode::SingleThread> MakeSharedSp(Args&& ...args)
{
    static_assert(!std::is_abstract_v<PointerType>);

    impl::TInlineSpRefCounter<PointerType>* refCounter = new impl::TInlineSpRefCounter<PointerType>(std::forward<Args>(args)...);
    return TSharedPtr<PointerType, EThreadMode::SingleThread>{refCounter};
}

template <typename PointerType, typename ...Args>
TSharedPtr<PointerType, EThreadMode::MultiThread> MakeSharedMp(Args&& ...args)
{
    static_assert(!std::is_abstract_v<PointerType>);

    impl::TInlineMpRefCounter<PointerType>* refCounter = new impl::TInlineMpRefCounter<PointerType>(std::forward<Args>(args)...);
    return TSharedPtr<PointerType, EThreadMode::MultiThread>{refCounter};
}

template <typename PointerType, typename ...Args>
TSharedPtr<PointerType> MakeShared(Args&& ...args)
{
    if constexpr (EThreadMode::Auto == EThreadMode::SingleThread)
    {
        return MakeSharedSp<PointerType>(std::forward<Args>(args)...);
    }
    else
    {
        return MakeSharedMp<PointerType>(std::forward<Args>(args)...);
    }
}

template <typename To, EThreadMode ThreadMode, typename From>
TSharedPtr<To, ThreadMode> DynamicCast(const TSharedPtr<From, ThreadMode>& from)
{
    using RefCounter = TSharedPtr<To, ThreadMode>::RefCounter;

    To* p = dynamic_cast<To*>(from.Get());
    if (p)
    {
        from.m_RefCounter->AddRef();
        return TSharedPtr<To, ThreadMode>((RefCounter*)from.m_RefCounter);
    }

    return NULL;
}

template <typename To, EThreadMode ThreadMode, typename From>
TSharedPtr<To, ThreadMode> ReinterpretCast(const TSharedPtr<From, ThreadMode>& from)
{
    using RefCounter = TSharedPtr<To, ThreadMode>::RefCounter;
    from.m_RefCounter->AddRef();

    return TSharedPtr<To, ThreadMode>((RefCounter*)from.m_RefCounter);
}

template <typename To, EThreadMode ThreadMode, typename From>
TWeakPtr<To, ThreadMode> DynamicCast(const TWeakPtr<From, ThreadMode>& from)
{
    using RefCounter = TWeakPtr<To, ThreadMode>::RefCounter;

    To* p = dynamic_cast<To*>(from.Get());
    if (p)
    {
        from.m_RefCounter->AddWeakRef();
        return TWeakPtr<To, ThreadMode>((RefCounter*)from.m_RefCounter);
    }

    return NULL;
}

template <typename To, EThreadMode ThreadMode, typename From>
TWeakPtr<To, ThreadMode> ReinterpretCast(const TWeakPtr<From, ThreadMode>& from)
{
    using RefCounter = TWeakPtr<To, ThreadMode>::RefCounter;
    from.m_RefCounter->AddWeakRef();
    return TWeakPtr<To, ThreadMode>((RefCounter*)from.m_RefCounter);
}