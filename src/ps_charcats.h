#pragma once

#include <cstdint>


namespace waavs {
    // Character category flags for PostScript token classification
    // ******************************************************************
    // DO NOT CHANGE THE VALUE OF THE FLAGS
    // ******************************************************************
    // they are used as bit fields in the 
    // PSCharClassifier::table array.
    // If you do want to change them, re-run the genclassifier code to create 
    // the table again.
    enum PSCharCategory : uint8_t {
        PS_WHITESPACE = 1 << 0,  // space, \t, \n, \r
        PS_NAME_CHAR = 1 << 1,  // printable ASCII except delimiters
        PS_NUMERIC = 1 << 2,  // 0-9, +, -, ., e, E
        PS_HEX_DIGIT = 1 << 3,  // 0-9, a-f, A-F
        PS_DELIMITER = 1 << 4,  // (), [], {}, <>, /, %, etc.
        PS_COMMENT_START = 1 << 5,  // %
        PS_STRING_DELIM = 1 << 6,  // ( )
        PS_PROC_DELIM = 1 << 7   // { }
    };

    // The table here comes from running the program genclassifier.cpp
    // run that, capture the output from stdout, and paste it here
    struct PSCharClass {
        alignas(256) static constexpr uint8_t table[256] = {
            1,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  1,  1,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            1,  2,  2,  2,  2, 48,  2,  2, 80, 80,  2,  6,  2,  6,  6, 16,
           14, 14, 14, 14, 14, 14, 14, 14, 14, 14,  2,  2, 16,  2, 16,  2,
            2, 10, 10, 10, 10, 14, 10,  2,  2,  2,  2,  2,  2,  2,  2,  2,
            2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 16,  2, 16,  2,  2,
            2, 10, 10, 10, 10, 14, 10,  2,  2,  2,  2,  2,  2,  2,  2,  2,
            2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,144,  2,144,  2,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        };

        static constexpr uint8_t flags(uint8_t c) noexcept { return table[c]; }

        static constexpr bool is(uint8_t c, uint8_t category) noexcept { return table[c] & category; }

        static constexpr bool isWhitespace(uint8_t c) noexcept { return is(c, PS_WHITESPACE); }
        static constexpr bool isNameChar(uint8_t c) noexcept { return is(c, PS_NAME_CHAR); }
        static constexpr bool isDigit(uint8_t c) noexcept { return c >= '0' && c <= '9'; }
        static constexpr bool isNumeric(uint8_t c) noexcept { return is(c, PS_NUMERIC); }
        static constexpr bool isNumericBegin(uint8_t c) noexcept { return (isdigit(c) || c == '.' || c == '+' || c == '-'); }
        static constexpr bool isHexDigit(uint8_t c) noexcept { return is(c, PS_HEX_DIGIT); }
        static constexpr bool isDelimiter(uint8_t c) noexcept { return is(c, PS_DELIMITER); }
        static constexpr bool isCommentStart(uint8_t c) noexcept { return is(c, PS_COMMENT_START); }
        static constexpr bool isStringDelim(uint8_t c) noexcept { return is(c, PS_STRING_DELIM); }
        static constexpr bool isProcDelim(uint8_t c) noexcept { return is(c, PS_PROC_DELIM); }

    };

}
