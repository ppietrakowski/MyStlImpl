#pragma once

#include <utility>

template <typename T>
struct TDefaultDeleter
{
    void operator()(T* p)
    {
        delete p;
    }
};

template <typename Pointer, typename EmptyType>
struct TCompressedPair : private EmptyType
{
    using Super = EmptyType;

    Pointer Ptr{};

    TCompressedPair() = default;
    TCompressedPair(TCompressedPair&& p) noexcept :
        Super{std::move(p)}
    {
    }

    void InvokeDeleter()
    {
        if (Ptr)
        {
            Super::operator()(Ptr);
            Ptr = nullptr;
        }
    }
};

template <typename PointerType, typename DeleterType = TDefaultDeleter<PointerType>>
class TUniquePtr
{
public:
    using SelfClass = TUniquePtr<PointerType, DeleterType>;

    TUniquePtr() = default;

    template <typename OtherDeleterType>
    TUniquePtr(const TUniquePtr<PointerType, OtherDeleterType>&) = delete;

    template <typename OtherDeleterType>
    TUniquePtr& operator=(const TUniquePtr<PointerType, OtherDeleterType>&) = delete;

    template <typename OtherPointerType, typename OtherDeleterType>
    TUniquePtr(TUniquePtr<OtherPointerType, OtherDeleterType>&& ptr) noexcept
    {
        PointerType* p = static_cast<PointerType*>(std::exchange(ptr.m_CompressedPair.Ptr, nullptr));

        m_CompressedPair = std::exchange(ptr.m_CompressedPair, TCompressedPair<PointerType*, DeleterType>{});
        ptr.m_CompressedPair.Ptr = nullptr;
        m_CompressedPair.Ptr = p;
    }

    template <typename OtherPointerType, typename OtherDeleterType>
    TUniquePtr& operator=(TUniquePtr<OtherPointerType, OtherDeleterType>&& ptr) noexcept
    {
        PointerType* p = static_cast<PointerType*>(std::exchange(ptr.m_CompressedPair.Ptr, nullptr));

        m_CompressedPair = std::exchange(ptr.m_CompressedPair, TCompressedPair<PointerType*, DeleterType>{});
        ptr.m_CompressedPair.Ptr = nullptr;
        m_CompressedPair.Ptr = p;

        return *this;
    }

    ~TUniquePtr() noexcept
    {
        m_CompressedPair.InvokeDeleter();
    }

    PointerType* Get() const
    {
        return m_CompressedPair.Ptr;
    }

    operator bool() const
    {
        return !!m_CompressedPair.Ptr;
    }

    bool operator!() const
    {
        return !m_CompressedPair.Ptr;
    }

    PointerType* Release()
    {
        PointerType* p = m_CompressedPair.Ptr;
        m_CompressedPair.Ptr = nullptr;
        return p;
    }

    void Reset(PointerType* p = nullptr)
    {
        m_CompressedPair.InvokeDeleter();
        m_CompressedPair.Ptr = p;
    }

    PointerType* operator->() const
    {
        return m_CompressedPair.Ptr;
    }

    PointerType& operator*() const
    {
        return m_CompressedPair.Ptr;
    }

private:
    TCompressedPair<PointerType*, DeleterType> m_CompressedPair;
};