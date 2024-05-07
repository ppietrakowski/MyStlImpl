#pragma once

#include <cstdint>

#include "Array.h"

template <typename ElementType>
class TSpanIterator
{
public:
    TSpanIterator(ElementType* element) :
        m_Element(element)
    {
    }

    TSpanIterator(const TSpanIterator<ElementType>&) = default;

    ElementType& operator*()
    {
        return *m_Element;
    }

    bool operator==(const TSpanIterator<ElementType>& it) const
    {
        return it.m_Element == m_Element;
    }

    bool operator!=(const TSpanIterator<ElementType>& it) const
    {
        return it.m_Element != m_Element;
    }

private:
    ElementType* m_Element;
};

template <typename ElementType>
class TConstSpanIterator
{
public:
    TConstSpanIterator(const ElementType* element) :
        m_Element(element)
    {
    }

    TConstSpanIterator(const TConstSpanIterator<ElementType>&) = default;

    const ElementType& operator*() const
    {
        return *m_Element;
    }

    bool operator==(const TConstSpanIterator<ElementType>& it) const
    {
        return it.m_Element == m_Element;
    }

    bool operator!=(const TConstSpanIterator<ElementType>& it) const
    {
        return it.m_Element != m_Element;
    }

private:
    const ElementType* m_Element;
};

template <typename ElementType>
class TSpan
{
public:
    TSpan() :
        m_Data(nullptr),
        m_Size(0)
    {
    }

    TSpan(TArray<ElementType>& array) :
        m_Data(array.GetArray()),
        m_Size(array.GetNum())
    {
    }

    TSpan(ElementType* array, int32_t size) :
        m_Data(array),
        m_Size(size)
    {
    }

    TSpan(const TSpan<ElementType>&) = default;

    TSpanIterator<ElementType> begin()
    {
        return TSpanIterator<ElementType>(m_Data);
    }
    TSpanIterator<ElementType> end()
    {
        return TSpanIterator<ElementType>(m_Data + m_Size);
    }

    TConstSpanIterator<ElementType> begin() const
    {
        return TConstSpanIterator<ElementType>(m_Data);
    }
    TConstSpanIterator<ElementType> end() const
    {
        return TConstSpanIterator<ElementType>(m_Data + m_Size);
    }

    ElementType& operator[](int32_t i)
    {
        assert(i >= 0 && i < m_Size); 
        return m_Data[i];
    }

    const ElementType& operator[](int32_t i) const
    {
        assert(i >= 0 && i < m_Size); 
        return m_Data[i];
    }

    int32_t GetNumBytes() const
    {
        return m_Size * sizeof(ElementType);
    }

    int32_t GetNum() const
    {
        return m_Size;
    }

    ElementType* GetData()
    {
        return m_Data;
    }
    const ElementType* GetData() const
    {
        return m_Data;
    }

    bool IsEmpty() const
    {
        return m_Size == 0;
    }

private:
    ElementType* m_Data;
    int32_t m_Size;
};