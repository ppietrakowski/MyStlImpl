#include "String.h"

#include <cstdio>
#include <math.h> 
#include <stdio.h> 
#include <cstdarg>

void String::Append(const char* str, int32_t length)
{
    CopyFrom(str, length);
}

String String::Substring(int32_t startOffset, int32_t length) const
{
    int32_t l = m_Data.GetNumElements() - startOffset - length;
    if (l < 0)
    {
        return String{};
    }

    return String{m_Data.GetData() + startOffset, length};
}

String String::Substring(int32_t startOffset) const
{
    int32_t l = m_Data.GetNumElements() - startOffset;
    if (l < 0)
    {
        return String{};
    }

    return String{m_Data.GetData() + startOffset, m_Data.GetNumElements() - startOffset};
}

static int32_t FindImplementation(const char* str, int32_t strLength, int32_t startOffset, TSpan<const char> searchedArray)
{
    if (startOffset < 0)
    {
        return IndexNone;
    }

    const int32_t len = searchedArray.GetNumElements();

    if (strLength == 0 || len == 0)
    {
        return IndexNone; // won't find anything!
    }

    for (int32_t i = startOffset; i <= (len - strLength); i++)
    {
        bool found = true;
        for (int32_t j = 0; j < strLength; j++)
        {
            int32_t readPos = i + j;

            if (readPos >= len)
            {
                return IndexNone;
            }

            if (searchedArray[readPos] != str[j])
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            return i;
        }
    }

    return IndexNone;
}

int32_t String::Find(const String& str, int32_t startOffset) const
{
    return FindImplementation(str.GetData(), str.GetLength(), startOffset, m_Data);
}

int32_t String::Find(const char* str, int32_t startOffset) const
{
    return FindImplementation(str, CharTraits::GetLength(str), startOffset, m_Data);
}

static int32_t RFindImplementation(const char* str, TSpan<const char> data, int32_t srcLen, int32_t endOffset)
{
    // establish a limit
    int32_t limit = data.GetNumElements() - srcLen;

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

    int32_t len = data.GetNumElements();

    if (srcLen == 0 || len == 0)
    {
        return -1; // won't find anything!
    }

    for (int32_t i = endOffset; i >= 0; i--)
    {
        bool found = true;
        for (int32_t j = 0; j < srcLen; j++)
        {
            int32_t readPos = i + j;

            if (readPos >= len)
            {
                return IndexNone;
            }

            if (data[readPos] != str[j])
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            return i;
        }
    }

    return IndexNone;
}

int32_t String::RFind(const String& str, int32_t endOffset) const
{
    return RFindImplementation(str.GetData(), m_Data, str.GetLength(), endOffset);
}

int32_t String::RFind(const char* str, int32_t endOffset) const
{
    int32_t srcLen = CharTraits::GetLength(str);
    return RFindImplementation(str, m_Data, srcLen, endOffset);
}

uint64_t String::GetHashCode() const
{
    /* simple djb2 hashing */
    const char* it = m_Data.GetData();
    uint64_t hashv = 5381;
    uint64_t c = *it++;

    while (c)
    {
        hashv = ((hashv << 5) + hashv) + c; /* hash * 33 + c */
        c = *it++;
    }

    return hashv;
}

#define MAX_PRINTF_BUFFER 4096

struct VaListAutoDeleter
{
    va_list List;

    ~VaListAutoDeleter() noexcept
    {
        va_end(List);
    }
};

void String::Clear()
{
    if (!m_Data.IsEmpty())
    {
        m_Data[0] = '\0';
        m_Data.Empty();
    }
}

std::ostream& String::operator<<(std::ostream& stream) const
{
    stream << m_Data.GetData();
    return stream;
}

String String::Printf(const char* format, ...)
{
    VaListAutoDeleter deleter{};

    va_start(deleter.List, format);
    return VPrintf(format, deleter.List);
}

String String::VPrintf(const char* format, va_list list)
{
    char data[MAX_PRINTF_BUFFER];

    int32_t length = vsnprintf(data, MAX_PRINTF_BUFFER - 1, format, list);
    assert(length > 0);

    data[MAX_PRINTF_BUFFER - 1] = 0;
    return String{data, length};
}

int32_t String::Compare(const String& other) const
{
    int32_t length = std::min(other.GetLength(), GetLength());
    return std::char_traits<char>::compare(m_Data.GetData(), other.GetData(), length);
}

int32_t String::Compare(const char* other) const
{
    size_t len = strlen(other);
    int32_t length = std::min<int32_t>((int32_t)len, GetLength());
    return std::char_traits<char>::compare(m_Data.GetData(), other, length);
}

int32_t String::FindFirstOf(const char* str, int32_t startpos) const
{
    if (startpos == IndexNone)
    {
        return -1;
    }

    auto i = CharTraits::Find(m_Data.UncheckedBegin() + startpos, m_Data.UncheckedEnd(),
        [str](const char c)
    {
        for (const char* it = str; *it; ++it)
        {
            if (*it == c)
            {
                return true;
            }
        }

        return false;
    });

    if (!i)
    {
        return IndexNone;
    }

    return static_cast<int32_t>(i - m_Data.UncheckedBegin());
}

int32_t String::FindFirstNotOf(const char* str, int32_t startpos) const
{
    if (startpos == IndexNone)
    {
        return -1;
    }

    auto i = CharTraits::Find(m_Data.UncheckedBegin() + startpos, m_Data.UncheckedEnd(),
        [str](const char c)
    {
        for (const char* it = str; *it; ++it)
        {
            if (*it != c)
            {
                return true;
            }
        }

        return false;
    });

    if (!i)
    {
        return IndexNone;
    }

    return static_cast<int32_t>(i - m_Data.UncheckedBegin());
}

int32_t String::FindLastOf(const char* str, int32_t lastIndex) const
{
    auto i = CharTraits::FindReverse(m_Data.UncheckedBegin(), m_Data.UncheckedEnd() - 1 - lastIndex,
        [str](const char c)
    {
        for (const char* it = str; *it; ++it)
        {
            if (*it == c)
            {
                return true;
            }
        }

        return false;
    });

    if (!i)
    {
        return IndexNone;
    }

    return static_cast<int32_t>(i - m_Data.UncheckedBegin());
}

int32_t String::FindNotLastOf(const char* str, int32_t lastIndex) const
{
    auto i = CharTraits::FindReverse(m_Data.UncheckedBegin(), m_Data.UncheckedEnd() - 2 - lastIndex,
        [str](const char c)
    {
        for (const char* it = str; *it; ++it)
        {
            if (*it != c)
            {
                return true;
            }
        }

        return false;
    });

    if (!i)
    {
        return IndexNone;
    }

    return static_cast<int32_t>(i - m_Data.UncheckedBegin());
}

void String::Split(const char* delimiter, TArray<String>& tokens) const
{
    int32_t lastPos = FindFirstNotOf(delimiter, 0);

    // Find first "non-delimiter".
    int32_t pos = FindFirstOf(delimiter, lastPos);

    tokens.AllocAbs(4);

    while (pos != IndexNone || lastPos != IndexNone)
    {
        // Found a token, add it to the vector.
        String tmp = Substring(lastPos, pos - lastPos);
        if (!tmp.IsEmpty())
        {
            tokens.Add(tmp);
        }

        if (pos == IndexNone)
        {
            tokens.Add(Substring(lastPos));
        }

        // Skip delimiters.  Note the "not_of"
        lastPos = FindFirstNotOf(delimiter, pos);

        // Find next "non-delimiter"
        pos = FindFirstOf(delimiter, lastPos);
    }
}

void String::CopyFrom(const char* str, int32_t length)
{
    assert(str != nullptr);

    if (!m_Data.IsEmpty())
    {
        m_Data.RemoveIndex(m_Data.GetNumElements() - 1);
    }

    m_Data.AllocAbs(m_Data.GetNumElements() + length);

    for (int32_t i = 0; i < length; ++i)
    {
        m_Data.Add(str[i]);
    }

    m_Data.Add('\0');
}