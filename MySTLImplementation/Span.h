#pragma once

#include <cstdint>

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

    TSpanIterator<ElementType> operator++(int) const
    {
        return TSpanIterator<ElementType>{element + 1};
    }

    TSpanIterator<ElementType>& operator++()
    {
        element++;
        return *this;
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
        m_Data(nullptr),
        m_Size(0)
    {
    }

    TSpan(ElementType* array, int32_t size) :
        m_Data(array),
        m_Size(size)
    {
    }

    template <typename IteratorType>
    TSpan(IteratorType begin, IteratorType end)
    {
        using ContigousStorageTrait = TContigousStorage<ElementType>;
        static_assert(ContigousStorageTrait::IsContigous());

        m_Data = &(*begin);
        ElementType* e = &(*end);
        m_Size = int32_t(e - m_Data);
    }

    TSpan(ElementType* begin, ElementType* end)
    {
        m_Data = &(*begin);
        ElementType* e = &(*end);
        m_Size = int32_t(e - m_Data);
    }

    template <int32_t Size>
    TSpan(ElementType(&data)[Size]) :
        m_Data(data),
        m_Size(Size)
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

    ElementType& operator[](int32_t i) const
    {
        assert(i >= 0 && i < m_Size);
        return m_Data[i];
    }

    intptr_t GetNumBytes() const
    {
        return m_Size * sizeof(ElementType);
    }

    int32_t GetNumElements() const
    {
        return m_Size;
    }

    ElementType* GetData() const
    {
        return m_Data;
    }

    bool IsEmpty() const
    {
        return m_Size == 0;
    }

    ElementType& Front() const
    {
        return m_Data[0];
    }

    ElementType& Back() const
    {
        return m_Data[m_Size - 1];
    }

private:
    ElementType* m_Data;
    int32_t m_Size;
};