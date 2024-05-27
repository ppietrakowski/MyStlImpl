#pragma once

#include <cstring>
#include <string>
#include <cstdint>
#include <numeric>

#include <vector>
#include <stdexcept>
#include <cstdarg>

#include <cassert>

enum : uint64_t
{
    AllCharacters = std::numeric_limits<uint64_t>::max(),
    InvalidPos = std::string::npos
};

/*
* Wrapper around stack allocated char array
* This class exposes interface like std::string
* Should be use in place, where allocations would be a much of overhead
* Also, for easier usage, this class contains tokenize function
* and sprintf/back_sprintf functions to sprintf like functionality
*/
template <size_t N>
class CString
{
public:
    CString();
    CString(const CString<N>& str);

    CString(const char* str);
    CString(const std::string& str);
    CString(const std::string_view str);

    CString& append(const char* str);
    CString& append(const char* str, size_t n);
    CString& append(const char* str, size_t offset, size_t n);

    CString& append(const std::string& str);
    CString& append(const CString<N>& str);
    CString& append(int32_t value);
    CString& append(uint32_t value);

    CString& append(int64_t value);
    CString& append(uint64_t value);
    CString& append(float value);
    CString& append(double value);

    void copy(char* buffer, size_t maxBufferSize) const;
    void clear();

    size_t size() const;
    constexpr size_t capacity() const;
    size_t length() const;

    const char* begin() const;
    const char* end() const;

    char* begin();
    char* end();

    CString& operator=(const CString<N>& str);

    CString& operator=(const char* str);
    CString& operator=(const std::string& str);

    char& operator[](size_t i);
    const char& operator[](size_t i) const;

    const char* c_str() const;
    char* data();
    const char* data() const;

    CString<N> substr(size_t offset, size_t n = AllCharacters) const;
    bool empty() const;

    size_t find(const char* str) const;
    size_t find(const std::string& str) const;
    size_t find(const CString<N>& str) const;

    size_t rfind(const char* str) const;
    size_t rfind(const std::string& str) const;
    size_t rfind(const CString<N>& str) const;

    size_t find_first_of(const char* str) const;
    size_t find_first_of(const std::string& str) const;
    size_t find_first_of(const CString<N>& str) const;

    size_t find_first_not_of(const char* str) const;
    size_t find_first_not_of(const std::string& str) const;
    size_t find_first_not_of(const CString<N>& str) const;

    size_t find_last_of(const char* str) const;
    size_t find_last_of(const std::string& str) const;
    size_t find_last_of(const CString<N>& str) const;

    size_t find_not_last_of(const char* str) const;
    size_t find_not_last_of(const std::string& str) const;
    size_t find_not_last_of(const CString<N>& str) const;

    void tokenize(const char* delimiter, std::vector<CString<N>>& outTokens) const;

    void pop_back();
    void push_back(char c);

    int32_t compare(const char* other) const;
    int32_t compare(const std::string& other) const;
    int32_t compare(const CString<N>& other) const;

    char& back() const
    {
        assert(m_Length > 0); return m_String[m_Length - 1];
    }
    char& front() const
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

    void sprintf(const char* format, ...);
    void vsprintf(const char* format, va_list list);

    void back_sprintf(const char* format, ...);
    void vback_sprintf(const char* format, va_list list);

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

    void swap(CString<N>& other);

    uint64_t hash_code() const
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
    size_t m_Length;
};

namespace std
{
    template <size_t N>
    struct hash<CString<N>>
    {
        size_t operator()(const CString<N>& _Keyval) const
        {
            return _Keyval.hash_code();
        }
    };

    template <size_t N>
    void swap(CString<N>& a, CString<N>& b)
    {
        a.swap(b);
    }

    template <size_t N>
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
template<size_t N>
inline void CString<N>::clear()
{
    memset(&m_String[0], 0, sizeof(m_String));
    m_Length = 0;
}

template<size_t N>
inline size_t CString<N>::size() const
{
    return m_Length;
}

template<size_t N>
inline constexpr size_t CString<N>::capacity() const
{
    return N;
}

template<size_t N>
inline size_t CString<N>::length() const
{
    return m_Length;
}

template<size_t N>
inline const char* CString<N>::begin() const
{
    return &m_String[0];
}

template<size_t N>
inline const char* CString<N>::end() const
{
    return &m_String[m_Length];
}

template<size_t N>
inline char* CString<N>::begin()
{
    return &m_String[0];
}

template<size_t N>
inline char* CString<N>::end()
{
    return &m_String[m_Length];
}

template<size_t N>
inline CString<N>& CString<N>::operator=(const CString<N>& str)
{
    if (this == &str)
    {
        return *this;
    }

    strncpy(m_String, str.m_String, N);

    // always ensure it contains trailing zero
    m_String[N] = 0;
    m_Length = strlen(m_String);
    return *this;
}

template<size_t N>
inline CString<N>& CString<N>::operator=(const char* str)
{
    strncpy(m_String, str, N);

    // always ensure it contains trailing zero
    m_String[N] = 0;
    m_Length = strlen(m_String);
    return *this;
}

template<size_t N>
inline CString<N>& CString<N>::operator=(const std::string& str)
{
    strncpy(m_String, str.c_str(), N);
    m_String[N] = 0;
    m_Length = strlen(m_String);
    return *this;
}

template<size_t N>
inline char& CString<N>::operator[](size_t i)
{
    assert(i < N && "CString out of range");
    return m_String[i];
}

template<size_t N>
inline const char& CString<N>::operator[](size_t i) const
{
    assert(i < N && "CString out of range");
    return m_String[i];
}

template<size_t N>
inline const char* CString<N>::c_str() const
{
    return &m_String[0];
}

template<size_t N>
inline char* CString<N>::data()
{
    return &m_String[0];
}

template<size_t N>
inline const char* CString<N>::data() const
{
    return &m_String[0];
}

template<size_t N>
inline CString<N> CString<N>::substr(size_t offset, size_t n) const
{
    CString<N> str;
    strncpy(str.m_String, m_String + offset, n);
    str.m_String[N] = 0;
    str.m_Length = strlen(str.m_String);
    return str;
}

template<size_t N>
inline bool CString<N>::empty() const
{
    return m_Length == 0;
}

template<size_t N>
inline size_t CString<N>::find(const char* str) const
{
    if (m_Length == 0)
    {
        return InvalidPos;
    }

    const char* foundAt = strstr(m_String, str);
    if (foundAt == nullptr)
    {
        return InvalidPos;
    }

    return static_cast<size_t>(foundAt - m_String);
}

template<size_t N>
inline size_t CString<N>::find(const std::string& str) const
{
    return find(str.c_str());
}

template<size_t N>
inline size_t CString<N>::find(const CString<N>& str) const
{
    return find(str.c_str());
}

template<size_t N>
inline size_t CString<N>::rfind(const char* str) const
{
    if (m_Length == 0)
    {
        return InvalidPos;
    }

    const char* foundAt = cstring::LastSubstring(m_String, str);
    if (foundAt == nullptr)
    {
        return InvalidPos;
    }

    return static_cast<size_t>(foundAt - m_String);
}

template<size_t N>
inline size_t CString<N>::rfind(const std::string& str) const
{
    return rfind(str.c_str());
}

template<size_t N>
inline size_t CString<N>::rfind(const CString<N>& str) const
{
    return rfind(str.m_String);
}

template<size_t N>
inline size_t CString<N>::find_first_of(const char* str) const
{
    if (m_Length == 0)
    {
        return InvalidPos;
    }

    const char* foundAt = strpbrk(m_String, str);
    if (foundAt == nullptr)
    {
        return InvalidPos;
    }

    return static_cast<size_t>(foundAt - m_String);
}

template<size_t N>
inline size_t CString<N>::find_first_of(const std::string& str) const
{
    return find_first_of(str.c_str());
}

template<size_t N>
inline size_t CString<N>::find_first_of(const CString<N>& str) const
{
    return find_first_of(str.m_String);
}

template<size_t N>
inline size_t CString<N>::find_first_not_of(const char* control) const
{
    const char* const baseControl = control;

    for (const char& c : m_String)
    {
        control = baseControl;
        while (control && *control)
        {
            if (c != *control)
            {
                size_t posOfFound = static_cast<size_t>(&c - &m_String[0]);
                return posOfFound;
            }
            control++;
        }
    }

    return InvalidPos;
}

template<size_t N>
inline size_t CString<N>::find_first_not_of(const std::string& str) const
{
    return find_first_not_of(str.c_str());
}

template<size_t N>
inline size_t CString<N>::find_first_not_of(const CString<N>& str) const
{
    return find_first_not_of(str.m_String);
}

template<size_t N>
inline size_t CString<N>::find_last_of(const char* str) const
{
    if (m_Length == 0)
    {
        return InvalidPos;
    }

    const char* foundAt = cstring::FindLastOf(m_String, str);
    if (foundAt == nullptr)
    {
        return InvalidPos;
    }

    return static_cast<size_t>(foundAt - m_String);
}

template<size_t N>
inline size_t CString<N>::find_last_of(const std::string& str) const
{
    return find_last_of(str.c_str());
}

template<size_t N>
inline size_t CString<N>::find_last_of(const CString<N>& str) const
{
    return find_last_of(str.m_String);
}

template<size_t N>
inline size_t CString<N>::find_not_last_of(const char* str) const
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
                size_t posOfFound = static_cast<size_t>(&c - &m_String[0]);
                return posOfFound;
            }
            str++;
        }
    }
}

template<size_t N>
inline size_t CString<N>::find_not_last_of(const std::string& str) const
{
    return find_not_last_of(str.c_str());
}

template<size_t N>
inline size_t CString<N>::find_not_last_of(const CString<N>& str) const
{
    return find_not_last_of(str.m_String);
}

template<size_t N>
inline void CString<N>::tokenize(const char* delimiter, std::vector<CString<N>>& outTokens) const
{
    char* foundAt = strtok(m_String, delimiter);

    while (foundAt != nullptr)
    {
        outTokens.push_back(foundAt);
        foundAt = strtok(nullptr, delimiter);
    }
}

template<size_t N>
inline void CString<N>::pop_back()
{
    assert(m_Length > 0 && "Nothing to pop");
    m_Length -= 1;
    m_String[m_Length] = 0;
}

template<size_t N>
inline void CString<N>::push_back(char c)
{
    if (m_Length < N - 1)
    {
        m_String[m_Length++] = c;
    }
}

template<size_t N>
inline int32_t CString<N>::compare(const char* other) const
{
    return strcmp(m_String, other);
}

template<size_t N>
inline int32_t CString<N>::compare(const std::string& other) const
{
    return compare(other.c_str());
}

template<size_t N>
inline int32_t CString<N>::compare(const CString<N>& other) const
{
    return compare(other.m_String);
}

template<size_t N>
inline bool CString<N>::operator==(const char* str) const
{
    return compare(str) == 0;
}

template<size_t N>
inline bool CString<N>::operator==(const CString<N>& str) const
{
    return compare(str) == 0;
}

template<size_t N>
inline bool CString<N>::operator==(const std::string& str) const
{
    return compare(str) == 0;
}

template<size_t N>
inline CString<N>::CString()
{
    memset(&m_String[0], 0, sizeof(m_String));
    m_Length = 0;
}

template<size_t N>
inline CString<N>::CString(const CString<N>& str)
{
    *this = str;
}

template<size_t N>
inline CString<N>::CString(const char* str)
{
    strncpy(m_String, str, N);
    m_String[N] = 0;
    m_Length = strlen(m_String);
}

template<size_t N>
inline CString<N>::CString(const std::string& str)
{
    strncpy(m_String, str.c_str(), N);
    m_String[N] = 0;
    m_Length = strlen(m_String);
}

template<size_t N>
inline CString<N>::CString(const std::string_view str) :
    CString<N>(str.data())
{
}

template<size_t N>
inline CString<N>& CString<N>::append(const char* str)
{
    return append(str, strlen(str));
}

template<size_t N>
inline CString<N>& CString<N>::append(const char* str, size_t n)
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

template<size_t N>
inline CString<N>& CString<N>::append(const char* str, size_t offset, size_t n)
{
    if (n == AllCharacters)
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

template<size_t N>
inline CString<N>& CString<N>::append(const std::string& str)
{
    return append(str.c_str());
}

template<size_t N>
inline CString<N>& CString<N>::append(const CString<N>& str)
{
    return append(str.m_String);
}

template<size_t N>
inline CString<N>& CString<N>::append(int32_t value)
{
    back_sprintf("%i", value);
    return *this;
}

template<size_t N>
inline CString<N>& CString<N>::append(uint32_t value)
{
    back_sprintf("%u", value);
    return *this;
}

template<size_t N>
inline CString<N>& CString<N>::append(int64_t value)
{
    back_sprintf("%ill", value);
    return *this;
}

template<size_t N>
inline CString<N>& CString<N>::append(uint64_t value)
{
    back_sprintf("%ull", value);
    return *this;
}

template<size_t N>
inline CString<N>& CString<N>::append(float value)
{
    back_sprintf("%f", value);
    return *this;
}

template<size_t N>
inline CString<N>& CString<N>::append(double value)
{
    back_sprintf("%f", value);
    return *this;
}

template<size_t N>
inline void CString<N>::copy(char* buffer, size_t maxBufferSize) const
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

template<size_t N>
inline void CString<N>::sprintf(const char* format, ...)
{
    va_list list;

    va_start(list, format);
    this->vsprintf(format, list);
    va_end(list);
}

template<size_t N>
inline void CString<N>::vsprintf(const char* format, va_list list)
{
    ::vsnprintf(m_String, N, format, list);
    m_String[N] = 0;
}

template<size_t N>
inline void CString<N>::back_sprintf(const char* format, ...)
{
    va_list list;
    va_start(list, format);
    this->vback_sprintf(format, list);
    va_end(list);
}

template<size_t N>
inline void CString<N>::vback_sprintf(const char* format, va_list list)
{
    CString<N> tempBuffer;
    ::vsnprintf(tempBuffer.m_String, N, format, list);
    append(tempBuffer);
}

#define OVERLOAD_APPEND() CString<N> control{ *this }; control.append(other); return control
template<size_t N>
inline CString<N> CString<N>::operator+(const CString<N>& other) const
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N> CString<N>::operator+(const char* other) const
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N> CString<N>::operator+(int32_t other) const
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N> CString<N>::operator+(int64_t other) const
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N> CString<N>::operator+(uint64_t other) const
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N> CString<N>::operator+(float other) const
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N> CString<N>::operator+(double other) const
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N> CString<N>::operator+(const std::string& other) const
{
    OVERLOAD_APPEND();
}

#undef OVERLOAD_APPEND
#define OVERLOAD_APPEND() append(other); return *this

template<size_t N>
inline CString<N>& CString<N>::operator+=(const std::string& other)
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N>& CString<N>::operator+=(const CString<N>& other)
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N>& CString<N>::operator+=(const char* other)
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N>& CString<N>::operator+=(int32_t other)
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N>& CString<N>::operator+=(int64_t other)
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N>& CString<N>::operator+=(uint64_t other)
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N>& CString<N>::operator+=(float other)
{
    OVERLOAD_APPEND();
}

template<size_t N>
inline CString<N>& CString<N>::operator+=(double other)
{
    OVERLOAD_APPEND();
}

#undef OVERLOAD_APPEND

template<size_t N>
inline void CString<N>::swap(CString<N>& other)
{
    CString<N> temp{other};

    other.sprintf("%s", c_str());
    this->sprintf("%s", temp.c_str());
}

template <size_t N>
inline std::ostream& operator<<(std::ostream& os, const CString<N>& str)
{
    os << str.c_str();
    return os;
}

template <size_t N>
inline std::istream& operator>>(std::istream& is, CString<N>& str)
{
    std::string tempStr;
    is >> tempStr;
    str = tempStr;
    return is;
}