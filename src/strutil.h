#pragma once

#include <ctype.h>
#include <cstddef> // for size_t
#include <cstdint>
#include <cstring>

namespace waavs {
    
    // Converts val into ASCII characters in dst
    // Returns number of characters written (no null terminator)
    // Assumes dst has at least 11 bytes available (enough for -2147483648)
    int int32_to_ascii(char* dst, int32_t val)
    {
        char tmp[11]; // max 10 digits + optional minus
        int pos = 0;

        uint32_t absval;

        if (val < 0) {
            dst[pos++] = '-';
            absval = static_cast<uint32_t>(-static_cast<int64_t>(val));
        }
        else {
            absval = static_cast<uint32_t>(val);
        }

        // Special case for 0
        if (absval == 0) {
            dst[pos++] = '0';
            return pos;
        }

        int len = 0;
        while (absval > 0) {
            tmp[len++] = '0' + (absval % 10);
            absval /= 10;
        }

        // Reverse digits into dst
        for (int i = len - 1; i >= 0; --i) {
            dst[pos++] = tmp[i];
        }

        return pos;
    }


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



    // Standard 32-bit Murmur3 implementation (simplified)
    uint32_t murmur3(const void* key, size_t len, uint32_t seed = 0)
    {
        const uint8_t* data = static_cast<const uint8_t*>(key);
        const int nblocks = len / 4;
        uint32_t h = seed;

        const uint32_t c1 = 0xcc9e2d51;
        const uint32_t c2 = 0x1b873593;

        // body
        const uint32_t* blocks = reinterpret_cast<const uint32_t*>(data);
        for (int i = 0; i < nblocks; i++)
        {
            uint32_t k = blocks[i];
            k *= c1;
            k = (k << 15) | (k >> 17);
            k *= c2;

            h ^= k;
            h = (h << 13) | (h >> 19);
            h = h * 5 + 0xe6546b64;
        }

        // tail
        const uint8_t* tail = data + nblocks * 4;
        uint32_t k1 = 0;

        switch (len & 3)
        {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
            k1 *= c1;
            k1 = (k1 << 15) | (k1 >> 17);
            k1 *= c2;
            h ^= k1;
        }

        // finalization
        h ^= len;
        h ^= h >> 16;
        h *= 0x85ebca6b;
        h ^= h >> 13;
        h *= 0xc2b2ae35;
        h ^= h >> 16;

        return h;
    }

#include <cstdint>
#include <cstddef>

    // Example MurmurHash64A
    uint64_t murmur_hash64(const void* key, size_t len, uint64_t seed)
    {
        const uint64_t m = 0xc6a4a7935bd1e995ULL;
        const int r = 47;

        uint64_t h = seed ^ (len * m);

        const uint64_t* data = (const uint64_t*)key;
        const uint64_t* end = data + (len / 8);

        while (data != end)
        {
            uint64_t k = *data++;

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }

        const unsigned char* data2 = (const unsigned char*)data;

        switch (len & 7)
        {
        case 7: h ^= uint64_t(data2[6]) << 48;
        case 6: h ^= uint64_t(data2[5]) << 40;
        case 5: h ^= uint64_t(data2[4]) << 32;
        case 4: h ^= uint64_t(data2[3]) << 24;
        case 3: h ^= uint64_t(data2[2]) << 16;
        case 2: h ^= uint64_t(data2[1]) << 8;
        case 1: h ^= uint64_t(data2[0]);
            h *= m;
        };

        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
    }

    /*
    // usage examples
    uintptr_t ptr = reinterpret_cast<uintptr_t>(my_pointer);
    uint64_t hash = murmur_hash64(&ptr, sizeof(ptr), 0x12345678);

    // Example for a PostScript-like dictionary key hash:
    uint32_t hashKey(const char* name)
    {
        return murmur3(name, strlen(name));
    }

    uint32_t hashKey(int32_t number)
    {
        return murmur3(&number, sizeof(number));
    }
    */

}