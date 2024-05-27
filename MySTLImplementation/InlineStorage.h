#pragma once

#include <cstring>

template <typename ElementType>
struct TInlineStorage
{
    uint8_t Pad[sizeof(ElementType) == 0 ? 1 : sizeof(ElementType)];

    TInlineStorage() = default;
    TInlineStorage(const ElementType& element)
    {
        new (&Pad[0]) ElementType(element);
    }

    void Destroy()
    {
        ElementType* e = (ElementType*)Pad;
        e->~ElementType();
    }

    const ElementType* GetValue() const
    {
        return (ElementType*)Pad;
    }

    ElementType* GetValue()
    {
        return (ElementType*)Pad;
    }

    /* Remember to clear content first */
    template <typename ...Args>
    void Replace(Args&& ...args)
    {
        std::memset(Pad, 0, sizeof(Pad));
        new (&Pad[0]) ElementType(std::forward<Args>(args)...);
    }
};
