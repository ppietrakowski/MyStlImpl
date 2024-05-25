#pragma once

#include <cstdint>
#include <atomic>

template <typename PointerType>
class SingleThreadSafeRefCounter
{
public:
    SingleThreadSafeRefCounter(PointerType* p) :
        numWeakRefs{0},
        numRefs{1},
        pointer{p}
    {
    }

    void Release()
    {
        int32_t numRefs = --this->numRefs;

        if (numRefs == 0)
        {
            if (numWeakRefs == 0)
            {
                delete this;
            }
            else
            {
                pointer = nullptr;
            }
        }
    }

    void AddRef()
    {
        numRefs++;
    }

    void ReleaseWeakRef()
    {
        int32_t numRefs = --numWeakRefs;

        if (numRefs == 0)
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

    PointerType* GetPointer() const
    {
        return pointer;
    }

    int32_t GetUseCount() const
    {
        return numRefs;
    }

protected:
    ~SingleThreadSafeRefCounter() noexcept
    {
        delete pointer;
    }

private:
    int32_t numRefs{0};
    int32_t numWeakRefs{0};
    PointerType* pointer;
};

template <typename PointerType>
class MultiThreadSafeRefCounter
{
public:
    MultiThreadSafeRefCounter(PointerType* p) :
        numWeakRefs{0},
        numRefs{1},
        pointer{p}
    {
    }

    void Release()
    {
        int32_t numRefs = --numRefs;

        if (numRefs == 0)
        {
            if (numWeakRefs == 0)
            {
                delete this;
            }
            else
            {
                pointer = nullptr;
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

    PointerType* GetPointer() const
    {
        return pointer;
    }

    int32_t GetUseCount() const
    {
        return numRefs;
    }

protected:
    ~MultiThreadSafeRefCounter() noexcept
    {
        delete pointer;
    }

private:
    std::atomic_int32_t numRefs{0};
    std::atomic_int32_t numWeakRefs{0};
    PointerType* pointer;
};

enum class EThreadMode
{
    SingleThread,
    MultiThread,
    Auto = SingleThread
};

template <typename PointerType, EThreadMode ThreadMode = EThreadMode::Auto>
class TSharedPtr
{
    template <typename OtherPointerType, EThreadMode OtherThreadMode>
    friend class TWeakPointer;

public:
    using RefCounter = std::conditional_t<ThreadMode == EThreadMode::SingleThread, SingleThreadSafeRefCounter<PointerType>, MultiThreadSafeRefCounter<PointerType>>;

    TSharedPtr() :
        refCounter{nullptr},
        pointer{nullptr}
    {
    }

    TSharedPtr(RefCounter *counter) :
        refCounter{counter},
        pointer{nullptr}
    {
        if (refCounter)
        {
            pointer = refCounter->GetPointer();
            refCounter->AddRef();
        }
    }

    TSharedPtr(PointerType* p) :
        refCounter{new RefCounter(p)},
        pointer{p}
    {
    }

    TSharedPtr(const TSharedPtr<PointerType, ThreadMode>& p) :
        refCounter{p.refCounter},
        pointer{p.pointer}
    {
        if (refCounter)
        {
            refCounter->AddRef();
        }
    }

    TSharedPtr& operator=(const TSharedPtr<PointerType, ThreadMode>& p)
    {
        refCounter = p.refCounter;
        pointer = p.pointer;

        if (refCounter)
        {
            refCounter->AddRef();
        }
    }

    ~TSharedPtr() noexcept
    {
        if (refCounter)
        {
            refCounter->Release();
        }
    }

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
            refCounter = new RefCounter(p);
        }
    }

    void Swap(TSharedPtr<PointerType, ThreadMode>& p)
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
        refCounter(p.refCounter)
    {
        if (refCounter)
        {
            refCounter->AddWeakRef();
        }
    }

    template <typename OtherPointerType>
    TWeakPointer(const TWeakPointer<OtherPointerType>& p)
    {
        static_assert(std::is_assignable_v<PointerType, OtherPointerType>);
        refCounter = (RefCounter*)p.refCounter;

        if (refCounter)
        {
            refCounter->AddWeakRef();
        }
    }

    ~TWeakPointer() noexcept
    {
        if (refCounter)
        {
            refCounter->ReleaseWeakRef();
        }
    }

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
        return TSharedPtr<PointerType, ThreadMode>{refCounter};
    }

private:
    RefCounter* refCounter{nullptr};
};