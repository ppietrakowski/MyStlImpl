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

int32_t String::Find(const String& str, int32_t startOffset) const
{
    return FindImpl(str.m_Data.GetData(), str.m_Data.GetNumElements(), startOffset);
}

int32_t String::Find(const char* str, int32_t startOffset) const
{
    return FindImpl(str, (int32_t)std::char_traits<char>::length(str), startOffset);
}

int32_t String::RFind(const String& str, int32_t endOffset) const
{
    return RFindImpl(str.m_Data.GetData(), str.m_Data.GetNumElements(), endOffset);
}

int32_t String::RFind(const char* str, int32_t endOffset) const
{
    return RFindImpl(str, (int32_t)std::char_traits<char>::length(str), endOffset);
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