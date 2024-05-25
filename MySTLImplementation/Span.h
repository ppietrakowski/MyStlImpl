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

template <typename ElementType>
class TConstSpanIterator
{
public:
    TConstSpanIterator(const ElementType* element) :
        element(element)
    {
    }

    TConstSpanIterator(const TConstSpanIterator<ElementType>&) = default;

    const ElementType& operator*() const
    {
        return *element;
    }

    bool operator==(const TConstSpanIterator<ElementType>& it) const
    {
        return it.element == element;
    }

    bool operator!=(const TConstSpanIterator<ElementType>& it) const
    {
        return it.element != element;
    }

private:
    const ElementType* element;
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

    TSpan(TArray<ElementType>& array) :
        data(array.GetData()),
        size(array.GetNumElements())
    {
    }

    TSpan(ElementType* array, int32_t size) :
        data(array),
        size(size)
    {
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

    TConstSpanIterator<ElementType> begin() const
    {
        return TConstSpanIterator<ElementType>(data);
    }
    TConstSpanIterator<ElementType> end() const
    {
        return TConstSpanIterator<ElementType>(data + size);
    }

    ElementType& operator[](int32_t i)
    {
        assert(i >= 0 && i < size); 
        return data[i];
    }

    const ElementType& operator[](int32_t i) const
    {
        assert(i >= 0 && i < size); 
        return data[i];
    }

    int32_t GetNumBytes() const
    {
        return size * sizeof(ElementType);
    }

    int32_t GetNum() const
    {
        return size;
    }

    ElementType* GetData()
    {
        return data;
    }
    const ElementType* GetData() const
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