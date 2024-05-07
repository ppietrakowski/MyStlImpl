#pragma once

#include <cstdint>
#include <atomic>

template <typename PointerType>
class SingleThreadSafeRefCounter
{
public:
    SingleThreadSafeRefCounter(PointerType* p):
        m_NumWeakRefs{0},
        m_NumRefs{1},
        m_Pointer{p}
    {
    }

    void Release()
    {
        int32_t numRefs = --m_NumRefs;

        if (numRefs == 0)
        {
            if (m_NumWeakRefs == 0)
            {
                delete this;
            }
            else
            {
                m_Pointer = nullptr
            }
        }
    }

    void AddRef()
    {
        m_NumRefs++;
    }

    void ReleaseWeakRef()
    {
        int32_t numRefs = --m_NumWeakRefs;

        if (numRefs == 0)
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
    
    T* GetPointer() const
    {
        return m_Pointer;
    }

    int32_t GetUseCount() const
    {
        return m_NumRefs;
    }

protected:
    ~SingleThreadSafeRefCounter() noexcept
    {
        delete m_Pointer;
    }

private:
    int32_t m_NumRefs{0};
    int32_t m_NumWeakRefs{0};
    PointerType* m_Pointer;
};

template <typename PointerType>
class MultiThreadSafeRefCounter
{
public:
    MultiThreadSafeRefCounter(PointerType* p) :
        m_NumWeakRefs{0},
        m_NumRefs{1},
        m_Pointer{p}
    {
    }

    void Release()
    {
        int32_t numRefs = --m_NumRefs;

        if (numRefs == 0)
        {
            if (m_NumWeakRefs == 0)
            {
                delete this;
            }
            else
            {
                m_Pointer = nullptr
            }
        }
    }

    void AddRef()
    {
        m_NumRefs++;
    }

    void ReleaseWeakRef()
    {
        int32_t numRefs = --m_NumWeakRefs;

        if (numRefs == 0)
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

    T* GetPointer() const
    {
        return m_Pointer;
    }

    int32_t GetUseCount() const
    {
        return m_NumRefs;
    }

protected:
    ~MultiThreadSafeRefCounter() noexcept
    {
        delete m_Pointer;
    }

private:
    std::atomic_int32_t m_NumRefs{0};
    std::atomic_int32_t m_NumWeakRefs{0};
    PointerType* m_Pointer;
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
        m_RefCounter{nullptr},
        m_Pointer{nullptr}
    {
    }

    TSharedPtr(PointerType *p) :
        m_RefCounter{new RefCounter(p)},
        m_Pointer{p}
    {
    }

    TSharedPtr(const TSharedPtr<PointerType, ThreadMode> &p) :
        m_RefCounter{p.m_RefCounter},
        m_Pointer{p.m_Pointer}
    {
        if (m_RefCounter)
        {
            m_RefCounter->AddRef();
        }
    }

    TSharedPtr& operator=(const TSharedPtr<PointerType, ThreadMode>& p)
    {
        m_RefCounter = p.m_RefCounter;
        m_Pointer = p.m_Pointer;

        if (m_RefCounter)
        {
            m_RefCounter->AddRef();
        }
    }

    ~TSharedPtr() noexcept
    {
        if (m_RefCounter)
        {
            m_RefCounter->Release();
        }
    }

    PointerType* Get() const
    {
        return m_Pointer;
    }

    void Reset(PointerType* p)
    {
        TSharedPtr<PointerType, ThreadMode>{}.Swap(*this);

        if (p)
        {
            m_Pointer = p;
            m_RefCounter = new RefCounter(p);
        }
    }

    void Swap(TSharedPtr<PointerType, ThreadMode>& p)
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
        return m_Pointer;
    }

    int32_t GetUseCount() const
    {
        return m_RefCounter->GetUseCount();
    }

private:
    RefCounter* m_RefCounter{nullptr};
    PointerType* m_Pointer{nullptr};
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
        m_RefCounter(p.m_RefCounter)
    {
        if (m_RefCounter)
        {
            m_RefCounter->AddWeakRef();
        }
    }

    template <typename OtherPointerType>
    TWeakPointer(const TWeakPointer<OtherPointerType>& p)
    {
        static_assert(std::is_assignable_v<PointerType, OtherPointerType>);
        m_RefCounter = (RefCounter*)p.m_RefCounter;

        if (m_RefCounter)
        {
            m_RefCounter->AddWeakRef();
        }
    }

    ~TWeakPointer() noexcept
    {
        if (m_RefCounter)
        {
            m_RefCounter->ReleaseWeakRef();
        }
    }

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
        return m_RefCounter->GetPointer();
    }

    PointerType& operator*() const
    {
        return *m_RefCounter->GetPointer();
    }

private:
    RefCounter* m_RefCounter{nullptr};
};