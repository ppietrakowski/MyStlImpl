#pragma once

#include "Array.h"

#include <cstring>
#include <cstdlib>

#include <locale>
#include <codecvt>

#include <iostream>

template <typename T>
struct TCharTraits
{
    static int32_t GetLength(const T* str)
    {
        int32_t length = 0;

        for (const T* i = str; *i != 0; ++i)
        {
            ++length;
        }

        return length;
    }

    static void Copy(T* destination, const T* src, int32_t srcLength)
    {
        for (int32_t i = 0; i < srcLength && *src != 0; ++i)
        {
            *destination++ = *src++;
        }
    }

    template <typename PredicateType>
    static const T* Find(const T* begin, const T* end, PredicateType&& predicate)
    {
        for (const T* i = begin; i != end; ++i)
        {
            if (predicate(*i))
            {
                return i;
            }
        }

        return nullptr;
    }

    template <typename PredicateType>
    static const T* FindReverse(const T* begin, const T* end, PredicateType&& predicate)
    {
        for (const T* i = end -1; i != begin; --i)
        {
            if (predicate(*i))
            {
                return i;
            }
        }

        return nullptr;
    }

    static int32_t Compare(const T* a, const T* b, int32_t count)
    {
        return memcmp(a, b, count * sizeof(T));
    }
};

class String
{
public:
    using CharContainer = TArray<char>;
    typedef TCharTraits<char> CharTraits;

    String() = default;

    String(const String& str)
    {
        m_Data.Empty();
        m_Data.Append(str.m_Data);
    }

    String& operator=(const String& str)
    {
        m_Data.Empty();
        m_Data.Append(str.m_Data);

        return *this;
    }

    String(String&& str) noexcept:
        m_Data(std::move(str.m_Data))
    {
    }

    String& operator=(String&& str) noexcept
    {
        m_Data = std::move(str.m_Data);
        return *this;
    }

    String(const char* str, int32_t length = -1)
    {
        if (length == -1)
        {
            length = (int32_t)strlen(str);
        }

        CopyFrom(str, length);
    }

    String& operator=(const char* str)
    {
        CopyFrom(str, (int32_t)strlen(str));
        return *this;
    }

    void Append(const char* str, int32_t length);
    String Substring(int32_t startOffset, int32_t length) const;

    int32_t Find(const String& str, int32_t startOffset = 0) const;
    int32_t Find(const char* str, int32_t startOffset = 0) const;

    int32_t RFind(const String& str, int32_t endOffset = 0) const;
    int32_t RFind(const char* str, int32_t endOffset = 0) const;

    int32_t GetLength() const
    {
        return m_Data.GetNumElements() - 1;
    }

    uint64_t GetHashCode() const;

    const char* GetData() const
    {
        return m_Data.GetData();
    }

    void Clear();

    std::ostream& operator<<(std::ostream& stream) const;

    static String Printf(const char* format, ...);
    static String VPrintf(const char* format, va_list list);

    int32_t Compare(const String& other) const;
    int32_t Compare(const char* other) const;

    bool operator==(const String& other) const
    {
        return Compare(other) == 0;
    }

    bool operator==(const char* other) const
    {
        return Compare(other) == 0;
    }

    bool operator<(const String& other) const
    {
        return Compare(other) < 0;
    }

    bool operator<(const char* other) const
    {
        return Compare(other) < 0;
    }

    bool operator>(const String& other) const
    {
        return Compare(other) > 0;
    }

    bool operator>(const char* other) const
    {
        return Compare(other) > 0;
    }
    
    int32_t FindFirstOf(const char* str, int32_t startpos = 0) const;
    int32_t FindFirstNotOf(const char* str, int32_t startpos = 0) const;
    int32_t FindLastOf(const char* str, int32_t lastIndex = 0) const;
    int32_t FindNotLastOf(const char* str, int32_t lastIndex = 0) const;
    
    char operator[](int32_t index) const
    {
        return m_Data[index];
    }

    char& operator[](int32_t index)
    {
        assert(index >= 0 & m_Data.GetNumElements() - 1);
        return m_Data[index];
    }

private:
    CharContainer m_Data;

private:
    void CopyFrom(const char* str, int32_t length);

    template <typename T>
    int32_t FindImpl(const T* str, int32_t srcLen, int32_t startOffset) const
    {
        if (startOffset < 0)
        {
            return IndexNone;
        }

        const int32_t len = m_Data.GetNumElements();

        if (srcLen == 0 || len == 0)
        {
            return IndexNone; // won't find anything!
        }

        const char* src = m_Data.GetData();

        for (int32_t i = startOffset; i <= (len - srcLen); i++)
        {
            bool bFound = true;
            for (int32_t j = 0; j < srcLen; j++)
            {
                int32_t readPos = i + j;

                if (readPos >= len)
                {
                    return IndexNone;
                }

                if (src[readPos] != str[j])
                {
                    bFound = false;
                    break;
                }
            }

            if (bFound)
            {
                return i;
            }
        }

        return IndexNone;
    }

    template <typename T>
    int32_t RFindImpl(const T* str, int32_t srcLen, int32_t endOffset) const
    {
        // establish a limit
        int32_t limit = m_Data.GetNumElements() - srcLen;

        if (limit < 0)
        {
            return IndexNone;
        }

        // establish a starting point
        if (endOffset < 0)
        {
            endOffset = limit;
        }
        else if (endOffset > limit)
        {
            endOffset = limit;
        }

        int32_t len = m_Data.GetNumElements();

        if (srcLen == 0 || len == 0)
        {
            return -1; // won't find anything!
        }

        const char* src = m_Data.GetData();

        for (int32_t i = endOffset; i >= 0; i--)
        {
            bool bFound = true;
            for (int32_t j = 0; j < srcLen; j++)
            {
                int32_t readPos = i + j;

                if (readPos >= len)
                {
                    return IndexNone;
                }

                if (src[readPos] != str[j])
                {
                    bFound = false;
                    break;
                }
            }

            if (bFound)
            {
                return i;
            }
        }

        return IndexNone;
    }
};


inline std::ostream& operator<<(std::ostream& stream, const String& s)
{
    return s.operator<<(stream);
}