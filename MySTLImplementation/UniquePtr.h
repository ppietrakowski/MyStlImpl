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

    Pointer p{};

    TCompressedPair() = default;
    TCompressedPair(TCompressedPair&& p) noexcept :
        Super{std::move(p)}
    {
    }

    void InvokeDeleter()
    {
        if (p)
        {
            Super::operator()(p);
            p = nullptr;
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
        PointerType* p = static_cast<PointerType*>(std::exchange(ptr.compressedPair.p, nullptr);

        compressedPair = std::exchange(ptr.compressedPair, TCompressedPair<PointerType*, DeleterType>{});
        ptr.compressedPair.p = nullptr;
        compressedPair.p = p;
    }

    template <typename OtherPointerType, typename OtherDeleterType>
    TUniquePtr& operator=(TUniquePtr<OtherPointerType, OtherDeleterType>&& ptr) noexcept
    {
        PointerType* p = static_cast<PointerType*>(std::exchange(ptr.compressedPair.p, nullptr);

        compressedPair = std::exchange(ptr.compressedPair, TCompressedPair<PointerType*, DeleterType>{});
        ptr.compressedPair.p = nullptr;
        compressedPair.p = p;

        return *this;
    }

    ~TUniquePtr() noexcept
    {
        compressedPair.InvokeDeleter();
    }

    PointerType* Get() const
    {
        return compressedPair.p;
    }

    operator bool() const
    {
        return !!compressedPair.p;
    }

    bool operator!() const
    {
        return !compressedPair.p;
    }

    PointerType* Release()
    {
        PointerType* p = compressedPair.p;
        compressedPair.p = nullptr;
        return p;
    }

    void Reset(PointerType* p)
    {
        compressedPair.InvokeDeleter();
        compressedPair.p = p;
    }

    PointerType* operator->() const
    {
        return compressedPair.p;
    }

    PointerType& operator*() const
    {
        return compressedPair.p;
    }

private:
    TCompressedPair<PointerType*, DeleterType> compressedPair;
};