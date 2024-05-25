#pragma once

#pragma once

#include <malloc.h>
#include <utility>

#include <cstdlib>
#include <initializer_list>
#include <algorithm>
#include <cassert>

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

    TArrayIterator(ContainerType& container, int32_t index) :
        container(&container),
        index(index)
    {
    }

    TArrayIterator(const SelfIterator&) = default;
    TArrayIterator& operator=(const SelfIterator&) = default;

    bool operator==(const SelfIterator& iterator)
    {
        return iterator.container == container && iterator.index == index;
    }

    bool operator!=(const SelfIterator& iterator)
    {
        return iterator.container != container || iterator.index != index;
    }

    ValueType& operator*() const
    {
        assert(HasValidIndex());
        return (*container)[index];
    }

    ValueType& operator*()
    {
        assert(HasValidIndex());
        return (*container)[index];
    }

    ValueType* operator->() const
    {
        assert(HasValidIndex());
        return container->GetData() + index;
    }

    SelfIterator& operator++()
    {
        assert(HasValidIndex());
        ++index;

        return *this;
    }

    SelfIterator& operator--()
    {
        assert(HasValidIndex());
        --index;

        return *this;
    }

    SelfIterator operator++(int)
    {
        assert(HasValidIndex());
        SelfIterator it{container, index + 1};
        return it;
    }

    SelfIterator operator--(int)
    {
        assert(HasValidIndex());
        SelfIterator it{container, index - 1};
        return it;
    }

    bool HasValidIndex()const
    {
        return index >= 0 && index < container->GetNumElements();
    }

    int32_t GetIndex() const
    {
        return index;
    }

private:

    ContainerType* container;
    int32_t index;
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
        data{nullptr},
        numElements{0},
        numAlloc{0}
    {
    }

    TArray(std::initializer_list<ElementType> elements) :
        data{nullptr},
        numElements{0},
        numAlloc{0}
    {
        Append(elements);
    }

    template <typename OtherElementType, typename OtherAllocator>
    TArray(const TArray<OtherElementType, OtherAllocator>& elements) :
        data{nullptr},
        numElements{0},
        numAlloc{0}
    {
        static_assert(std::is_convertible_v<OtherElementType, ElementType>, "OtherElementType must be convertible to ElementType");

        AllocAbs(elements.GetNumElements());
        for (auto& elementType : elements)
        {
            EmplaceBack(elementType);
        }
    }

    TArray(TArray<ElementType, AllocatorType>&& elements) noexcept
    {
        data = std::exchange(elements.data, nullptr);
        numElements = std::exchange(elements.numElements, 0);
        numAlloc = std::exchange(elements.numAlloc, 0);
        allocator = std::exchange(elements.allocator, AllocatorType{});
    }

    TArray& operator=(TArray<ElementType, AllocatorType>&& elements) noexcept
    {
        data = std::exchange(elements.data, nullptr);
        numElements = std::exchange(elements.numElements, 0);
        numAlloc = std::exchange(elements.numAlloc, 0);
        allocator = std::exchange(elements.allocator, AllocatorType{});

        return *this;
    }

    ~TArray() noexcept
    {
        Empty();
        allocator.Free(data);
    }

    template <typename ...Args>
    void EmplaceBack(Args&& ...args)
    {
        TryExpand();

        allocator.ConstructElement(&data[numElements], std::forward<Args>(args)...);
        numElements++;
    }

    void SetIndex(const ElementType& element, int32_t index)
    {
        if (IsValidIndex(index))
        {
            data[index] = element;
        }
    }

    void SetIndex(ElementType&& element, int32_t index)
    {
        if (IsValidIndex(index))
        {
            data[index] = std::move(element);
        }
    }

    template <typename ...Args>
    void EmplaceAt(int32_t index, Args&& ...args)
    {
        assert(index >= 0 && index < numElements);
        TryExpand();

        std::move(&data[index], &data[numElements], &data[index + 1]);
        allocator.ConstructElement(&data[index], std::forward<Args>(args)...);

        numElements++;
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
        return numElements - 1;
    }

    int32_t Add(ElementType&& element)
    {
        EmplaceBack(std::move(element));
        return numElements - 1;
    }

    void AddZeroed(int32_t numZeroed)
    {
        AllocAbs(numElements + numZeroed + 1);

        allocator.ConstructDefaultRange(&data[numElements], &data[numElements + numZeroed]);
        numElements += numZeroed;
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
        auto i = std::find(data, data + numElements, elementType);

        if (i == data + numElements)
        {
            return Add(std::move(elementType));
        }

        return (int32_t)std::distance(data, i);
    }

    void Append(std::initializer_list<ElementType> type)
    {
        Append(type.begin(), (int32_t)type.size());
    }

    void Append(const ElementType* data, int32_t size)
    {
        AllocAbs(numElements + size);

        for (int32_t i = 0; i < size; ++i)
        {
            Add(data[i]);
        }
    }


    template <typename OtherElementType, typename OtherAllocator>
    void Append(const TArray<OtherElementType, OtherAllocator>& elements)
    {
        AllocAbs(numElements + elements.GetNumElements());

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

        int32_t newCapacity = delta + numAlloc;
        ElementType* data = (ElementType*)allocator.Allocate(newCapacity * sizeof(ElementType));

        for (int32_t i = 0; i < numElements; ++i)
        {
            allocator.ConstructElement(&data[i], std::move(data[i]));
        }

        allocator.DestroyRange(data, data + numElements);
        allocator.Free(data);

        data = data;
        numAlloc = newCapacity;
    }

    void AllocAbs(int32_t abs)
    {
        AllocDelta(abs - numAlloc);
    }

    int32_t GetNumElements() const
    {
        return numElements;
    }

    int32_t GetNumAlloc() const
    {
        return numAlloc;
    }

    int32_t GetSizeBytes() const
    {
        return numElements * sizeof(ElementType);
    }

    int32_t FindIndexOf(const ElementType& type) const
    {
        for (int32_t i = 0; i < numElements; ++i)
        {
            if (data[i] == type)
            {
                return i;
            }
        }

        return IndexNone;
    }

    template <typename Predicate>
    int32_t FindIndexOfByPredicate(Predicate&& predicate) const
    {
        for (int32_t i = 0; i < numElements; ++i)
        {
            if (predicate(data[i]))
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
        if (numElements == numAlloc)
        {
            return;
        }

        int32_t newCapacity = numElements;
        ElementType* data = (ElementType*)allocator.Allocate(newCapacity * sizeof(ElementType));

        for (int32_t i = 0; i < numElements; ++i)
        {
            allocator.ConstructElement(&data[i], std::move(data[i]));
        }

        allocator.DestroyRange(data, data + numElements);
        allocator.Free(data);

        data = data;
        numAlloc = newCapacity;
    }

    void RemoveIndex(int32_t index)
    {
        assert(index >= 0 && index < numElements);

        std::move(&data[index + 1], &data[numElements], &data[index]);
        numElements--;
    }

    void Remove(const ElementType& type)
    {
        RemoveIndex(FindIndexOf(type));
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
        return numElements == 0;
    }

    void Empty()
    {
        allocator.DestroyRange(data, data + numElements);
        numElements = 0;
    }

    void swap(TArray<ElementType, AllocatorType>& array)
    {
        data = std::exchange(array.data, data);
        numElements = std::exchange(array.numElements, numElements);
        numAlloc = std::exchange(array.numAlloc, numAlloc);
        allocator = std::exchange(array.allocator, allocator);
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
        return Iterator{*this, numElements};
    }

    ConstIterator end() const
    {
        return ConstIterator{*this, numElements};
    }

    bool IsValidIndex(int32_t index) const
    {
        return index >= 0 && index < numElements;
    }

    ElementType& operator[](int32_t index)
    {
        assert(IsValidIndex(index));
        return data[index];
    }

    const ElementType& operator[](int32_t index) const
    {
        assert(IsValidIndex(index));
        return data[index];
    }

    template <typename Predicate = std::less<ElementType>>
    void Sort(Predicate&& predicate)
    {
        std::sort(data, data + numElements, predicate);
    }

private:
    ElementType* data;
    int32_t numElements;
    int32_t numAlloc;
    AllocatorType allocator;

    void TryExpand()
    {
        if (numElements >= numAlloc)
        {
            int32_t cap = numAlloc + numAlloc / 2;
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