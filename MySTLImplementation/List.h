#pragma once

#include "Delegate.h"

template <typename ElementType>
struct TListNode
{
    ElementType data;
    TListNode<ElementType>* next;
    TListNode<ElementType>* previous;

    template <typename ...Args>
    TListNode(Args&& ...args) :
        data(std::forward<Args>(args)...),
        next(nullptr),
        previous(nullptr)
    {
    }

    template <typename ...Args>
    void Reset(Args&& ...args)
    {
        data = ElementType(std::forward<Args>(args)...);
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
    TListNode<ElementType>* currentNode;
    int32_t index;
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
    const TListNode<ElementType>* currentNode;
    int32_t index;
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
    TListNode<ElementType>* root;
    int32_t numElements;

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
    root(nullptr),
    numElements(0)
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
    if (root != nullptr)
    {
        TListNode<ElementType>* node = root->next;

        /* First delete all objects from range root->Next to root->Previous */
        while (node != root)
        {
            TListNode<ElementType>* temp = node->next;
            delete node;
            node = temp;
        }

        /* It is safe to just delete root */
        delete root;
    }

    root = nullptr;
    numElements = 0;
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
    TListNode<ElementType>* iterator = root;

    if (IsEmpty())
    {
        return false;
    }

    int32_t index = 0;
    while (iterator->next != root)
    {
        if (iterator->data == data)
        {
            return index;
        }

        index++;
        iterator = iterator->next;
    }

    return IndexNone;
}

template<typename ElementType>
inline bool TList<ElementType>::Remove(const ElementType& data)
{
    TListNode<ElementType>* iterator = root;

    if (IsEmpty())
    {
        return false;
    }

    while (iterator != nullptr && iterator->data != data)
    {
        iterator = iterator->next;
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
    TListNode<ElementType>* iterator = root;

    if (IsEmpty())
    {
        return false;
    }

    while (iterator != nullptr && !predicate(iterator->data))
    {
        iterator = iterator->next;
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
    TListNode<ElementType>* iterator = root;

    while (true)
    {
        if (iterator->data == data)
        {
            return true;
        }

        iterator = iterator->next;
        if (iterator == root)
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
    assert(iterator && "Trying to access empty list");
    return iterator->data;
}

template<typename ElementType>
inline const ElementType& TList<ElementType>::operator[](int32_t index) const
{
    const TListNode<ElementType>* iterator = GetNodeAt(index);
    assert(iterator && "Trying to access empty list");
    return iterator->data;
}

template<typename ElementType>
inline int32_t TList<ElementType>::GetNumElements() const
{
    return numElements;
}

template<typename ElementType>
inline bool TList<ElementType>::IsEmpty() const
{
    return numElements == 0;
}

template<typename ElementType>
inline TListIterator<ElementType> TList<ElementType>::begin()
{
    return TListIterator<ElementType>(root, 0);
}

template<typename ElementType>
inline TListIterator<ElementType> TList<ElementType>::end()
{
    if (IsEmpty())
    {
        return TListIterator<ElementType>(nullptr, 0);
    }

    return TListIterator<ElementType>(root->previous, numElements);
}

template<typename ElementType>
inline TConstListIterator<ElementType> TList<ElementType>::begin() const
{
    return TConstListIterator<ElementType>(root, 0);
}

template<typename ElementType>
inline TConstListIterator<ElementType> TList<ElementType>::end() const
{
    if (IsEmpty())
    {
        return TConstListIterator<ElementType>(nullptr, 0);
    }

    return TConstListIterator<ElementType>(root->previous, numElements);
}

template<typename ElementType>
inline TListNode<ElementType>* TList<ElementType>::GetNodeAt(int32_t index)
{
    if (index >= numElements)
    {
        return nullptr;
    }

    if (index == 0)
    {
        return root;
    }

    // check to from which iterator is easier to find object
    if (index > numElements / 2)
    {
        TListNode<ElementType>* iterator = root->previous;
        int32_t tempIndex = numElements - 1;

        while (iterator != root)
        {
            if (tempIndex == index)
            {
                return iterator;
            }

            iterator = iterator->previous;
            tempIndex--;
        }
    }
    else
    {
        TListNode<ElementType>* iterator = root->next;
        int32_t tempIndex = 1;

        while (iterator != root)
        {
            if (tempIndex == index)
            {
                return iterator;
            }

            iterator = iterator->next;
            tempIndex++;
        }
    }

    return nullptr;
}

template<typename ElementType>
inline const TListNode<ElementType>* TList<ElementType>::GetNodeAt(int32_t index) const
{
    if (index >= numElements)
    {
        return nullptr;
    }

    if (index == 0)
    {
        return root;
    }

    // check to from which iterator is easier to find object
    if (index > numElements / 2)
    {
        TListNode<ElementType>* iterator = root->previous;
        int32_t tempIndex = numElements - 1;

        while (iterator != root)
        {
            if (tempIndex == index)
            {
                return iterator;
            }

            iterator = iterator->previous;
            tempIndex--;
        }
    }
    else
    {
        TListNode<ElementType>* iterator = root->next;
        int32_t tempIndex = 1;

        while (iterator != root)
        {
            if (tempIndex == index)
            {
                return iterator;
            }

            iterator = iterator->next;
            tempIndex++;
        }
    }

    return nullptr;
}

template<typename ElementType>
inline void TList<ElementType>::InsertNodeAtFront(TListNode<ElementType>* node)
{
    node->next = root;
    node->previous = root->previous;
    root->previous->next = node;
    root->previous = node;
    root = node;
    ++numElements;
}

template<typename ElementType>
inline void TList<ElementType>::InsertNodeAtBack(TListNode<ElementType>* node)
{
    root->previous->next = node;
    node->previous = root->previous;
    node->next = root;
    root->previous = node;

    ++numElements;
}

template<typename ElementType>
inline void TList<ElementType>::UnlinkFromHierarchy(TListNode<ElementType>* iterator)
{
    TListNode<ElementType>* prevNode = iterator->previous;
    TListNode<ElementType>* nextNode = iterator->next;

    prevNode->next = nextNode;
    nextNode->previous = prevNode;

    bool isIteratorSameAsRoot = iterator == root;

    delete iterator;

    if (isIteratorSameAsRoot)
    {
        root = prevNode;
    }

    --numElements;

    if (numElements == 0)
    {
        root = nullptr;
    }
}

template<typename ElementType>
inline void TList<ElementType>::AssignNewHierarchy(TListNode<ElementType>* root)
{
    this->root = root;
    this->root->next = this->root;
    this->root->previous = this->root;
    numElements = 1;
}

template<typename ElementType>
template<typename IteratorType>
inline TList<ElementType>::TList(IteratorType begin, IteratorType end)
{
    numElements = 0;
    root = nullptr;

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
    currentNode{node},
    index{index}
{
}

template <typename ElementType>
inline bool TListIterator<ElementType>::operator==(const TListIterator<ElementType>& otherIterator) const
{
    return index == otherIterator.index;
}

template <typename ElementType>
inline bool TListIterator<ElementType>::operator!=(const TListIterator<ElementType>& otherIterator) const
{
    return index != otherIterator.index;
}

template<typename ElementType>
inline TListIterator<ElementType>& TListIterator<ElementType>::operator++()
{
    ++index;
    currentNode = currentNode->next;
    return *this;
}

template <typename ElementType>
inline TListIterator<ElementType> TListIterator<ElementType>::operator++(int32_t)
{
    return TListIterator<ElementType>(currentNode->next, index + 1);
}

template <typename ElementType>
inline ElementType& TListIterator<ElementType>::operator*()
{
    return currentNode->data;
}

template<typename ElementType>
inline TConstListIterator<ElementType>::TConstListIterator(const TListNode<ElementType>* node, int32_t i) :
    currentNode(node),
    index(i)
{
}

template <typename ElementType>
inline bool TConstListIterator<ElementType>::operator==(const TConstListIterator<ElementType>& otherIterator) const
{
    return index == otherIterator.index;
}

template <typename ElementType>
inline bool TConstListIterator<ElementType>::operator!=(const TConstListIterator<ElementType>& otherIterator) const
{
    return index != otherIterator.index;
}

template<typename ElementType>
inline TConstListIterator<ElementType>& TConstListIterator<ElementType>::operator++()
{
    ++index;
    currentNode = currentNode->next;
    return *this;
}

template <typename ElementType>
inline TConstListIterator<ElementType> TConstListIterator<ElementType>::operator++(int32_t)
{
    return TConstListIterator<ElementType>(currentNode->next, index + 1);
}

template <typename ElementType>
inline const ElementType& TConstListIterator<ElementType>::operator*() const
{
    return currentNode->data;
}
