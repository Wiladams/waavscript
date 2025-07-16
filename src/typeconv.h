#pragma once


#include "ocspan.h"
#include "ps_charcats.h"


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

    // Assumption:  We're sitting at beginning of a number, all whitespace handling
    // has already occured.
    static bool inline readNumber(OctetCursor& s, double& value, bool &isInteger) noexcept
    {
        const unsigned char* startAt = s.begin();
        const unsigned char* endAt = s.end();

        bool isNegative = false;
        double res = 0.0;

        // integer part
        uint64_t intPart = 0;

        // fractional part
        uint64_t fracPart = 0;
        uint64_t fracBase = 1;


        bool hasIntPart = false;
        bool hasFracPart = false;

        // Parse optional sign
        if (*startAt == '+') {
            startAt++;
        }
        else if (*startAt == '-') {
            isNegative = true;
            startAt++;
        }

        // The beginning can either be a digit, or a decimal point
        // Parse integer part
        unsigned char c = *startAt;
        if (PSCharClass::isDigit(c))
        {
            hasIntPart = true;
            s.fStart = startAt;
            read_u64(s, intPart);
            startAt = s.fStart;
            res = static_cast<double>(intPart);
            isInteger = true;
        } else if (c!= '.') {
            // if the first thing we see is not a digit, or a decimal, 
            // then we're not parsing a number, so just return false, 
            // although we might have already consumed the '+' or '-' sign
            return false;
        }

        // Check to see if there's a fractional part to be parsed
        if ((startAt < endAt) && (*startAt == '.'))
        {
            isInteger = false;
            hasFracPart = true;
            startAt++; // move past '.'

            fracBase = 1;

            // Add the fraction portion without calling out to powd
            while ((startAt < endAt) && PSCharClass::isDigit(*startAt)) {
                fracPart = fracPart * 10 + static_cast<uint64_t>(*startAt - '0');
                fracBase *= 10;
                startAt++;
            }
            res += (static_cast<double>(fracPart) / static_cast<double>(fracBase));


            // Parse optional exponent
            // mostly we don't see this, so we won't bother trying
            // to optimize it beyond using powd
            if ((startAt < endAt) && (((*startAt == 'e') || (*startAt == 'E')) && PSCharClass::isDigit(startAt[1])))
            {
                // exponent parts
                uint64_t expPart = 0;
                double expSign = 1.0;

                startAt++; // skip 'E'
                if (*startAt == '+') {
                    startAt++;
                }
                else if (*startAt == '-') {
                    expSign = -1.0;
                    startAt++;
                }


                if (PSCharClass::isDigit(*startAt)) {
                    s.fStart = startAt;
                    read_u64(s, expPart);
                    startAt = s.fStart;
                    res = res * std::pow(10, double(expSign * double(expPart)));
                }
            }
        }

        // If we don't have an integer or fractional
        // part, then just return false
        if (!hasIntPart && !hasFracPart)
            return false;

        s.fStart = startAt;

        value = (!isNegative) ? res : -res;

        return true;
    }
}
