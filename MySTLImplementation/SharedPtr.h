#pragma once

#include <cstdint>
#include <atomic>

#define FORCE_MULTITHREAD_MODE 0

namespace impl
{
    template <typename PointerType>
    class TSingleThreadSafeRefCounter
    {
    public:
        TSingleThreadSafeRefCounter() :
            numWeakRefs{0},
            numSharedRefs{1}
        {
        }

        void Release()
        {
            int32_t numRefs = --this->numSharedRefs;

            if (numRefs == 0)
            {
                DestroyObject();

                if (numWeakRefs == 0)
                {
                    delete this;
                }
            }
        }

        void AddRef()
        {
            numSharedRefs++;
        }

        void ReleaseWeakRef()
        {
            int32_t numRefs = --numWeakRefs;

            if (numRefs == 0)
            {
                if (this->numSharedRefs == 0)
                {
                    delete this;
                }
            }
        }

        void AddWeakRef()
        {
            numWeakRefs++;
        }

        virtual PointerType* GetPointer() const = 0;

        int32_t GetUseCount() const
        {
            return numSharedRefs;
        }

    protected:
        ~TSingleThreadSafeRefCounter() noexcept = default;

        virtual void DestroyObject() = 0;

    private:
        int32_t numSharedRefs{0};
        int32_t numWeakRefs{0};
    };

    template <typename PointerType>
    class TMultiThreadSafeRefCounter
    {
    public:
        TMultiThreadSafeRefCounter() :
            numWeakRefs{0},
            numRefs{1}
        {
        }

        void Release()
        {
            int32_t numRefs = --numRefs;

            if (numRefs == 0)
            {
                DestroyObject();

                if (numWeakRefs == 0)
                {
                    delete this;
                }
            }
        }

        void AddRef()
        {
            numRefs++;
        }

        void ReleaseWeakRef()
        {
            --numWeakRefs;

            if (numWeakRefs == 0)
            {
                if (numRefs == 0)
                {
                    delete this;
                }
            }
        }

        void AddWeakRef()
        {
            numWeakRefs++;
        }

        virtual PointerType* GetPointer() const = 0;

        int32_t GetUseCount() const
        {
            return numRefs;
        }

    protected:
        ~TMultiThreadSafeRefCounter() noexcept = default;

        virtual void DestroyObject() = 0;

    private:
        std::atomic_int32_t numRefs{0};
        std::atomic_int32_t numWeakRefs{0};
    };

    template <typename PointerType>
    class TDefaultSpRefCounter : public impl::TSingleThreadSafeRefCounter<PointerType>
    {
    public:
        typedef impl::TSingleThreadSafeRefCounter<PointerType> Super;

        TDefaultSpRefCounter(PointerType* p) :
            pointer(p)
        {
        }

        virtual PointerType* GetPointer() const override
        {
            return pointer;
        }

    protected:
        ~TDefaultSpRefCounter() noexcept = default;

        virtual void DestroyObject() override
        {
            delete pointer;
            pointer = nullptr;
        }

    private:
        PointerType* pointer;
    };

    template <typename PointerType>
    class TInlineSpRefCounter : public impl::TSingleThreadSafeRefCounter<PointerType>
    {
    public:
        typedef impl::TSingleThreadSafeRefCounter<PointerType> Super;

        template <typename ...Args>
        TInlineSpRefCounter(Args&& ...args)
        {
            new (&pad[0]) PointerType(std::forward<Args>(args)...);
            isSet = true;
        }

        virtual PointerType* GetPointer() const override
        {
            return isSet ? (PointerType*)pad : NULL;
        }

    protected:
        ~TInlineSpRefCounter() noexcept = default;

        virtual void DestroyObject() override
        {
            if constexpr (!std::is_trivially_destructible_v<PointerType>)
            {
                PointerType* p = (PointerType*)pad;
                p->~PointerType();
            }

            isSet = false;
        }

    private:
        uint8_t pad[sizeof(PointerType)];
        bool isSet{false};
    };

    template <typename PointerType>
    class TDefaultMpRefCounter : public impl::TMultiThreadSafeRefCounter<PointerType>
    {
    public:
        typedef impl::TMultiThreadSafeRefCounter<PointerType> Super;

        TDefaultMpRefCounter(PointerType* p) :
            pointer(p)
        {
        }

        virtual PointerType* GetPointer() const override
        {
            return pointer;
        }

    protected:
        ~TDefaultMpRefCounter() noexcept = default;

        virtual void DestroyObject() override
        {
            delete pointer;
            pointer = nullptr;
        }

    private:
        PointerType* pointer;
    };

    template <typename PointerType>
    class TInlineMpRefCounter : public impl::TMultiThreadSafeRefCounter<PointerType>
    {
    public:
        typedef impl::TMultiThreadSafeRefCounter<PointerType> Super;

        template <typename ...Args>
        TInlineMpRefCounter(Args&& ...args)
        {
            new (&pad[0]) PointerType(std::forward<Args>(args)...);
            isSet = true;
        }

        virtual PointerType* GetPointer() const override
        {
            return isSet ? (PointerType*)pad : NULL;
        }

    protected:
        ~TInlineMpRefCounter() noexcept = default;

        virtual void DestroyObject() override
        {
            if constexpr (!std::is_trivially_destructible_v<PointerType>)
            {
                PointerType* p = (PointerType*)pad;
                p->~PointerType();
            }

            isSet = false;
        }

    private:
        uint8_t pad[sizeof(PointerType)];
        bool isSet{false};
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
    friend class TWeakPointer;

    template <typename OtherPointerType, EThreadMode OtherThreadMode>
    friend class TSharedPtr;

public:
    using RefCounter = std::conditional_t<ThreadMode == EThreadMode::SingleThread, impl::TSingleThreadSafeRefCounter<PointerType>, impl::TMultiThreadSafeRefCounter<PointerType>>;

    using DefaultRefCounter = std::conditional_t<ThreadMode == EThreadMode::SingleThread, impl::TDefaultSpRefCounter<PointerType>, impl::TDefaultMpRefCounter<PointerType>>;

    using SelfSharedPtr = TSharedPtr<PointerType, ThreadMode>;

    TSharedPtr() :
        refCounter{nullptr},
        pointer{nullptr}
    {
    }

    TSharedPtr(RefCounter* counter) :
        refCounter{counter},
        pointer{nullptr}
    {
        if (refCounter)
        {
            pointer = static_cast<PointerType*>(refCounter->GetPointer());
        }
    }

    explicit TSharedPtr(PointerType* p) :
        refCounter{new DefaultRefCounter(static_cast<PointerType*>(p))},
        pointer{static_cast<PointerType*>(p)}
    {
    }

    template <typename OtherPointerType>
    TSharedPtr(const TSharedPtr<OtherPointerType, ThreadMode>& p) :
        refCounter{(RefCounter*)p.refCounter},
        pointer{static_cast<PointerType*>(p.pointer)}
    {
        if (refCounter)
        {
            refCounter->AddRef();
        }
    }

    template <typename OtherPointerType>
    SelfSharedPtr& operator=(const TSharedPtr<OtherPointerType, ThreadMode>& p)
    {
        Clear();
        refCounter = (RefCounter*)p.refCounter;
        pointer = static_cast<PointerType*>(p.pointer);

        if (refCounter)
        {
            refCounter->AddRef();
        }

        return *this;
    }

    TSharedPtr(const SelfSharedPtr& p) :
        refCounter{p.refCounter},
        pointer{p.pointer}
    {
        if (refCounter)
        {
            refCounter->AddRef();
        }
    }

    SelfSharedPtr& operator=(const SelfSharedPtr& p)
    {
        Clear();
        refCounter = p.refCounter;
        pointer = p.pointer;

        if (refCounter)
        {
            refCounter->AddRef();
        }

        return *this;
    }

    ~TSharedPtr() noexcept
    {
        Clear();
    }

public:

    PointerType* Get() const
    {
        return pointer;
    }

    void Reset(PointerType* p)
    {
        TSharedPtr<PointerType, ThreadMode>{}.Swap(*this);

        if (p)
        {
            pointer = p;
            refCounter = new DefaultRefCounter(p);
        }
    }

    void swap(TSharedPtr<PointerType, ThreadMode>& p)
    {
        std::swap(pointer, p.pointer);
        std::swap(refCounter, p.refCounter);
    }

    PointerType* operator->() const
    {
        return pointer;
    }

    PointerType& operator*() const
    {
        return pointer;
    }

    int32_t GetUseCount() const
    {
        return refCounter->GetUseCount();
    }

private:
    RefCounter* refCounter{nullptr};
    PointerType* pointer{nullptr};

private:
    void Clear()
    {
        if (refCounter)
        {
            refCounter->Release();
        }

        refCounter = nullptr;
    }
};

template <typename PointerType, EThreadMode ThreadMode = EThreadMode::Auto>
class TWeakPointer
{
public:
    using SharedPtr = TSharedPtr<PointerType, ThreadMode>;
    using RefCounter = SharedPtr::RefCounter;

    TWeakPointer() = default;

    template <typename OtherPointerType>
    TWeakPointer(const TSharedPtr<OtherPointerType, ThreadMode>& p) :
        refCounter((RefCounter*)p.refCounter)
    {
        if (refCounter)
        {
            refCounter->AddWeakRef();
        }
    }

    TWeakPointer(const TWeakPointer<PointerType, ThreadMode>& p) :
        refCounter((RefCounter*)p.refCounter)
    {
        if (refCounter)
        {
            refCounter->AddWeakRef();
        }
    }

    TWeakPointer& operator=(const TWeakPointer<PointerType, ThreadMode>& p)
    {
        Clear();
        refCounter = p.refCounter;

        if (refCounter)
        {
            refCounter->AddWeakRef();
        }

        return *this;
    }

    template <typename OtherPointerType>
    TWeakPointer& operator=(const TSharedPtr<OtherPointerType, ThreadMode>& p)
    {
        static_assert(std::is_assignable_v<PointerType*, OtherPointerType*>);
        Clear();

        refCounter = (RefCounter*)p.refCounter;

        if (refCounter)
        {
            refCounter->AddWeakRef();
        }

        return *this;
    }

    template <typename OtherPointerType>
    TWeakPointer(const TWeakPointer<OtherPointerType, ThreadMode>& p)
    {
        static_assert(std::is_assignable_v<PointerType*, OtherPointerType*>);
        refCounter = (RefCounter*)p.refCounter;

        if (refCounter)
        {
            refCounter->AddWeakRef();
        }
    }

    ~TWeakPointer() noexcept
    {
        Clear();
    }

public:

    bool IsValid() const
    {
        return refCounter;
    }

    PointerType* Get() const
    {
        return refCounter->GetPointer();
    }

    PointerType* operator->() const
    {
        return refCounter->GetPointer();
    }

    PointerType& operator*() const
    {
        return *refCounter->GetPointer();
    }

    TSharedPtr<PointerType, ThreadMode> ToShared() const
    {
        if (refCounter)
        {
            refCounter->AddRef();
        }

        return TSharedPtr<PointerType, ThreadMode>{refCounter};
    }

private:
    RefCounter* refCounter{nullptr};
    
private:
    void Clear()
    {
        if (refCounter)
        {
            refCounter->ReleaseWeakRef();
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