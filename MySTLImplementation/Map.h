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
    KeyType key;
    ValueType value;

    TKeyValue() = default;

    using SelfClass = TKeyValue<KeyType, ValueType>;

    template <typename ...Args>
    TKeyValue(const KeyType& key, Args&& ...args) :
        key{key},
        value{std::forward<Args>(args)...}
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
        values.EmplaceBack(key, std::move(ValueType(std::forward<Args>(args)...)));

        values.Sort([](const ArrayType& a, const ArrayType& b)
        {
            Predicate pd{};
            return pd(a.key, b.key);
        });
    }

    const ValueType* Find(const KeyType& key) const
    {
        const ArrayType* v = FindValue(key);
        return v ? &v->value : nullptr;
    }

    ValueType* Find(const KeyType& key)
    {
        const ArrayType* v = FindValue(key);
        return v ? const_cast<ValueType*>(&v->value) : nullptr;
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
        return values.GetNumElements();
    }

    auto begin() const
    {
        return values.begin();
    }

    auto end() const
    {
        return values.end();
    }

    TArray<KeyType> GetKeys() const
    {
        TArray<KeyType> keys;
        keys.AllocAbs(values.GetNumElements());

        for (const ArrayType& v : values)
        {
            keys.Add(v.key);
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
        values.Clear();
    }

    void Remove(const KeyType& key)
    {
        const ArrayType* m = FindValue(key);
        int32_t index = static_cast<int32_t>(m - values.GetData());
        values.RemoveIndex(index);
    }

private:
    TArray<ArrayType> values;

    const ArrayType* FindValue(const KeyType& key) const
    {
        int32_t l = 0;
        int32_t r = values.GetNumElements() - 1;
        Predicate pred{};

        while (l <= r)
        {
            int32_t m = l + (r - l) / 2;

            // Check if x is present at mid
            if (values[m].Key == key)
            {
                return &values[m];
            }

            // If x greater, ignore left half
            if (pred(values[m].Key, key))
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

    uint64_t operator()(const String& str) const
    {
        return str.GetHashCode();
    }
};


template <typename KeyType, typename ValueType>
struct THashMapElement
{
    TKeyValue<KeyType, ValueType> keyValue;
    int32_t nextIndex{IndexNone};
    int32_t prevIndex{IndexNone};
};

template <typename MapType, typename KeyType, typename AllocatorType>
class TMapIterator
{
public:
    using SelfClass = TMapIterator<MapType, KeyType, AllocatorType>;
    using KeyValueType = MapType::KeyValueType;

    TMapIterator(MapType& map, const TArray<KeyType, AllocatorType>& keys, int32_t index) :
        map{&map},
        keys(&keys),
        index(index)
    {
    }

    TMapIterator(const SelfClass&) = default;
    TMapIterator& operator=(const SelfClass&) = default;

    SelfClass operator++(int)
    {
        return SelfClass{*map, *keys, index + 1};
    }

    SelfClass& operator++()
    {
        ++index;
        return *this;
    }

    KeyValueType& operator*()
    {
        assert(keys->IsValidIndex(index));
        return *map->FindKeyValuePair(keys->operator[](index));
    }

    const KeyValueType& operator*() const
    {
        assert(keys->IsValidIndex(index));
        return *map->FindKeyValuePair(keys->operator[](index));
    }

    KeyValueType* operator->()
    {
        assert(keys->IsValidIndex(index));
        return map->FindKeyValuePair(keys->operator[](index));
    }

    const KeyValueType* operator->() const
    {
        assert(keys->IsValidIndex(index));
        return map->FindKeyValuePair(keys->operator[](index));
    }

    bool operator==(const SelfClass& other) const
    {
        return index == other.index;
    }

    bool operator!=(const SelfClass& other) const
    {
        return index != other.index;
    }

private:
    MapType* map;
    const TArray<KeyType, AllocatorType>* keys;
    int32_t index{0};
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
    using ConstIterator = TMapIterator<const SelfClass, KeyType, AllocatorType>;

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
            buckets[index].value = value;
        }
        else
        {
            if (keys.GetNumElements() + 1 > MaxLoadFactor() * buckets.GetNumElements())
            {
                Rehash();
            }

            uint64_t hash = hashFunction(key);
            int32_t pos = static_cast<int32_t>(hash % buckets.GetNumAlloc());

            buckets.SetIndex(TKeyValue<KeyType, ValueType>{key, value}, pos);
            keys.Add(key);

            hashes[pos] = hash;
        }
    }

    void Remove(const KeyType& key)
    {
        int32_t index;

        if (LookupIndex(key, index))
        {
            hashes[index] = 0;
            keys.Remove(key);
        }
    }

    Iterator begin()
    {
        return Iterator{*this, keys, 0};
    }

    Iterator end()
    {
        return Iterator{*this, keys, keys.GetNumElements()};
    }

    ConstIterator begin() const
    {
        return ConstIterator{*this, keys, 0};
    }

    ConstIterator end() const
    {
        return ConstIterator{*this, keys, keys.GetNumElements()};
    }

    /* Returns key value pair or nullptr if couldn't be find. Key property must not be changed */
    TKeyValue<KeyType, ValueType>* FindKeyValuePair(const KeyType& key)
    {
        uint64_t hash = hashFunction(key);
        int32_t pos = static_cast<int32_t>(hash % buckets.GetNumAlloc());

        if (hashes[pos] == hash && equalCompare(key, buckets[pos].Key))
        {
            return &buckets[pos];
        }

        return nullptr;
    }

    /* Returns key value pair or nullptr if couldn't be find. Key property must not be changed */
    const TKeyValue<KeyType, ValueType>* FindKeyValuePair(const KeyType& key) const
    {
        uint64_t hash = hashFunction(key);
        int32_t pos = static_cast<int32_t>(hash % buckets.GetNumAlloc());

        if (hashes[pos] == hash && equalCompare(key, buckets[pos].Key))
        {
            return &buckets[pos];
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
        return keys.GetNumElements();
    }

    template <typename OtherAllocatorType>
    void GetKeys(TArray<KeyType, OtherAllocatorType>& outKeys)
    {
        outKeys.Append(keys);
    }

    void Clear()
    {
        for (auto& hash : hashes)
        {
            hash = 0;
        }

        keys.Empty();
    }

private:
    TArray<uint64_t, AllocatorType> hashes;
    TArray<TKeyValue<KeyType, ValueType>, AllocatorType> buckets;
    TArray<KeyType, AllocatorType> keys;

    HashFunction hashFunction;
    EqualCompare equalCompare;

private:
    bool LookupIndex(const KeyType& key, int32_t& outIndex) const
    {
        if (buckets.IsEmpty() || keys.IsEmpty())
        {
            return false;
        }

        uint64_t hash = hashFunction(key);
        int32_t pos = static_cast<int32_t>(hash % buckets.GetNumAlloc());

        if (hashes[pos] == 0)
        {
            return false;
        }

        if (hashes[pos] == hash && equalCompare(key, buckets[pos].key))
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

        if (buckets.IsEmpty())
        {
            newHashes.AddZeroed(16);
            newBuckets.AddZeroed(16);
        }
        else
        {
            newHashes.AddZeroed(hashes.GetNumElements() * 2);
            newBuckets.AddZeroed(buckets.GetNumElements() * 2);
        }

        for (auto& v : this->buckets)
        {
            uint64_t hash = hashFunction(v.key);
            int32_t pos = static_cast<int32_t>(hash % newBuckets.GetNumAlloc());

            newHashes[pos] = hash;
            newBuckets[pos] = v;
        }

        hashes = std::move(newHashes);
        buckets = std::move(newBuckets);
    }
};

