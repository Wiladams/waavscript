#pragma once

#include "bspan.h"

namespace waavs {
    // read_u64
// Read a 64-bit unsigned integer from the input span
// advance the span 
    static INLINE bool read_u64(ByteSpan& s, uint64_t& v) noexcept
    {
        if (!s)
            return false;

        v = 0;
        const unsigned char* sStart = s.fStart;
        const unsigned char* sEnd = s.fEnd;

        while ((sStart < sEnd) && is_digit(*sStart))
        {
            v = (v * 10) + (uint64_t)(*sStart - '0');
            sStart++;
        }

        s.fStart = sStart;

        return true;
    }

    // Assumption:  We're sitting at beginning of a number, all whitespace handling
    // has already occured.
    static bool inline readDecimal(ByteSpan& s, double& value) noexcept
    {
        const unsigned char* startAt = s.fStart;
        const unsigned char* endAt = s.fEnd;

        bool isNegative = false;
        //double sign = 1.0;
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
            //sign = -1;
            isNegative = true;
            startAt++;
        }

        // Parse integer part
        unsigned char c = *startAt;
        if (is_digit(c))
        {
            hasIntPart = true;
            s.fStart = startAt;
            read_u64(s, intPart);
            startAt = s.fStart;
            res = static_cast<double>(intPart);
        }

        // Parse fractional part.
        if ((startAt < endAt) && (*startAt == '.'))
        {
            hasFracPart = true;
            startAt++; // Skip '.'

            fracBase = 1;

            // Add the fraction portion without calling out to powd
            while ((startAt < endAt) && is_digit(*startAt)) {
                fracPart = fracPart * 10 + static_cast<uint64_t>(*startAt - '0');
                fracBase *= 10;
                startAt++;
            }
            res += (static_cast<double>(fracPart) / static_cast<double>(fracBase));

        }

        // If we don't have an integer or fractional
        // part, then just return false
        if (!hasIntPart && !hasFracPart)
            return false;

        // Parse optional exponent
        // mostly we don't see this, so we won't bother trying
        // to optimize it beyond using powd
        if ((startAt < endAt) &&
            (((*startAt == 'e') || (*startAt == 'E')) &&
                ((startAt[1] != 'm') && (startAt[1] != 'x'))))
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


            if (is_digit(*startAt)) {
                s.fStart = startAt;
                read_u64(s, expPart);
                startAt = s.fStart;
                res = res * std::pow(10, double(expSign * double(expPart)));
            }
        }
        s.fStart = startAt;

        value = (!isNegative) ? res : -res;

        return true;
    }
}
