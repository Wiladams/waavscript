// typeconv.h
#pragma once


#include "ocspan.h"
#include "ps_charclass.h"


namespace waavs {

    // hexToNibble
    // 
    // Turn a supposed hex ascii character into a nibble (4-bit) value.
    // Returns true if the character was a valid hex digit, and out is set to the value.
    // Returns false if the character was not a valid hex digit, and out is unchanged.
    //
    static inline constexpr bool  hexToNibble(const uint8_t c, uint8_t& out) noexcept
    {
        if (c >= '0' && c <= '9') { out = (c - '0'); return true; }
        if (c >= 'a' && c <= 'f') { out = (c - 'a') + 10; return true; }
        if (c >= 'A' && c <= 'F') { out = (c - 'A') + 10; return true; }

        return false;
    }

    /*
    static inline constexpr bool hexToNibble(uint8_t c, uint8_t& out) noexcept
    {
        uint8_t nibble = (c & 0x0F)
                   + ((c >> 6) & 0x01) * 9;  // Adds 9 if alphabetic

        // Valid if c is in ['0'-'9', 'A'-'F', 'a'-'f']
        bool valid = ((c >= '0') & (c <= '9')) |
                 ((c >= 'A') & (c <= 'F')) |
                 ((c >= 'a') & (c <= 'f'));

        out = nibble;
        return valid;
    }

    // If you're targeting maximum speed in hot loops, the bitmask version above is optimal. 
    // Otherwise, your current implementation is already idiomatic, readable, and efficient.
    */
    
    // read_u64
    // Read a 64-bit unsigned integer from the input span
    // advance the span 
    static INLINE bool read_u64(OctetCursor& s, uint64_t& v) noexcept
    {
        if (s.empty())
            return false;

        v = 0;
        const unsigned char* sStart = s.begin();
        const unsigned char* sEnd = s.end();

        while ((sStart < sEnd) && PSCharClass::isDigit(*sStart))
        {
            v = (v * 10) + (uint64_t)(*sStart - '0');
            sStart++;
        }

        s.fStart = sStart;

        return true;
    }

    /*
    // Parse a complete PostScript numeric token.
    // Supports decimal integers, real numbers, exponents, and radix integers.
    static inline bool readPSNumber(OctetCursor& s, double& value, bool& isInteger) noexcept
    {
        const uint8_t* p = s.begin();
        const uint8_t* end = s.end();

        if (p == end) return false;

        bool negative = false;

        if (*p == '+' || *p == '-') {
            negative = (*p == '-');
            ++p;
        }

        if (p == end) return false;

        const uint8_t* digits = p;
        uint64_t integerPart = 0;

        while (p < end && *p >= '0' && *p <= '9') {
            uint32_t digit = *p++ - '0';
            if (integerPart > (UINT64_MAX - digit) / 10) return false;
            integerPart = integerPart * 10 + digit;
        }

        // PostScript radix integer: radix#digits
        if (p < end && *p == '#') {
            if (negative || digits == p || integerPart < 2 || integerPart > 36)
                return false;

            uint32_t radix = static_cast<uint32_t>(integerPart);
            uint32_t result = 0;
            ++p;

            if (p == end) return false;

            while (p < end) {
                uint8_t ch = *p++;
                uint32_t digit;

                if (ch >= '0' && ch <= '9')
                    digit = ch - '0';
                else if (ch >= 'A' && ch <= 'Z')
                    digit = ch - 'A' + 10;
                else if (ch >= 'a' && ch <= 'z')
                    digit = ch - 'a' + 10;
                else
                    return false;

                if (digit >= radix) return false;
                if (result > (UINT32_MAX - digit) / radix) return false;

                result = result * radix + digit;
            }

            value = static_cast<double>(std::bit_cast<int32_t>(result));
            isInteger = true;
            s.fStart = p;
            return true;
        }

        bool hasIntegerDigits = (p != digits);
        bool hasFraction = false;
        double result = static_cast<double>(integerPart);

        if (p < end && *p == '.') {
            hasFraction = true;
            ++p;

            double scale = 0.1;

            while (p < end && *p >= '0' && *p <= '9') {
                result += static_cast<double>(*p++ - '0') * scale;
                scale *= 0.1;
            }
        }

        if (!hasIntegerDigits && !hasFraction) return false;

        bool hasExponent = false;

        if (p < end && (*p == 'e' || *p == 'E')) {
            hasExponent = true;
            ++p;

            bool negativeExponent = false;

            if (p < end && (*p == '+' || *p == '-')) {
                negativeExponent = (*p == '-');
                ++p;
            }

            if (p == end || *p < '0' || *p > '9') return false;

            int exponent = 0;

            while (p < end && *p >= '0' && *p <= '9') {
                int digit = *p++ - '0';
                if (exponent > (10000 - digit) / 10) return false;
                exponent = exponent * 10 + digit;
            }

            result *= std::pow(10.0, negativeExponent ? -exponent : exponent);
        }

        if (p != end) return false;

        if (negative) result = -result;
        if (!std::isfinite(result)) return false;

        isInteger = !hasFraction && !hasExponent && result >= INT32_MIN && result <= INT32_MAX;
        value = result;
        s.fStart = p;

        return true;
    }
    */
}
