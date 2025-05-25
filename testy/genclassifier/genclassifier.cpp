#include <cstdio>
#include <cctype>
#include <cstdint>


#include "ps_lexer.h"

using namespace waavs;

int main() {
    printf("alignas(256) static constexpr uint8_t table[256] = {\n");

    for (int row = 0; row < 16; ++row) {
        printf("  ");
        for (int col = 0; col < 16; ++col) {
            int c = row * 16 + col;
            uint8_t flags = 0;

            // Whitespace
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                flags |= PS_WHITESPACE;

            // Delimiters
            if (c == '/' || c == '[' || c == ']' || c == '(' || c == ')' ||
                c == '{' || c == '}' || c == '<' || c == '>')
                flags |= PS_DELIMITER;

            // Comment start
            if (c == '%')
                flags |= PS_COMMENT_START | PS_DELIMITER;

            // String delimiters
            if (c == '(' || c == ')')
                flags |= PS_STRING_DELIM;

            // Procedure delimiters
            if (c == '{' || c == '}')
                flags |= PS_PROC_DELIM;

            // Hex digits
            if ((c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'f') ||
                (c >= 'A' && c <= 'F'))
                flags |= PS_HEX_DIGIT;

            // Numeric characters
            if ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E')
                flags |= PS_NUMERIC;

            // Name characters (printable ASCII excluding delimiters)
            if (c >= 33 && c <= 126 && !(flags & PS_DELIMITER))
                flags |= PS_NAME_CHAR;

            printf("%3d,", flags);
        }
        printf("\n");
    }

    printf("};\n");
    return 0;
}

