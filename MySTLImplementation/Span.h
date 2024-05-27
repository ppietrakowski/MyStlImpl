#pragma once

#include <cstdint>

#include "Array.h"

template <typename ElementType>
class TSpanIterator
{
public:
    TSpanIterator(ElementType* element) :
        element(element)
    {
    }

    TSpanIterator(const TSpanIterator<ElementType>&) = default;

    ElementType& operator*()
    {
        return *element;
    }

    bool operator==(const TSpanIterator<ElementType>& it) const
    {
        return it.element == element;
    }

    bool operator!=(const TSpanIterator<ElementType>& it) const
    {
        return it.element != element;
    }

private:
    ElementType* element;
};

template <typename IteratorType>
struct TContigousStorage
{
    constexpr static bool IsContigous()
    {
        return IteratorType::IsContigous();
    }
};

template <typename ElementType>
class TSpan
{
public:

    TSpan() :
        data(nullptr),
        size(0)
    {
    }

    TSpan(ElementType* array, int32_t size) :
        data(array),
        size(size)
    {
    }

    template <typename IteratorType>
    TSpan(IteratorType begin, IteratorType end)
    {
        using ContigousStorageTrait = TContigousStorage<ElementType>;
        static_assert(ContigousStorageTrait::IsContigous());

        data = &(*begin);
        ElementType* e = &(*end);
        size = int32_t(e - data);
    }

    TSpan(const TSpan<ElementType>&) = default;

    TSpanIterator<ElementType> begin()
    {
        return TSpanIterator<ElementType>(data);
    }
    TSpanIterator<ElementType> end()
    {
        return TSpanIterator<ElementType>(data + size);
    }

    ElementType& operator[](int32_t i) const
    {
        assert(i >= 0 && i < size);
        return data[i];
    }

    intptr_t GetNumBytes() const
    {
        return size * sizeof(ElementType);
    }

    int32_t GetNumElements() const
    {
        return size;
    }

    ElementType* GetData() const
    {
        return data;
    }

    bool IsEmpty() const
    {
        return size == 0;
    }

private:
    ElementType* data;
    int32_t size;
};