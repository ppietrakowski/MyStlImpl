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
    int32_t l = data.GetNumElements() - startOffset - length;
    if (l < 0)
    {
        return String{};
    }

    return String{data.GetData() + startOffset, length};
}

int32_t String::Find(const String& str, int32_t startOffset) const
{
    return FindImpl(str.data.GetData(), str.data.GetNumElements(), startOffset);
}

int32_t String::Find(const char* str, int32_t startOffset) const
{
    return FindImpl(str, (int32_t)std::char_traits<char>::length(str), startOffset);
}

int32_t String::RFind(const String& str, int32_t endOffset) const
{
    return RFindImpl(str.data.GetData(), str.data.GetNumElements(), endOffset);
}

int32_t String::RFind(const char* str, int32_t endOffset) const
{
    return RFindImpl(str, (int32_t)std::char_traits<char>::length(str), endOffset);
}

uint64_t String::GetHashCode() const
{
    /* simple djb2 hashing */
    const char* it = data.GetData();
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
    if (!data.IsEmpty())
    {
        data[0] = '\0';
        data.Empty();
    }
}

std::ostream& String::operator<<(std::ostream& stream) const
{
    stream << data.GetData();
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
    return std::char_traits<char>::compare(data.GetData(), other.GetData(), length);
}

int32_t String::Compare(const char* other) const
{
    size_t len = strlen(other);
    int32_t length = std::min<int32_t>((int32_t)len, GetLength());
    return std::char_traits<char>::compare(data.GetData(), other, length);
}

void String::CopyFrom(const char* str, int32_t length)
{
    assert(str != nullptr);

    if (!data.IsEmpty())
    {
        data.RemoveIndex(data.GetNumElements() - 1);
    }

    data.AllocAbs(data.GetNumElements() + length);

    for (int32_t i = 0; i < length; ++i)
    {
        data.Add(str[i]);
    }

    data.Add('\0');
}