#pragma once

#include "Array.h"

#include <cstring>
#include <cstdlib>

#include <locale>
#include <codecvt>

#include <iostream>


class String
{
public:
    using CharContainer = TArray<char>;

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

    String(String&&) noexcept = default;
    String& operator=(String&&) noexcept = default;

    String(const char* str, int32_t length = -1)
    {
        if (length == -1)
        {
            length = (int32_t)strlen(str);
        }

        CopyFrom(str, length);
    }

    void Append(const char* str, int32_t length);
    String Substring(int32_t startOffset, int32_t length) const;

    int32_t Find(const String& str, int32_t startOffset = 0) const;
    int32_t Find(const char* str, int32_t startOffset = 0) const;

    int32_t RFind(const String& str, int32_t endOffset = 0) const;
    int32_t RFind(const char* str, int32_t endOffset = 0) const;

    int32_t GetLength() const
    {
        return m_Data.GetNumElements();
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