#pragma once

#include "Delegate.h"

template <typename ElementType>
struct TListNode
{
    ElementType Data;
    TListNode<ElementType>* Next;
    TListNode<ElementType>* Previous;

    template <typename ...Args>
    TListNode(Args&& ...args) :
        Data(std::forward<Args>(args)...),
        Next(nullptr),
        Previous(nullptr)
    {
    }

    template <typename ...Args>
    void Reset(Args&& ...args)
    {
        Data = ElementType(std::forward<Args>(args)...);
    }
};

template <typename ElementType>
class TListIterator
{
public:
    TListIterator(TListNode<ElementType>* node, int32_t i);

    bool operator==(const TListIterator<ElementType>& other) const;
    bool operator!=(const TListIterator<ElementType>& other) const;

    TListIterator<ElementType>& operator++();
    TListIterator<ElementType> operator++(int32_t);

    ElementType& operator*();

private:
    TListNode<ElementType>* m_CurrentNode;
    int32_t m_Index;
};

template <typename ElementType>
class TConstListIterator
{
public:
    TConstListIterator(const TListNode<ElementType>* node, int32_t i);

    bool operator==(const TConstListIterator<ElementType>& other) const;
    bool operator!=(const TConstListIterator<ElementType>& other) const;

    TConstListIterator<ElementType>& operator++();
    TConstListIterator<ElementType> operator++(int32_t);

    const ElementType& operator*() const;

private:
    const TListNode<ElementType>* m_CurrentNode;
    int32_t m_Index;
};

template <typename ElementType>
class TList
{
public:
    TList();
    TList(std::initializer_list<ElementType> list);
    TList(const TList<ElementType>& list);
    TList<ElementType>& operator=(const TList<ElementType>& list);

    template <typename IteratorType>
    TList(IteratorType begin, IteratorType end);

    ~TList() noexcept;

public:

    void Clear();

    template <typename ...Args>
    void Replace(int32_t index, Args&& ...args);

    void Add(const ElementType& data);
    void PushBack(const ElementType& data);
    void PushBack(ElementType&& data);
    void InsertFront(const ElementType& data);

    template <typename ...Args>
    void EmplaceBack(Args&& ...args);

    int32_t Find(const ElementType& data) const;

    bool Remove(const ElementType& data);
    bool RemoveIf(const TDelegate<bool(const ElementType&)>& predicate);
    bool Contains(const ElementType& data) const;

    ElementType& operator[](int32_t index);
    const ElementType& operator[](int32_t index) const;

    int32_t GetNumElements() const;
    bool IsEmpty() const;

    TListIterator<ElementType> begin();
    TListIterator<ElementType> end();

    TConstListIterator<ElementType> begin() const;
    TConstListIterator<ElementType> end() const;

private:
    TListNode<ElementType>* m_Root;
    int32_t m_NumElements;

private:
    TListNode<ElementType>* GetNodeAt(int32_t index);
    const TListNode<ElementType>* GetNodeAt(int32_t index) const;

    void InsertNodeAtFront(TListNode<ElementType>* node);
    void InsertNodeAtBack(TListNode<ElementType>* node);

    void UnlinkFromHierarchy(TListNode<ElementType>* iterator);
    void AssignNewHierarchy(TListNode<ElementType>* root);
};


template<typename ElementType>
inline TList<ElementType>::TList() :
    m_Root(nullptr),
    m_NumElements(0)
{
}

template<typename ElementType>
inline TList<ElementType>::TList(std::initializer_list<ElementType> list):
    TList{list.begin(), list.end()}
{
}

template<typename ElementType>
inline TList<ElementType>::TList(const TList<ElementType>& list) :
    TList(list.begin(), list.end())
{
}

template<typename ElementType>
inline TList<ElementType>& TList<ElementType>::operator=(const TList<ElementType>& list)
{
    if (&list != this)
    {
        return *this;
    }

    for (auto it = list.begin(); it != list.end(); ++it)
    {
        PushBack(*it);
    }

    return *this;
}

template<typename ElementType>
inline TList<ElementType>::~TList() noexcept
{
    Clear();
}

template<typename ElementType>
inline void TList<ElementType>::Clear()
{
    if (m_Root != nullptr)
    {
        TListNode<ElementType>* node = m_Root->Next;

        /* First delete all objects from range root->Next to root->Previous */
        while (node != m_Root)
        {
            TListNode<ElementType>* temp = node->Next;
            delete node;
            node = temp;
        }

        /* It is safe to just delete root */
        delete m_Root;
    }

    m_Root = nullptr;
    m_NumElements = 0;
}

template<typename ElementType>
inline void TList<ElementType>::Add(const ElementType& data)
{
    EmplaceBack(data);
}

template<typename ElementType>
inline void TList<ElementType>::PushBack(const ElementType& data)
{
    EmplaceBack(data);
}

template<typename ElementType>
inline void TList<ElementType>::PushBack(ElementType&& data)
{
    EmplaceBack(std::move(data));
}

template<typename ElementType>
inline void TList<ElementType>::InsertFront(const ElementType& data)
{
    EmplaceBack(data);
}

template<typename ElementType>
inline int32_t TList<ElementType>::Find(const ElementType& data) const
{
    TListNode<ElementType>* iterator = m_Root;

    if (IsEmpty())
    {
        return false;
    }

    int32_t index = 0;
    while (iterator->Next != m_Root)
    {
        if (iterator->Data == data)
        {
            return index;
        }

        index++;
        iterator = iterator->Next;
    }

    return nullptr;
}

template<typename ElementType>
inline bool TList<ElementType>::Remove(const ElementType& data)
{
    TListNode<ElementType>* iterator = m_Root;

    if (IsEmpty())
    {
        return false;
    }

    while (iterator != nullptr && iterator->Data != data)
    {
        iterator = iterator->Next;
    }

    if (iterator != nullptr)
    {
        UnlinkFromHierarchy(iterator);
        return true;
    }

    return false;
}

template<typename ElementType>
inline bool TList<ElementType>::RemoveIf(const TDelegate<bool(const ElementType&)>& predicate)
{
    TListNode<ElementType>* iterator = m_Root;

    if (IsEmpty())
    {
        return false;
    }

    while (iterator != nullptr && !predicate(iterator->Data))
    {
        iterator = iterator->Next;
    }

    if (iterator != nullptr)
    {
        UnlinkFromHierarchy(iterator);
        return true;
    }

    return false;
}

template<typename ElementType>
inline bool TList<ElementType>::Contains(const ElementType& data) const
{
    TListNode<ElementType>* iterator = m_Root;

    while (true)
    {
        if (iterator->Data == data)
        {
            return true;
        }

        iterator = iterator->Next;
        if (iterator == m_Root)
        {
            break;
        }
    }

    return false;
}

template<typename ElementType>
inline ElementType& TList<ElementType>::operator[](int32_t index)
{
    TListNode<ElementType>* iterator = GetNodeAt(index);

    if (!iterator)
    {
        throw std::runtime_error("Trying to access empty list");
    }

    return iterator->Data;
}

template<typename ElementType>
inline const ElementType& TList<ElementType>::operator[](int32_t index) const
{
    const TListNode<ElementType>* iterator = GetNodeAt(index);

    if (!iterator)
    {
        throw std::runtime_error("Trying to access empty list");
    }

    return iterator->Data;
}

template<typename ElementType>
inline int32_t TList<ElementType>::GetNumElements() const
{
    return m_NumElements;
}

template<typename ElementType>
inline bool TList<ElementType>::IsEmpty() const
{
    return m_NumElements == 0;
}

template<typename ElementType>
inline TListIterator<ElementType> TList<ElementType>::begin()
{
    return TListIterator<ElementType>(m_Root, 0);
}

template<typename ElementType>
inline TListIterator<ElementType> TList<ElementType>::end()
{
    if (IsEmpty())
    {
        return TListIterator<ElementType>(nullptr, 0);
    }

    return TListIterator<ElementType>(m_Root->Previous, m_NumElements);
}

template<typename ElementType>
inline TConstListIterator<ElementType> TList<ElementType>::begin() const
{
    return TConstListIterator<ElementType>(m_Root, 0);
}

template<typename ElementType>
inline TConstListIterator<ElementType> TList<ElementType>::end() const
{
    if (IsEmpty())
    {
        return TConstListIterator<ElementType>(nullptr, 0);
    }

    return TConstListIterator<ElementType>(m_Root->Previous, m_NumElements);
}

template<typename ElementType>
inline TListNode<ElementType>* TList<ElementType>::GetNodeAt(int32_t index)
{
    if (index >= m_NumElements)
    {
        return nullptr;
    }

    if (index == 0)
    {
        return m_Root;
    }

    // check to from which iterator is easier to find object
    if (index > m_NumElements / 2)
    {
        TListNode<ElementType>* iterator = m_Root->Previous;
        int32_t tempIndex = m_NumElements - 1;

        while (iterator != m_Root)
        {
            if (tempIndex == index)
            {
                return iterator;
            }

            iterator = iterator->Previous;
            tempIndex--;
        }
    }
    else
    {
        TListNode<ElementType>* iterator = m_Root->Next;
        int32_t tempIndex = 1;

        while (iterator != m_Root)
        {
            if (tempIndex == index)
            {
                return iterator;
            }

            iterator = iterator->Next;
            tempIndex++;
        }
    }

    return nullptr;
}

template<typename ElementType>
inline const TListNode<ElementType>* TList<ElementType>::GetNodeAt(int32_t index) const
{
    if (index >= m_NumElements)
    {
        return nullptr;
    }

    if (index == 0)
    {
        return m_Root;
    }

    // check to from which iterator is easier to find object
    if (index > m_NumElements / 2)
    {
        TListNode<ElementType>* iterator = m_Root->Previous;
        int32_t tempIndex = m_NumElements - 1;

        while (iterator != m_Root)
        {
            if (tempIndex == index)
            {
                return iterator;
            }

            iterator = iterator->Previous;
            tempIndex--;
        }
    }
    else
    {
        TListNode<ElementType>* iterator = m_Root->Next;
        int32_t tempIndex = 1;

        while (iterator != m_Root)
        {
            if (tempIndex == index)
            {
                return iterator;
            }

            iterator = iterator->Next;
            tempIndex++;
        }
    }

    return nullptr;
}

template<typename ElementType>
inline void TList<ElementType>::InsertNodeAtFront(TListNode<ElementType>* node)
{
    node->Next = m_Root;
    node->Previous = m_Root->Previous;
    m_Root->Previous->Next = node;
    m_Root->Previous = node;
    m_Root = node;
    ++m_NumElements;
}

template<typename ElementType>
inline void TList<ElementType>::InsertNodeAtBack(TListNode<ElementType>* node)
{
    m_Root->Previous->Next = node;
    node->Previous = m_Root->Previous;
    node->Next = m_Root;
    m_Root->Previous = node;

    ++m_NumElements;
}

template<typename ElementType>
inline void TList<ElementType>::UnlinkFromHierarchy(TListNode<ElementType>* iterator)
{
    TListNode<ElementType>* prevNode = iterator->Previous;
    TListNode<ElementType>* nextNode = iterator->Next;

    prevNode->Next = nextNode;
    nextNode->Previous = prevNode;

    bool isIteratorSameAsRoot = iterator == m_Root;

    delete iterator;

    if (isIteratorSameAsRoot)
    {
        m_Root = prevNode;
    }

    --m_NumElements;

    if (m_NumElements == 0)
    {
        m_Root = nullptr;
    }
}

template<typename ElementType>
inline void TList<ElementType>::AssignNewHierarchy(TListNode<ElementType>* root)
{
    this->m_Root = root;
    this->m_Root->Next = this->m_Root;
    this->m_Root->Previous = this->m_Root;
    m_NumElements = 1;
}

template<typename ElementType>
template<typename IteratorType>
inline TList<ElementType>::TList(IteratorType begin, IteratorType end)
{
    m_NumElements = 0;
    m_Root = nullptr;

    for (auto it = begin; it != end; ++it)
    {
        PushBack(*it);
    }
}

template<typename ElementType>
template<typename ...Args>
inline void TList<ElementType>::Replace(int32_t index, Args && ...args)
{
    TListNode<ElementType>* node = GetNodeAt(index);
    node->Reset(std::forward<Args>(args)...);
}

template<typename ElementType>
template<typename ...Args>
inline void TList<ElementType>::EmplaceBack(Args && ...args)
{
    TListNode<ElementType>* newNode = new TListNode<ElementType>(std::forward<Args>(args)...);

    /* Is adding first node ? */
    if (IsEmpty())
    {
        AssignNewHierarchy(newNode);
        return;
    }

    /* Link the nodes in two linked manner */
    InsertNodeAtBack(newNode);
}

template <typename ElementType>
inline TListIterator<ElementType>::TListIterator(TListNode<ElementType>* node, int32_t index) :
    m_CurrentNode{node},
    m_Index{index}
{
}

template <typename ElementType>
inline bool TListIterator<ElementType>::operator==(const TListIterator<ElementType>& otherIterator) const
{
    return m_Index == otherIterator.m_Index;
}

template <typename ElementType>
inline bool TListIterator<ElementType>::operator!=(const TListIterator<ElementType>& otherIterator) const
{
    return m_Index != otherIterator.m_Index;
}

template<typename ElementType>
inline TListIterator<ElementType>& TListIterator<ElementType>::operator++()
{
    ++m_Index;
    m_CurrentNode = m_CurrentNode->Next;
    return *this;
}

template <typename ElementType>
inline TListIterator<ElementType> TListIterator<ElementType>::operator++(int32_t)
{
    return TListIterator<ElementType>(m_CurrentNode->Next, m_Index + 1);
}

template <typename ElementType>
inline ElementType& TListIterator<ElementType>::operator*()
{
    return m_CurrentNode->Data;
}

template<typename ElementType>
inline TConstListIterator<ElementType>::TConstListIterator(const TListNode<ElementType>* node, int32_t i) :
    m_CurrentNode(node),
    m_Index(i)
{
}

template <typename ElementType>
inline bool TConstListIterator<ElementType>::operator==(const TConstListIterator<ElementType>& otherIterator) const
{
    return m_Index == otherIterator.m_Index;
}

template <typename ElementType>
inline bool TConstListIterator<ElementType>::operator!=(const TConstListIterator<ElementType>& otherIterator) const
{
    return m_Index != otherIterator.m_Index;
}

template<typename ElementType>
inline TConstListIterator<ElementType>& TConstListIterator<ElementType>::operator++()
{
    ++m_Index;
    m_CurrentNode = m_CurrentNode->Next;
    return *this;
}

template <typename ElementType>
inline TConstListIterator<ElementType> TConstListIterator<ElementType>::operator++(int32_t)
{
    return TConstListIterator<ElementType>(m_CurrentNode->Next, m_Index + 1);
}

template <typename ElementType>
inline const ElementType& TConstListIterator<ElementType>::operator*() const
{
    return m_CurrentNode->Data;
}
