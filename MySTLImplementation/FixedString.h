#pragma once

#include <cstring>
#include <cstdint>
#include <numeric>

#include <stdexcept>
#include <cstdarg>

#include <cassert>

#include "Array.h"

/*
* Wrapper around stack allocated char array
* This class exposes interface like std::string
* Should be use in place, where allocations would be a much of overhead
* Also, for easier usage, this class contains tokenize function
* and sprintf/back_sprintf functions to sprintf like functionality
*/
template <int32_t N>
class CString
{
public:
    CString();
    CString(const CString<N>& str);

    CString(const char* str);
    CString(const std::string& str);
    CString(const std::string_view str);

    CString& Append(const char* str);
    CString& Append(const char* str, int32_t n);
    CString& Append(const char* str, int32_t offset, int32_t n);

    CString& Append(const std::string& str);
    CString& Append(const CString<N>& str);
    CString& Append(int32_t value);
    CString& Append(uint32_t value);

    CString& Append(int64_t value);
    CString& Append(uint64_t value);
    CString& Append(float value);
    CString& Append(double value);

    void Copy(char* buffer, int32_t maxBufferSize) const;
    void Clear();

    int32_t GetNumElements() const;
    constexpr int32_t GetCapacity() const;
    int32_t GetLength() const;

    const char* begin() const;
    const char* end() const;

    char* begin();
    char* end();

    CString& operator=(const CString<N>& str);

    CString& operator=(const char* str);
    CString& operator=(const std::string& str);

    char& operator[](int32_t i);
    const char& operator[](int32_t i) const;

    const char* c_str() const;
    char* GetData();
    const char* GetData() const;

    CString<N> Substring(int32_t offset, int32_t n = IndexNone) const;
    bool IsEmpty() const;

    int32_t Find(const char* str) const;
    int32_t Find(const std::string& str) const;
    int32_t Find(const CString<N>& str) const;

    int32_t RFind(const char* str) const;
    int32_t RFind(const std::string& str) const;
    int32_t RFind(const CString<N>& str) const;

    int32_t FindFirstOf(const char* str) const;
    int32_t FindFirstOf(const std::string& str) const;
    int32_t FindFirstOf(const CString<N>& str) const;

    int32_t FindFirstNotOf(const char* str) const;
    int32_t FindFirstNotOf(const std::string& str) const;
    int32_t FindFirstNotOf(const CString<N>& str) const;

    int32_t FindLastOf(const char* str) const;
    int32_t FindLastOf(const std::string& str) const;
    int32_t FindLastOf(const CString<N>& str) const;

    int32_t FindNotLastOf(const char* str) const;
    int32_t FindNotLastOf(const std::string& str) const;
    int32_t FindNotLastOf(const CString<N>& str) const;

    void Split(const char* delimiter, TArray<CString<N>>& outTokens) const;

    void PopBack();
    void PushBack(char c);

    int32_t Compare(const char* other) const;
    int32_t Compare(const std::string& other) const;
    int32_t Compare(const CString<N>& other) const;

    char& Back() const
    {
        assert(m_Length > 0); return m_String[m_Length - 1];
    }
    char& Front() const
    {
        return m_String[0];
    }

    operator std::string() const
    {
        return std::string(&m_String[0]);
    }
    operator std::string_view() const
    {
        return std::string_view(&m_String[0]);
    }

    bool operator==(const char* str) const;
    bool operator==(const CString<N>& str) const;
    bool operator==(const std::string& str) const;

    void Sprintf(const char* format, ...);
    void VSprintf(const char* format, va_list list);

    void BackSprintf(const char* format, ...);
    void VBackSprintf(const char* format, va_list list);

    CString<N> operator+(const CString<N>& other) const;
    CString<N> operator+(const char* other) const;
    CString<N> operator+(int32_t other) const;

    CString<N> operator+(int64_t other) const;
    CString<N> operator+(uint64_t other) const;

    CString<N> operator+(float other) const;
    CString<N> operator+(double other) const;
    CString<N> operator+(const std::string& other) const;

    CString<N>& operator+=(const std::string& other);
    CString<N>& operator+=(const CString<N>& other);
    CString<N>& operator+=(const char* other);

    CString<N>& operator+=(int32_t other);
    CString<N>& operator+=(int64_t other);

    CString<N>& operator+=(uint64_t other);
    CString<N>& operator+=(float other);
    CString<N>& operator+=(double other);

    void Swap(CString<N>& other);

    uint64_t GetHashCode() const
    {
        /* simple djb2 hashing */
        const char* it = m_String;
        uint64_t hashv = 5381;
        uint64_t c = *it++;

        while (c)
        {
            hashv = ((hashv << 5) + hashv) + c; /* hash * 33 + c */
            c = *it++;
        }

        return hashv;
    }

private:
    char m_String[N + 1];
    int32_t m_Length;
};

namespace std
{
    template <int32_t N>
    struct hash<CString<N>>
    {
        int32_t operator()(const CString<N>& _Keyval) const
        {
            return _Keyval.GetHashCode();
        }
    };

    template <int32_t N>
    void swap(CString<N>& a, CString<N>& b)
    {
        a.Swap(b);
    }

    template <int32_t N>
    std::istream& getline(std::istream& input, CString<N>& str, char delimiter = '\n')
    {
        std::string temp;
        ::std::getline(input, temp);
        str = CString<N>(temp);
        return input;
    }
}

namespace cstring
{
    char* LastSubstring(const char* haystack, const char* needle);
    char* FindLastOf(const char* str, const char* accept);
}

/* implementation */
template<int32_t N>
inline void CString<N>::Clear()
{
    memset(&m_String[0], 0, sizeof(m_String));
    m_Length = 0;
}

template<int32_t N>
inline int32_t CString<N>::GetNumElements() const
{
    return m_Length;
}

template<int32_t N>
inline constexpr int32_t CString<N>::GetCapacity() const
{
    return N;
}

template<int32_t N>
inline int32_t CString<N>::GetLength() const
{
    return m_Length;
}

template<int32_t N>
inline const char* CString<N>::begin() const
{
    return &m_String[0];
}

template<int32_t N>
inline const char* CString<N>::end() const
{
    return &m_String[m_Length];
}

template<int32_t N>
inline char* CString<N>::begin()
{
    return &m_String[0];
}

template<int32_t N>
inline char* CString<N>::end()
{
    return &m_String[m_Length];
}

template<int32_t N>
inline CString<N>& CString<N>::operator=(const CString<N>& str)
{
    if (this == &str)
    {
        return *this;
    }

    strncpy(m_String, str.m_String, N);

    // always ensure it contains trailing zero
    m_String[N] = 0;
    m_Length = (int32_t)strlen(m_String);
    return *this;
}

template<int32_t N>
inline CString<N>& CString<N>::operator=(const char* str)
{
    strncpy(m_String, str, N);

    // always ensure it contains trailing zero
    m_String[N] = 0;
    m_Length = strlen(m_String);
    return *this;
}

template<int32_t N>
inline CString<N>& CString<N>::operator=(const std::string& str)
{
    strncpy(m_String, str.c_str(), N);
    m_String[N] = 0;
    m_Length = strlen(m_String);
    return *this;
}

template<int32_t N>
inline char& CString<N>::operator[](int32_t i)
{
    assert(i < N && "CString out of range");
    return m_String[i];
}

template<int32_t N>
inline const char& CString<N>::operator[](int32_t i) const
{
    assert(i < N && "CString out of range");
    return m_String[i];
}

template<int32_t N>
inline const char* CString<N>::c_str() const
{
    return &m_String[0];
}

template<int32_t N>
inline char* CString<N>::GetData()
{
    return &m_String[0];
}

template<int32_t N>
inline const char* CString<N>::GetData() const
{
    return &m_String[0];
}

template<int32_t N>
inline CString<N> CString<N>::Substring(int32_t offset, int32_t n) const
{
    CString<N> str;
    strncpy(str.m_String, m_String + offset, n);
    str.m_String[N] = 0;
    str.m_Length = strlen(str.m_String);
    return str;
}

template<int32_t N>
inline bool CString<N>::IsEmpty() const
{
    return m_Length == 0;
}

template<int32_t N>
inline int32_t CString<N>::Find(const char* str) const
{
    if (m_Length == 0)
    {
        return IndexNone;
    }

    const char* foundAt = strstr(m_String, str);
    if (foundAt == nullptr)
    {
        return IndexNone;
    }

    return static_cast<int32_t>(foundAt - m_String);
}

template<int32_t N>
inline int32_t CString<N>::Find(const std::string& str) const
{
    return Find(str.c_str());
}

template<int32_t N>
inline int32_t CString<N>::Find(const CString<N>& str) const
{
    return Find(str.c_str());
}

template<int32_t N>
inline int32_t CString<N>::RFind(const char* str) const
{
    if (m_Length == 0)
    {
        return IndexNone;
    }

    const char* foundAt = cstring::LastSubstring(m_String, str);
    if (foundAt == nullptr)
    {
        return IndexNone;
    }

    return static_cast<int32_t>(foundAt - m_String);
}

template<int32_t N>
inline int32_t CString<N>::RFind(const std::string& str) const
{
    return RFind(str.c_str());
}

template<int32_t N>
inline int32_t CString<N>::RFind(const CString<N>& str) const
{
    return RFind(str.m_String);
}

template<int32_t N>
inline int32_t CString<N>::FindFirstOf(const char* str) const
{
    if (m_Length == 0)
    {
        return IndexNone;
    }

    const char* foundAt = strpbrk(m_String, str);
    if (foundAt == nullptr)
    {
        return IndexNone;
    }

    return static_cast<int32_t>(foundAt - m_String);
}

template<int32_t N>
inline int32_t CString<N>::FindFirstOf(const std::string& str) const
{
    return FindFirstOf(str.c_str());
}

template<int32_t N>
inline int32_t CString<N>::FindFirstOf(const CString<N>& str) const
{
    return FindFirstOf(str.m_String);
}

template<int32_t N>
inline int32_t CString<N>::FindFirstNotOf(const char* control) const
{
    const char* const baseControl = control;

    for (const char& c : m_String)
    {
        control = baseControl;
        while (control && *control)
        {
            if (c != *control)
            {
                int32_t posOfFound = static_cast<int32_t>(&c - &m_String[0]);
                return posOfFound;
            }
            control++;
        }
    }

    return IndexNone;
}

template<int32_t N>
inline int32_t CString<N>::FindFirstNotOf(const std::string& str) const
{
    return FindFirstNotOf(str.c_str());
}

template<int32_t N>
inline int32_t CString<N>::FindFirstNotOf(const CString<N>& str) const
{
    return FindFirstNotOf(str.m_String);
}

template<int32_t N>
inline int32_t CString<N>::FindLastOf(const char* str) const
{
    if (m_Length == 0)
    {
        return IndexNone;
    }

    const char* foundAt = cstring::FindLastOf(m_String, str);
    if (foundAt == nullptr)
    {
        return IndexNone;
    }

    return static_cast<int32_t>(foundAt - m_String);
}

template<int32_t N>
inline int32_t CString<N>::FindLastOf(const std::string& str) const
{
    return FindLastOf(str.c_str());
}

template<int32_t N>
inline int32_t CString<N>::FindLastOf(const CString<N>& str) const
{
    return FindLastOf(str.m_String);
}

template<int32_t N>
inline int32_t CString<N>::FindNotLastOf(const char* str) const
{
    const char* const baseStr = str;

    for (int64_t n = m_Length - 1; n >= 0; n--)
    {
        str = baseStr;

        const char& c = m_String[n];
        while (str && *str)
        {
            if (c != *str)
            {
                int32_t posOfFound = static_cast<int32_t>(&c - &m_String[0]);
                return posOfFound;
            }
            str++;
        }
    }
}

template<int32_t N>
inline int32_t CString<N>::FindNotLastOf(const std::string& str) const
{
    return FindNotLastOf(str.c_str());
}

template<int32_t N>
inline int32_t CString<N>::FindNotLastOf(const CString<N>& str) const
{
    return FindNotLastOf(str.m_String);
}

template<int32_t N>
inline void CString<N>::Split(const char* delimiter, TArray<CString<N>>& outTokens) const
{
    char buffer[N];
    std::memcpy(buffer, m_String, N);

    char* foundAt = strtok(buffer, delimiter);

    while (foundAt != nullptr)
    {
        outTokens.Add(foundAt);
        foundAt = strtok(nullptr, delimiter);
    }
}

template<int32_t N>
inline void CString<N>::PopBack()
{
    assert(m_Length > 0 && "Nothing to pop");
    m_Length -= 1;
    m_String[m_Length] = 0;
}

template<int32_t N>
inline void CString<N>::PushBack(char c)
{
    if (m_Length < N - 1)
    {
        m_String[m_Length++] = c;
    }
}

template<int32_t N>
inline int32_t CString<N>::Compare(const char* other) const
{
    return strcmp(m_String, other);
}

template<int32_t N>
inline int32_t CString<N>::Compare(const std::string& other) const
{
    return Compare(other.c_str());
}

template<int32_t N>
inline int32_t CString<N>::Compare(const CString<N>& other) const
{
    return Compare(other.m_String);
}

template<int32_t N>
inline bool CString<N>::operator==(const char* str) const
{
    return Compare(str) == 0;
}

template<int32_t N>
inline bool CString<N>::operator==(const CString<N>& str) const
{
    return Compare(str) == 0;
}

template<int32_t N>
inline bool CString<N>::operator==(const std::string& str) const
{
    return Compare(str) == 0;
}

template<int32_t N>
inline CString<N>::CString()
{
    memset(&m_String[0], 0, sizeof(m_String));
    m_Length = 0;
}

template<int32_t N>
inline CString<N>::CString(const CString<N>& str)
{
    *this = str;
}

template<int32_t N>
inline CString<N>::CString(const char* str)
{
    strncpy(m_String, str, N);
    m_String[N] = 0;
    m_Length = strlen(m_String);
}

template<int32_t N>
inline CString<N>::CString(const std::string& str)
{
    strncpy(m_String, str.c_str(), N);
    m_String[N] = 0;
    m_Length = strlen(m_String);
}

template<int32_t N>
inline CString<N>::CString(const std::string_view str) :
    CString<N>(str.data())
{
}

template<int32_t N>
inline CString<N>& CString<N>::Append(const char* str)
{
    return Append(str, strlen(str));
}

template<int32_t N>
inline CString<N>& CString<N>::Append(const char* str, int32_t n)
{
    assert(n <= strlen(str));

    if (m_Length + n >= N)
    {
        n = N - m_Length;
    }

    strncat(m_String + m_Length, str, n);
    m_String[N] = 0;
    m_Length = strlen(m_String);
    return *this;
}

template<int32_t N>
inline CString<N>& CString<N>::Append(const char* str, int32_t offset, int32_t n)
{
    if (n == IndexNone)
    {
        n = strlen(str);
    }

    if (m_Length + n >= N)
    {
        n = N - m_Length;
    }

    strncat(m_String + m_Length, str + offset, n);
    m_String[N] = 0;
    m_Length = strlen(m_String);
    return *this;
}

template<int32_t N>
inline CString<N>& CString<N>::Append(const std::string& str)
{
    return Append(str.c_str());
}

template<int32_t N>
inline CString<N>& CString<N>::Append(const CString<N>& str)
{
    return Append(str.m_String);
}

template<int32_t N>
inline CString<N>& CString<N>::Append(int32_t value)
{
    BackSprintf("%i", value);
    return *this;
}

template<int32_t N>
inline CString<N>& CString<N>::Append(uint32_t value)
{
    BackSprintf("%u", value);
    return *this;
}

template<int32_t N>
inline CString<N>& CString<N>::Append(int64_t value)
{
    BackSprintf("%ill", value);
    return *this;
}

template<int32_t N>
inline CString<N>& CString<N>::Append(uint64_t value)
{
    BackSprintf("%ull", value);
    return *this;
}

template<int32_t N>
inline CString<N>& CString<N>::Append(float value)
{
    BackSprintf("%f", value);
    return *this;
}

template<int32_t N>
inline CString<N>& CString<N>::Append(double value)
{
    BackSprintf("%f", value);
    return *this;
}

template<int32_t N>
inline void CString<N>::Copy(char* buffer, int32_t maxBufferSize) const
{
    if (maxBufferSize >= m_Length)
    {
        strcpy(buffer, m_String);
    }
    else
    {
        strncpy(buffer, m_String, maxBufferSize);
    }

    buffer[maxBufferSize - 1] = 0;
}

template<int32_t N>
inline void CString<N>::Sprintf(const char* format, ...)
{
    va_list list;

    va_start(list, format);
    this->VSprintf(format, list);
    va_end(list);
}

template<int32_t N>
inline void CString<N>::VSprintf(const char* format, va_list list)
{
    ::vsnprintf(m_String, N, format, list);
    m_String[N] = 0;
}

template<int32_t N>
inline void CString<N>::BackSprintf(const char* format, ...)
{
    va_list list;
    va_start(list, format);
    this->VBackSprintf(format, list);
    va_end(list);
}

template<int32_t N>
inline void CString<N>::VBackSprintf(const char* format, va_list list)
{
    CString<N> tempBuffer;
    ::vsnprintf(tempBuffer.m_String, N, format, list);
    Append(tempBuffer);
}

#define OVERLOAD_APPEND() CString<N> control{ *this }; control.Append(other); return control
template<int32_t N>
inline CString<N> CString<N>::operator+(const CString<N>& other) const
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N> CString<N>::operator+(const char* other) const
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N> CString<N>::operator+(int32_t other) const
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N> CString<N>::operator+(int64_t other) const
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N> CString<N>::operator+(uint64_t other) const
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N> CString<N>::operator+(float other) const
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N> CString<N>::operator+(double other) const
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N> CString<N>::operator+(const std::string& other) const
{
    OVERLOAD_APPEND();
}

#undef OVERLOAD_APPEND
#define OVERLOAD_APPEND() Append(other); return *this

template<int32_t N>
inline CString<N>& CString<N>::operator+=(const std::string& other)
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N>& CString<N>::operator+=(const CString<N>& other)
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N>& CString<N>::operator+=(const char* other)
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N>& CString<N>::operator+=(int32_t other)
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N>& CString<N>::operator+=(int64_t other)
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N>& CString<N>::operator+=(uint64_t other)
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N>& CString<N>::operator+=(float other)
{
    OVERLOAD_APPEND();
}

template<int32_t N>
inline CString<N>& CString<N>::operator+=(double other)
{
    OVERLOAD_APPEND();
}

#undef OVERLOAD_APPEND

template<int32_t N>
inline void CString<N>::Swap(CString<N>& other)
{
    CString<N> temp{other};

    other.Sprintf("%s", c_str());
    this->Sprintf("%s", temp.c_str());
}

template <int32_t N>
inline std::ostream& operator<<(std::ostream& os, const CString<N>& str)
{
    os << str.c_str();
    return os;
}

template <int32_t N>
inline std::istream& operator>>(std::istream& is, CString<N>& str)
{
    std::string tempStr;
    is >> tempStr;
    str = tempStr;
    return is;
}