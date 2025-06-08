#pragma once

#include <cstdint>

namespace waavs {



	// PostScript character classifier - 256-byte aligned lookup table
	struct alignas(256) PSCharClassifier final {
		uint8_t table[256];

		constexpr PSCharClassifier() : table{} {
			for (int i = 0; i < 256; ++i) {
				uint8_t c = static_cast<uint8_t>(i);
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

				table[i] = flags;
			}
		}

		constexpr bool is(uint8_t c, uint8_t category) const noexcept {
			return table[c] & category;
		}

		constexpr uint8_t flags(uint8_t c) const noexcept {
			return table[c];
		}
	};

	// Global constexpr instance
	inline constexpr PSCharClassifier psCharClassifier{};

} // namespace waavs
