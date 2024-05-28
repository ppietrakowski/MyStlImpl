#pragma once

#include <cstdint>
#include <algorithm>
#include <memory>

#include "Array.h"
#include "Optional.h"
#include "String.h"
#include <string>

#define CHECK_ITEM_EXISTS(Ptr) assert((Ptr) != nullptr && "Item with this key doesn't exits")

template <typename KeyType, typename ValueType>
struct TKeyValue
{
    KeyType Key;
    ValueType Value;

    TKeyValue() = default;

    using SelfClass = TKeyValue<KeyType, ValueType>;

    template <typename ...Args>
    TKeyValue(const KeyType& key, Args&& ...args) :
        Key{key},
        Value{std::forward<Args>(args)...}
    {
    }

    TKeyValue(const SelfClass& copy) = default;
    TKeyValue& operator=(const SelfClass& copy) = default;

    TKeyValue(SelfClass&& temp) = default;
    TKeyValue& operator=(SelfClass&& temp) = default;
};

// Container that maps Key to Value.
// It's search, inserting have logarithmic complexity
// Removing cost is same as moving array elements to new location
template <typename KeyType, typename ValueType, typename Predicate = std::less<KeyType>>
class TOrderedMap
{
public:
    using ArrayType = TKeyValue<KeyType, ValueType>;
    using iterator = TArray<ArrayType>::Iterator;
    using const_iterator = TArray<ArrayType>::ConstIterator;

    TOrderedMap() = default;

    void Append(const TOrderedMap<KeyType, ValueType, Predicate>& map)
    {
        for (auto& [key, value] : map)
        {
            Insert(key, value);
        }
    }

    void Insert(const KeyType& key, const ValueType& value)
    {
        Emplace(key, value);
    }

    template <typename ...Args>
    void Emplace(const KeyType& key, Args&& ...args)
    {
        m_Values.EmplaceBack(key, std::move(ValueType(std::forward<Args>(args)...)));

        m_Values.Sort([](const ArrayType& a, const ArrayType& b)
        {
            Predicate pd{};
            return pd(a.Key, b.Key);
        });
    }

    const ValueType* Find(const KeyType& key) const
    {
        const ArrayType* v = FindValue(key);
        return v ? &v->Value : nullptr;
    }

    ValueType* Find(const KeyType& key)
    {
        const ArrayType* v = FindValue(key);
        return v ? const_cast<ValueType*>(&v->Value) : nullptr;
    }

    const ValueType& operator[](const KeyType& key) const
    {
        auto it = Find(key);
        CHECK_ITEM_EXISTS(it);
        return *it;
    }

    ValueType& operator[](const KeyType& key)
    {
        auto it = Find(key);
        CHECK_ITEM_EXISTS(it);
        return const_cast<ValueType&>(*it);
    }

    int32_t GetNumElements() const
    {
        return m_Values.GetNumElements();
    }

    auto begin() const
    {
        return m_Values.begin();
    }

    auto end() const
    {
        return m_Values.end();
    }

    TArray<KeyType> GetKeys() const
    {
        TArray<KeyType> keys;
        keys.AllocAbs(m_Values.GetNumElements());

        for (const ArrayType& v : m_Values)
        {
            keys.Add(v.Key);
        }

        return keys;
    }

    bool Contains(const KeyType& key) const
    {
        auto it = Find(key);
        return it != nullptr;
    }

    void Clear()
    {
        m_Values.Clear();
    }

    void Remove(const KeyType& key)
    {
        const ArrayType* m = FindValue(key);
        int32_t index = static_cast<int32_t>(m - m_Values.GetData());
        m_Values.RemoveIndex(index);
    }

private:
    TArray<ArrayType> m_Values;

    const ArrayType* FindValue(const KeyType& key) const
    {
        int32_t l = 0;
        int32_t r = m_Values.GetNumElements() - 1;
        Predicate pred{};

        while (l <= r)
        {
            int32_t m = l + (r - l) / 2;

            // Check if x is present at mid
            if (m_Values[m].Key == key)
            {
                return &m_Values[m];
            }

            // If x greater, ignore left half
            if (pred(m_Values[m].Key, key))
            {
                l = m + 1;
            }

            // If x is smaller, ignore right half
            else
            {
                r = m - 1;
            }
        }

        // If we reach here, then element was not present
        return nullptr;
    }
};

struct DefaultHashFunctions
{
    uint64_t operator()(double element) const
    {
        return *((uint64_t*)&element);
    }

    uint64_t operator()(float element) const
    {
        return *((uint32_t*)&element);
    }

    uint64_t operator()(int32_t element) const
    {
        return element;
    }

    uint64_t operator()(const std::string& element) const
    {
        uint64_t n = std::hash<std::string>()(element);
        return static_cast<uint64_t>(n);
    }
};


template <typename MapType, typename KeyType, typename AllocatorType>
class TMapIterator
{
public:
    using SelfClass = TMapIterator<MapType, KeyType, AllocatorType>;
    using KeyValueType = MapType::KeyValueType;

    TMapIterator(MapType& map, const TArray<KeyType, AllocatorType>& keys, int32_t index) :
        m_Map{&map},
        m_Keys(&keys),
        m_Index(index)
    {
    }

    TMapIterator(const SelfClass&) = default;
    TMapIterator& operator=(const SelfClass&) = default;

    SelfClass operator++(int)
    {
        return SelfClass{*m_Map, *m_Keys, m_Index + 1};
    }

    SelfClass& operator++()
    {
        ++m_Index;
        return *this;
    }

    KeyValueType& operator*()
    {
        assert(m_Keys->IsValidIndex(m_Index));
        return *m_Map->FindKeyValuePair(m_Keys->operator[](m_Index));
    }

    const KeyValueType& operator*() const
    {
        assert(m_Keys->IsValidIndex(m_Index));
        return *m_Map->FindKeyValuePair(m_Keys->operator[](m_Index));
    }

    KeyValueType* operator->()
    {
        assert(m_Keys->IsValidIndex(m_Index));
        return m_Map->FindKeyValuePair(m_Keys->operator[](m_Index));
    }

    const KeyValueType* operator->() const
    {
        assert(m_Keys->IsValidIndex(m_Index));
        return m_Map->FindKeyValuePair(m_Keys->operator[](m_Index));
    }

    bool operator==(const SelfClass& other) const
    {
        return m_Index == other.m_Index;
    }

    bool operator!=(const SelfClass& other) const
    {
        return m_Index != other.m_Index;
    }

private:
    MapType* m_Map;
    const TArray<KeyType, AllocatorType>* m_Keys;
    int32_t m_Index{0};
};

template <typename MapType, typename KeyType, typename AllocatorType>
class TConstMapIterator
{
public:
    using SelfClass = TConstMapIterator<MapType, KeyType, AllocatorType>;
    using KeyValueType = MapType::KeyValueType;

    TConstMapIterator(MapType& map, const TArray<KeyType, AllocatorType>& keys, int32_t index) :
        m_Map{&map},
        m_Keys(&keys),
        m_Index(index)
    {
    }

    TConstMapIterator(const SelfClass&) = default;
    TConstMapIterator& operator=(const SelfClass&) = default;

    SelfClass operator++(int)
    {
        return SelfClass{*m_Map, *m_Keys, m_Index + 1};
    }

    SelfClass& operator++()
    {
        ++m_Index;
        return *this;
    }

    const KeyValueType& operator*() const
    {
        assert(m_Keys->IsValidIndex(m_Index));
        return *m_Map->FindKeyValuePair(m_Keys->operator[](m_Index));
    }

    const KeyValueType* operator->() const
    {
        assert(m_Keys->IsValidIndex(m_Index));
        return m_Map->FindKeyValuePair(m_Keys->operator[](m_Index));
    }

    bool operator==(const SelfClass& other) const
    {
        return m_Index == other.m_Index;
    }

    bool operator!=(const SelfClass& other) const
    {
        return m_Index != other.m_Index;
    }

private:
    MapType* m_Map;
    const TArray<KeyType, AllocatorType>* m_Keys;
    int32_t m_Index{0};
};

template <typename KeyType, typename ValueType, typename HashFunction = DefaultHashFunctions, typename EqualCompare = std::equal_to<KeyType>,
    typename AllocatorType = DefaultAllocator>
class TMap
{
    double MaxLoadFactor()
    {
        return 0.75;
    }

public:
    using KeyValueType = TKeyValue<KeyType, ValueType>;

    using SelfClass = TMap<KeyType, ValueType, HashFunction, EqualCompare, AllocatorType>;

    using Iterator = TMapIterator<SelfClass, KeyType, AllocatorType>;
    using ConstIterator = TConstMapIterator<const SelfClass, KeyType, AllocatorType>;

    TMap() = default;

    template <typename OtherAllocatorType, typename OtherHashFunction, typename OtherEqualComparator>
    TMap(const TMap<KeyType, ValueType, OtherHashFunction, OtherEqualComparator, OtherAllocatorType>& map)
    {
        Append(map);
    }

    template <typename OtherAllocatorType, typename OtherHashFunction, typename OtherEqualComparator>
    void Append(const TMap<KeyType, ValueType, OtherHashFunction, OtherEqualComparator, OtherAllocatorType>& map)
    {
        for (auto& [key, value] : map)
        {
            Insert(key, value);
        }
    }

    void Insert(const KeyType& key, const ValueType& value)
    {
        int32_t index{IndexNone};

        if (LookupIndex(key, index))
        {
            m_Buckets[index].Value = value;
        }
        else
        {
            if (m_Keys.GetNumElements() + 1 > MaxLoadFactor() * m_Buckets.GetNumElements())
            {
                Rehash();
            }

            uint64_t hash = m_HashFunction(key);
            int32_t pos = static_cast<int32_t>(hash % m_Buckets.GetNumAlloc());

            m_Buckets.SetIndex(TKeyValue<KeyType, ValueType>{key, value}, pos);
            m_Keys.Add(key);

            m_Hashes[pos] = hash;
        }
    }

    void Remove(const KeyType& key)
    {
        int32_t index;

        if (LookupIndex(key, index))
        {
            m_Hashes[index] = 0;
            m_Keys.Remove(key);
        }
    }

    Iterator begin()
    {
        return Iterator{*this, m_Keys, 0};
    }

    Iterator end()
    {
        return Iterator{*this, m_Keys, m_Keys.GetNumElements()};
    }

    ConstIterator begin() const
    {
        return ConstIterator{*this, m_Keys, 0};
    }

    ConstIterator end() const
    {
        return ConstIterator{*this, m_Keys, m_Keys.GetNumElements()};
    }

    /* Returns key value pair or nullptr if couldn't be find. Key property must not be changed */
    TKeyValue<KeyType, ValueType>* FindKeyValuePair(const KeyType& key)
    {
        uint64_t hash = m_HashFunction(key);
        int32_t pos = static_cast<int32_t>(hash % m_Buckets.GetNumAlloc());

        if (m_Hashes[pos] == hash && m_EqualCompare(key, m_Buckets[pos].Key))
        {
            return &m_Buckets[pos];
        }

        return nullptr;
    }

    /* Returns key value pair or nullptr if couldn't be find. Key property must not be changed */
    const TKeyValue<KeyType, ValueType>* FindKeyValuePair(const KeyType& key) const
    {
        uint64_t hash = m_HashFunction(key);
        int32_t pos = static_cast<int32_t>(hash % m_Buckets.GetNumAlloc());

        if (m_Hashes[pos] == hash && m_EqualCompare(key, m_Buckets[pos].Key))
        {
            return &m_Buckets[pos];
        }

        return nullptr;
    }

    ValueType* FindValue(const KeyType& key)
    {
        auto p = FindKeyValuePair(key);
        return p ? &p->Value : nullptr;
    }

    const ValueType* FindValue(const KeyType& key) const
    {
        auto p = FindKeyValuePair(key);
        return p ? &p->Value : nullptr;
    }

    const ValueType& operator[](const KeyType& key) const
    {
        auto valuePtr = FindValue(key);
        assert(valuePtr);
        return *valuePtr;
    }

    ValueType& operator[](const KeyType& key)
    {
        auto valuePtr = FindValue(key);
        assert(valuePtr);
        return *valuePtr;
    }

    int32_t GetNumElements() const
    {
        return m_Keys.GetNumElements();
    }

    template <typename OtherAllocatorType>
    void GetKeys(TArray<KeyType, OtherAllocatorType>& outKeys)
    {
        outKeys.Append(m_Keys);
    }

    void Clear()
    {
        for (auto& hash : m_Hashes)
        {
            hash = 0;
        }

        m_Keys.Empty();
    }

    bool Contains(const KeyType& key) const
    {
        return !!FindKeyValuePair(key);
    }

private:
    TArray<uint64_t, AllocatorType> m_Hashes;
    TArray<TKeyValue<KeyType, ValueType>, AllocatorType> m_Buckets;
    TArray<KeyType, AllocatorType> m_Keys;

    HashFunction m_HashFunction;
    EqualCompare m_EqualCompare;

private:
    bool LookupIndex(const KeyType& key, int32_t& outIndex) const
    {
        if (m_Buckets.IsEmpty() || m_Keys.IsEmpty())
        {
            return false;
        }

        uint64_t hash = m_HashFunction(key);
        int32_t pos = static_cast<int32_t>(hash % m_Buckets.GetNumAlloc());

        if (m_Hashes[pos] == 0)
        {
            return false;
        }

        if (m_Hashes[pos] == hash && m_EqualCompare(key, m_Buckets[pos].Key))
        {
            outIndex = pos;
            return true;
        }

        return false;
    }

    void Rehash()
    {
        TArray<uint64_t, AllocatorType> newHashes;
        TArray<TKeyValue<KeyType, ValueType>, AllocatorType> newBuckets;

        if (m_Buckets.IsEmpty())
        {
            newHashes.AddZeroed(16);
            newBuckets.AddZeroed(16);
        }
        else
        {
            newHashes.AddZeroed(m_Hashes.GetNumElements());
            newBuckets.AddZeroed(m_Buckets.GetNumElements());
        }

        for (auto& v : this->m_Buckets)
        {
            uint64_t hash = m_HashFunction(v.Key);
            int32_t pos = static_cast<int32_t>(hash % newBuckets.GetNumAlloc());

            newHashes[pos] = hash;
            newBuckets[pos] = v;
        }

        m_Hashes = std::move(newHashes);
        m_Buckets = std::move(newBuckets);
    }
};

