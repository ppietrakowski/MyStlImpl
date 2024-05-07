#include "FixedString.h"

namespace cstring
{
    char* LastSubstring(const char* haystack, const char* needle)
    {
        if (*needle == '\0')
            return (char*)haystack;

        char* result = NULL;
        for (;;)
        {
            char* p = (char*)strstr(haystack, needle);
            if (p == NULL)
                break;
            result = p;
            haystack = p + 1;
        }

        return result;
    }

    char* FindLastOf(const char* s, const char* accept)
    {
        const char* p = nullptr;
        char* p0 = nullptr, * p1 = nullptr;

        for (p = accept, p0 = p1 = NULL; p && *p; ++p)
        {
            p1 = (char*)strrchr(s, *p);
            if (p1 && p1 > p0)
                p0 = p1;
        }
        return p0;
    }
}