#pragma once

#include "Array.h"

#include <cstring>
#include <cstdlib>

#include <locale>
#include <codecvt>

#include <iostream>
#include <string_view>

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

    explicit String(std::string_view str)
    {
        CopyFrom(str.data(), (int32_t)str.length());
    }

    String& operator=(const char* str)
    {
        CopyFrom(str, (int32_t)strlen(str));
        return *this;
    }

    void Append(const char* str, int32_t length);
    String Substring(int32_t startOffset, int32_t length) const;
    String Substring(int32_t startOffset) const;

    int32_t Find(const String& str, int32_t startOffset = 0) const;
    int32_t Find(const char* str, int32_t startOffset = 0) const;
    int32_t Find(std::string_view str, int32_t startOffset = 0) const;

    int32_t RFind(const String& str, int32_t endOffset = 0) const;
    int32_t RFind(const char* str, int32_t endOffset = 0) const;
    int32_t RFind(std::string_view str, int32_t endOffset = 0) const;

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
    int32_t Compare(std::string_view other) const;

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
    int32_t FindFirstOf(std::string_view str, int32_t startpos = 0) const;

    int32_t FindFirstNotOf(const char* str, int32_t startpos = 0) const;
    int32_t FindFirstNotOf(std::string_view str, int32_t startpos = 0) const;

    int32_t FindLastOf(const char* str, int32_t lastIndex = 0) const;
    int32_t FindLastOf(std::string_view str, int32_t lastIndex = 0) const;

    int32_t FindNotLastOf(const char* str, int32_t lastIndex = 0) const;
    int32_t FindNotLastOf(std::string_view str, int32_t lastIndex = 0) const;
    
    char operator[](int32_t index) const
    {
        assert(index >= 0 && index < m_Data.GetNumElements() - 1);
        return m_Data[index];
    }

    char& operator[](int32_t index)
    {
        assert(index >= 0 && index < m_Data.GetNumElements() - 1);
        return m_Data[index];
    }

    void Split(const char* delimiter, TArray<String>& tokens) const;

    bool IsEmpty() const
    {
        return m_Data.GetNumElements() == 1 || m_Data.IsEmpty();
    }

private:
    CharContainer m_Data;

private:
    void CopyFrom(const char* str, int32_t length);
};

inline std::ostream& operator<<(std::ostream& stream, const String& s)
{
    return s.operator<<(stream);
}