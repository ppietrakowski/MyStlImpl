#pragma once

#include <utility>
#include <concepts>
#include <algorithm>

constexpr inline int32_t IndexNone = -1;

class Arrays
{
public:
    template <typename ElementType>
    static int32_t Find(const ElementType* begin, const ElementType* end, const ElementType& element)
    {
        for (const ElementType* i = begin; i != end; ++i)
        {
            if (*i == element)
            {
                return static_cast<int32_t>(i - begin);
            }
        }

        return IndexNone;
    }

    template <typename ElementType, std::predicate<const ElementType> Predicate>
    static int32_t FindPredicate(const ElementType* begin, const ElementType* end, Predicate&& predicate)
    {
        for (const ElementType* i = begin; i != end; ++i)
        {
            if (predicate(*i))
            {
                return static_cast<int32_t>(i - begin);
            }
        }

        return IndexNone;
    }

    template <typename ElementType, typename Compare>
    static void Sort(ElementType* begin, ElementType* end, Compare&& comparator)
    {
        std::sort(begin, end, std::forward<Compare>(comparator));
    }
};

