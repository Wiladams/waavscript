#pragma once

#include <ctype.h>
#include <cstddef> // for size_t

namespace waavs {
    // Portable replacement for POSIX strncasecmp
    inline int pstrncasecmp(const char* s1, const char* s2, size_t n) {
        for (size_t i = 0; i < n; ++i) {
            unsigned char c1 = static_cast<unsigned char>(s1[i]);
            unsigned char c2 = static_cast<unsigned char>(s2[i]);

            // Null terminator ends comparison
            if (c1 == '\0' || c2 == '\0') {
                return c1 - c2;
            }

            // Compare lowercased characters
            int diff = std::tolower(c1) - std::tolower(c2);
            if (diff != 0)
                return diff;
        }
        return 0;
    }
}