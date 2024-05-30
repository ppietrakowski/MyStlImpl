#pragma once

#pragma once

#include <malloc.h>
#include <utility>

#include <cstdlib>
#include <initializer_list>
#include <algorithm>
#include <cassert>

#include "Span.h"

struct DefaultAllocator
{
    void* Allocate(size_t size)
    {
        void* m = calloc(1, size);
        if (!m)
        {
            std::exit(EXIT_FAILURE);
        }

        return m;
    }

    void Free(void* memory)
    {
        if (memory)
        {
            free(memory);
        }
    }

    template <typename T>
    void ConstructDefaultRange(T* begin, T* end)
    {
        for (T* i = begin; i != end; ++i)
        {
            new (i) T();
        }
    }

    template <typename T, typename ...Args>
    void ConstructElement(T* element, Args&& ...args)
    {
        new (element) T(std::forward<Args>(args)...);
    }

    template <typename T>
    void DestroyRange(T* begin, T* end)
    {
        for (T* i = begin; i != end; ++i)
        {
            i->~T();
        }
    }
};

constexpr int32_t IndexNone = -1;

template <typename ContainerType, typename ValueType>
class TArrayIterator
{
public:
    using SelfIterator = TArrayIterator<ContainerType, ValueType>;

    constexpr static bool IsContigous()
    {
        return true;
    }

    TArrayIterator(ContainerType& container, int32_t index) :
        m_Container(&container),
        m_Index(index)
    {
    }

    TArrayIterator(const SelfIterator&) = default;
    TArrayIterator& operator=(const SelfIterator&) = default;

    bool operator==(const SelfIterator& iterator)
    {
        return iterator.m_Container == m_Container && iterator.m_Index == m_Index;
    }

    bool operator!=(const SelfIterator& iterator)
    {
        return iterator.m_Container != m_Container || iterator.m_Index != m_Index;
    }

    ValueType& operator*() const
    {
        assert(HasValidIndex());
        return (*m_Container)[m_Index];
    }

    ValueType& operator*()
    {
        assert(HasValidIndex());
        return (*m_Container)[m_Index];
    }

    ValueType* operator->() const
    {
        assert(HasValidIndex());
        return m_Container->GetData() + m_Index;
    }

    SelfIterator& operator++()
    {
        assert(HasValidIndex());
        ++m_Index;

        return *this;
    }

    SelfIterator& operator--()
    {
        assert(HasValidIndex());
        --m_Index;

        return *this;
    }

    SelfIterator operator++(int)
    {
        assert(HasValidIndex());
        SelfIterator it{m_Container, m_Index + 1};
        return it;
    }

    SelfIterator operator--(int)
    {
        assert(HasValidIndex());
        SelfIterator it{m_Container, m_Index - 1};
        return it;
    }

    bool HasValidIndex()const
    {
        return m_Index >= 0 && m_Index < m_Container->GetNumElements();
    }

    int32_t GetIndex() const
    {
        return m_Index;
    }

private:

    ContainerType* m_Container;
    int32_t m_Index;
};

template <typename ElementType, typename AllocatorType = DefaultAllocator>
class TArray
{
    template <typename OtherInElementType, typename OtherAllocator>
    friend class TArray;
public:
    using ValueType = ElementType;
    using ConstValueType = const ElementType;
    using SelfClass = TArray<ElementType, AllocatorType>;

    using Iterator = TArrayIterator<SelfClass, ValueType>;
    using ConstIterator = TArrayIterator<const SelfClass, ConstValueType>;

    TArray() :
        m_Data{nullptr},
        m_NumElements{0},
        m_NumAlloc{0}
    {
    }

    TArray(std::initializer_list<ElementType> elements) :
        m_Data{nullptr},
        m_NumElements{0},
        m_NumAlloc{0}
    {
        Append(elements);
    }

    template <typename OtherElementType, typename OtherAllocator>
    TArray(const TArray<OtherElementType, OtherAllocator>& elements) :
        m_Data{nullptr},
        m_NumElements{0},
        m_NumAlloc{0}
    {
        static_assert(std::is_convertible_v<OtherElementType, ElementType>, "OtherElementType must be convertible to ElementType");

        AllocAbs(elements.GetNumElements());
        for (auto& elementType : elements)
        {
            EmplaceBack(elementType);
        }
    }

    TArray(const TArray<ElementType>& elements) :
        m_Data{nullptr},
        m_NumElements{0},
        m_NumAlloc{0}
    {
        AllocAbs(elements.GetNumElements());
        for (auto& elementType : elements)
        {
            EmplaceBack(elementType);
        }
    }

    TArray& operator=(const TArray<ElementType>& elements)
    {
        Empty();
        AllocAbs(elements.GetNumElements());
        for (auto& elementType : elements)
        {
            EmplaceBack(elementType);
        }

        return *this;
    }

    TArray(TArray<ElementType, AllocatorType>&& elements) noexcept
    {
        m_Data = std::exchange(elements.m_Data, nullptr);
        m_NumElements = std::exchange(elements.m_NumElements, 0);
        m_NumAlloc = std::exchange(elements.m_NumAlloc, 0);
        m_Allocator = std::exchange(elements.m_Allocator, AllocatorType{});
    }

    TArray& operator=(TArray<ElementType, AllocatorType>&& elements) noexcept
    {
        m_Data = std::exchange(elements.m_Data, nullptr);
        m_NumElements = std::exchange(elements.m_NumElements, 0);
        m_NumAlloc = std::exchange(elements.m_NumAlloc, 0);
        m_Allocator = std::exchange(elements.m_Allocator, AllocatorType{});

        return *this;
    }

    template <typename IteratorType>
    TArray(IteratorType begin, IteratorType end) :
        m_Data{nullptr},
        m_NumElements{0},
        m_NumAlloc{0}
    {
        for (auto i = begin; i != end; ++i)
        {
            Add(*i);
        }
    }

    ~TArray() noexcept
    {
        Empty();
        m_Allocator.Free(m_Data);
    }

public:

    template <typename ...Args>
    void EmplaceBack(Args&& ...args)
    {
        TryExpand();

        m_Allocator.ConstructElement(&m_Data[m_NumElements], std::forward<Args>(args)...);
        m_NumElements++;
    }

    void SetIndex(const ElementType& element, int32_t index)
    {
        if (IsValidIndex(index))
        {
            m_Data[index] = element;
        }
    }

    void SetIndex(ElementType&& element, int32_t index)
    {
        if (IsValidIndex(index))
        {
            m_Data[index] = std::move(element);
        }
    }

    template <typename ...Args>
    void EmplaceAt(int32_t index, Args&& ...args)
    {
        assert(index >= 0 && index < m_NumElements);
        TryExpand();

        std::move(&m_Data[index], &m_Data[m_NumElements], &m_Data[index + 1]);
        m_Allocator.ConstructElement(&m_Data[index], std::forward<Args>(args)...);

        m_NumElements++;
    }

    void PushBack(const ElementType& element)
    {
        EmplaceBack(element);
    }

    void PushBack(ElementType&& element)
    {
        EmplaceBack(std::move(element));
    }

    int32_t Add(const ElementType& element)
    {
        EmplaceBack(element);
        return m_NumElements - 1;
    }

    int32_t Add(ElementType&& element)
    {
        EmplaceBack(std::move(element));
        return m_NumElements - 1;
    }

    void AddZeroed(int32_t numZeroed)
    {
        AllocAbs(m_NumElements + numZeroed);

        m_Allocator.ConstructDefaultRange(&m_Data[m_NumElements], &m_Data[m_NumElements + numZeroed]);
        m_NumElements += numZeroed;
    }

    int32_t AddUnique(const ElementType& elementType)
    {
        int32_t i = FindIndexOf(elementType);

        if (i != IndexNone)
        {
            return Add(elementType);
        }

        return i;
    }

    int32_t AddUnique(ElementType&& elementType)
    {
        auto i = std::find(m_Data, m_Data + m_NumElements, elementType);

        if (i == m_Data + m_NumElements)
        {
            return Add(std::move(elementType));
        }

        return (int32_t)std::distance(m_Data, i);
    }

    void Append(std::initializer_list<ElementType> type)
    {
        Append(type.begin(), (int32_t)type.size());
    }

    void Append(const ElementType* data, int32_t size)
    {
        AllocAbs(m_NumElements + size);

        for (int32_t i = 0; i < size; ++i)
        {
            Add(data[i]);
        }
    }


    template <typename OtherElementType, typename OtherAllocator>
    void Append(const TArray<OtherElementType, OtherAllocator>& elements)
    {
        AllocAbs(m_NumElements + elements.GetNumElements());

        for (const OtherElementType& element : elements)
        {
            Add(static_cast<ElementType>(element));
        }
    }

    void AllocDelta(int32_t delta)
    {
        if (delta < 0)
        {
            return;
        }

        int32_t newCapacity = delta + m_NumAlloc;
        ElementType* data = (ElementType*)m_Allocator.Allocate(newCapacity * sizeof(ElementType));

        for (int32_t i = 0; i < m_NumElements; ++i)
        {
            m_Allocator.ConstructElement(&data[i], std::move(m_Data[i]));
        }

        m_Allocator.DestroyRange(m_Data, m_Data + m_NumElements);
        m_Allocator.Free(m_Data);

        m_Data = data;
        m_NumAlloc = newCapacity;
    }

    void AllocAbs(int32_t abs)
    {
        AllocDelta(abs - m_NumAlloc);
    }

    int32_t GetNumElements() const
    {
        return m_NumElements;
    }

    int32_t GetNumAlloc() const
    {
        return m_NumAlloc;
    }

    int32_t GetSizeBytes() const
    {
        return m_NumElements * sizeof(ElementType);
    }

    int32_t FindIndexOf(const ElementType& type) const
    {
        for (int32_t i = 0; i < m_NumElements; ++i)
        {
            if (m_Data[i] == type)
            {
                return i;
            }
        }

        return IndexNone;
    }

    template <typename Predicate>
    int32_t FindIndexOfByPredicate(Predicate&& predicate) const
    {
        for (int32_t i = 0; i < m_NumElements; ++i)
        {
            if (predicate(m_Data[i]))
            {
                return i;
            }
        }

        return IndexNone;
    }

    bool Contains(const ElementType& element) const
    {
        int32_t i = FindIndexOf(element);
        return i != IndexNone;
    }

    template <typename Predicate>
    bool ContainsByPredicate(Predicate&& predicate) const
    {
        int32_t i = FindIndexOfByPredicate(std::forward<Predicate>(predicate)...);
        return i != IndexNone;
    }

    void ShrinkToFit()
    {
        if (m_NumElements == m_NumAlloc)
        {
            return;
        }

        int32_t newCapacity = m_NumElements;
        ElementType* data = (ElementType*)m_Allocator.Allocate(newCapacity * sizeof(ElementType));

        for (int32_t i = 0; i < m_NumElements; ++i)
        {
            m_Allocator.ConstructElement(&data[i], std::move(m_Data[i]));
        }

        m_Allocator.DestroyRange(data, data + m_NumElements);
        m_Allocator.Free(data);

        m_Data = data;
        m_NumAlloc = newCapacity;
    }

    void RemoveIndex(int32_t index)
    {
        assert(index >= 0 && index < m_NumElements);

        std::move(&m_Data[index + 1], &m_Data[m_NumElements], &m_Data[index]);
        m_NumElements--;
    }

    void Remove(const ElementType& type)
    {
        RemoveIndex(FindIndexOf(type));
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
        return m_NumElements == 0;
    }

    void Empty()
    {
        m_Allocator.DestroyRange(m_Data, m_Data + m_NumElements);
        m_NumElements = 0;
    }

    void swap(TArray<ElementType, AllocatorType>& array)
    {
        m_Data = std::exchange(array.m_Data, m_Data);
        m_NumElements = std::exchange(array.m_NumElements, m_NumElements);
        m_NumAlloc = std::exchange(array.m_NumAlloc, m_NumAlloc);
        m_Allocator = std::exchange(array.m_Allocator, m_Allocator);
    }

    Iterator begin()
    {
        return Iterator{*this, 0};
    }

    ConstIterator begin() const
    {
        return ConstIterator{*this, 0};
    }

    Iterator end()
    {
        return Iterator{*this, m_NumElements};
    }

    ConstIterator end() const
    {
        return ConstIterator{*this, m_NumElements};
    }

    bool IsValidIndex(int32_t index) const
    {
        return index >= 0 && index < m_NumElements;
    }

    ElementType& operator[](int32_t index)
    {
        assert(IsValidIndex(index));
        return m_Data[index];
    }

    const ElementType& operator[](int32_t index) const
    {
        assert(IsValidIndex(index));
        return m_Data[index];
    }

    template <typename Predicate = std::less<ElementType>>
    void Sort(Predicate&& predicate)
    {
        std::sort(m_Data, m_Data + m_NumElements, predicate);
    }

    void Sort()
    {
        Sort(std::less<ElementType>{});
    }

    template <typename Func>
    void Generate(Func&& func)
    {
        std::generate(m_Data, m_Data + m_NumElements, std::forward<Func>(func));
    }

    void Fill(const ElementType& element)
    {
        std::fill(m_Data, m_Data + m_NumElements, element);
    }

    operator TSpan<ElementType>()
    {
        return TSpan<ElementType>{m_Data, m_NumElements};
    }

    operator TSpan<const ElementType>() const
    {
        return TSpan<const ElementType>{m_Data, m_NumElements};
    }

    ElementType& Back()
    {
        return m_Data[m_NumElements - 1];
    }

    const ElementType& Back() const
    {
        return m_Data[m_NumElements - 1];
    }

    const ElementType& Front() const
    {
        return m_Data[0];
    }

    ElementType& Front()
    {
        return m_Data[0];
    }

    ElementType* UncheckedBegin()
    {
        return m_Data;
    }

    const ElementType* UncheckedBegin() const
    {
        return m_Data;
    }

    ElementType* UncheckedEnd()
    {
        return m_Data + m_NumElements;
    }

    const ElementType* UncheckedEnd() const
    {
        return m_Data + m_NumElements;
    }

private:
    ElementType* m_Data;
    int32_t m_NumElements;
    int32_t m_NumAlloc;
    AllocatorType m_Allocator;

private:
    void TryExpand()
    {
        if (m_NumElements >= m_NumAlloc)
        {
            int32_t cap = m_NumAlloc + m_NumAlloc / 2;
            if (cap == 0)
            {
                cap = 16;
            }

            AllocAbs(cap);
        }
    }
};

template <typename ElementType, int32_t Size>
struct TStaticArray
{
    using ValueType = ElementType;
    using ConstValueType = const ElementType;
    using SelfClass = TStaticArray<ElementType, Size>;

    using Iterator = TArrayIterator<SelfClass, ValueType>;
    using ConstIterator = TArrayIterator<SelfClass, ConstValueType>;

    ElementType Data[Size];

    constexpr bool IsValidIndex(int32_t index) const
    {
        return index >= 0 && index < Size;
    }

    constexpr ElementType& operator[](int32_t index)
    {
        assert(IsValidIndex(index));
        return Data[index];
    }

    constexpr const ElementType& operator[](int32_t index) const
    {
        assert(IsValidIndex(index));
        return Data[index];
    }

    constexpr ElementType* GetData()
    {
        return Data;
    }

    constexpr const ElementType* GetData() const
    {
        return Data;
    }

    Iterator begin()
    {
        return Iterator{*this, 0};
    }

    ConstIterator begin() const
    {
        return ConstIterator{*this, 0};
    }

    Iterator end()
    {
        return Iterator{*this, Size};
    }

    ConstIterator end() const
    {
        return ConstIterator{*this, Size};
    }

    ElementType* UncheckedBegin()
    {
        return Data;
    }

    const ElementType* UncheckedBegin() const
    {
        return Data;
    }

    ElementType* UncheckedEnd()
    {
        return Data + Size;
    }

    const ElementType* UncheckedEnd() const
    {
        return Data + Size;
    }

    constexpr int32_t GetNumElements() const
    {
        return Size;
    }

    constexpr int32_t GetSizeBytes() const
    {
        return Size * sizeof(ElementType);
    }

    int32_t FindIndexOf(const ElementType& type) const
    {
        for (int32_t i = 0; i < Size; ++i)
        {
            if (Data[i] == type)
            {
                return i;
            }
        }

        return IndexNone;
    }

    template <typename Predicate>
    int32_t FindIndexOfByPredicate(Predicate&& predicate) const
    {
        for (int32_t i = 0; i < Size; ++i)
        {
            if (predicate(Data[i]))
            {
                return i;
            }
        }

        return IndexNone;
    }

    bool Contains(const ElementType& element) const
    {
        int32_t i = FindIndexOf(element);
        return i != IndexNone;
    }

    template <typename Predicate>
    bool ContainsByPredicate(Predicate&& predicate) const
    {
        int32_t i = FindIndexOfByPredicate(std::forward<Predicate>(predicate)...);
        return i != IndexNone;
    }

    template <typename Func>
    void Generate(Func&& func)
    {
        std::generate(Data, Data + Size, std::forward<Func>(func));
    }

    void Fill(const ElementType& element)
    {
        std::fill(Data, Data + Size, element);
    }

    operator TSpan<ElementType>()
    {
        return TSpan<ElementType>{Data, Size};
    }

    operator TSpan<const ElementType>() const
    {
        return TSpan<const ElementType>{Data, Size};
    }
};

/* Template for auto detection of TStaticArray */
template <class _First, class... _Rest>
struct Enforce_same
{
    static_assert(
        std::conjunction_v<std::is_same<_First, _Rest>...>, "N4950 [array.cons]/2: Mandates: (is_same_v<T, U> && ...) is true.");
    using type = _First;
};

template <class _First, class... _Rest>
TStaticArray(_First, _Rest...) -> TStaticArray<typename Enforce_same<_First, _Rest...>::type, 1 + sizeof...(_Rest)>;

