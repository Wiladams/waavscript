#pragma once

#include "ocspan.h"


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
		static constexpr bool isNumeric(uint8_t c) noexcept { return is(c, PS_NUMERIC); }
		static constexpr bool isNumericBegin(uint8_t c) noexcept { return (isdigit(c) || c == '+' || c == '-'); }
		static constexpr bool isHexDigit(uint8_t c) noexcept { return is(c, PS_HEX_DIGIT); }
		static constexpr bool isDelimiter(uint8_t c) noexcept { return is(c, PS_DELIMITER); }
		static constexpr bool isCommentStart(uint8_t c) noexcept { return is(c, PS_COMMENT_START); }
		static constexpr bool isStringDelim(uint8_t c) noexcept { return is(c, PS_STRING_DELIM); }
		static constexpr bool isProcDelim(uint8_t c) noexcept { return is(c, PS_PROC_DELIM); }



	};

}

namespace waavs {

	// Skip characters in the input stream that match the given category mask
	static inline const uint8_t* skipWhile(OctetCursor& src, uint8_t categoryMask) noexcept
	{
		const uint8_t* p = src.begin();
		const uint8_t* end = src.end();

		if (p == end) return p;  // Empty input

		while (p < end && PSCharClass::is(*p, categoryMask))
		{
			++p;
		}

		src.fStart = p;  // Update cursor position

		return p;
	}

	// Skip characters in the input stream until a character matching the given category mask is found
	static inline const uint8_t* skipUntil(OctetCursor& src, uint8_t categoryMask) noexcept
	{
		const uint8_t* p = src.begin();
		const uint8_t* end = src.end();

		if (p == end) return p;  // Empty input


		while (p < end && !PSCharClass::is(*p, categoryMask))
		{
			++p;
		}
		src.fStart = p;  // Update cursor position
		return p;
	}
}


namespace waavs 
{
	enum class PSLexType {
		Invalid = 0,
		Whitespace,
		Name,
		Number,
		LiteralName,
		String,
		UnterminatedString,
		HexString,
		LBRACE,			// {
		RBRACE,			// }
		LBRACKET,		// [
		RBRACKET,		// ]
		LLANGLE,		// for <<
		RRANGLE,		// for >>
		Comment,
		Delimiter,
		Eof
	};

	struct PSLexeme 
	{
		PSLexType type = PSLexType::Invalid;
		OctetCursor span; // points to original data slice
	};

}

namespace waavs {
	static bool scanCommentLexeme(OctetCursor& src, PSLexeme& lex) noexcept 
	{
		const uint8_t* start = src.begin();
		const uint8_t* p = start + 1; // skip '%'
		const uint8_t* end = src.end();
		while (p < end && *p != '\n' && *p != '\r') {
			++p;
		}
		// Handle line endings
		if (p < end && *p == '\r') {
			if (p + 1 < end && *(p + 1) == '\n') ++p; // consume \r\n
			else ++p; // just consume \r
		}
		else if (p < end && *p == '\n') {
			++p; // consume \n
		}
		src.fStart = p; // Update cursor position
		lex.type = PSLexType::Comment;
		lex.span = OctetCursor(start, p - start); // span includes '%'
		
		return true;
	}

	// scanStringLexeme
	//
	// Strings are probably the most complex of the data structures in Postscript,
	// at least from a scanning perspective.  This routine isolates that complexity.
	//
	static bool scanStringLexeme(OctetCursor& src, PSLexeme& lex) noexcept 
	{
		const uint8_t* start = src.begin(); // points to '('
		const uint8_t* p = start + 1;
		const uint8_t* end = src.end();

		int depth = 1;
		bool inEscape = false;

		while (p < end && depth > 0) {
			uint8_t c = *p++;

			if (inEscape) {
				if (c >= '0' && c <= '7') {
					// Consume up to 2 more octal digits
					int count = 0;
					while (count < 2 && p < end && *p >= '0' && *p <= '7') {
						++p;
						++count;
					}
				}
				else if (c == '\n' || c == '\r') {
					// Line continuation: consume \n or \r\n
					if (c == '\r' && p < end && *p == '\n') ++p;
				}
				// For all other escape cases: do nothing extra
				inEscape = false;
			}
			else {
				if (c == '\\') {
					inEscape = true;
				}
				else if (c == '(') {
					++depth;
				}
				else if (c == ')') {
					--depth;
				}
			}
		}

		const uint8_t* spanEnd = p;
		lex.type = (depth == 0) ? PSLexType::String : PSLexType::UnterminatedString;
		lex.span = OctetCursor(start + 1, (spanEnd - 1) - (start + 1));  // span excludes outer parens
		src.fStart = spanEnd;

		return true;
	}

	static bool scanNumberLexeme(OctetCursor& src, PSLexeme& lex) noexcept 
	{
		const uint8_t* start = src.begin();
		const uint8_t* p = start;
		const uint8_t* end = src.end();

		// Optional sign at the start
		if (*p == '+' || *p == '-') ++p;

		// Try to detect radix format: digits followed by '#' (e.g., 16#1A)
		const uint8_t* radixDigitsStart = p;
		while (p < end && isdigit(*p)) ++p;

		if (p < end && *p == '#') {
			++p; // skip '#'

			const uint8_t* valueStart = p;
			while (p < end && PSCharClass::isNameChar(*p)) ++p; // allow base-36 alphanumerics

			if (valueStart < p) {
				// We have something like 16#1A
				lex.type = PSLexType::Number;
				lex.span = OctetCursor(start, p - start);
				src.fStart = p;
				return true;
			}
		}

		// Not radix format — try regular number with optional '.' and exponent
		p = start;
		if (*p == '+' || *p == '-') ++p;

		bool hasDigits = false;
		bool hasDot = false;
		bool hasExp = false;

		while (p < end) {
			uint8_t c = *p;

			if (isdigit(c)) {
				hasDigits = true;
				++p;
			}
			else if (c == '.' && !hasDot && !hasExp) {
				hasDot = true;
				++p;
			}
			else if ((c == 'e' || c == 'E') && hasDigits && !hasExp) {
				hasExp = true;
				++p;
				if (p < end && (*p == '+' || *p == '-')) ++p;
			}
			else {
				break;
			}
		}

		if (p > start && hasDigits) {
			lex.type = PSLexType::Number;
			lex.span = OctetCursor(start, p - start);
			src.fStart = p;
			return true;
		}

		// Fallback — not a number
		return false;
	}


	// nextPSLexeme
	// Return the next lexically significant token from the input stream.
	// Updates the source cursor to point to the next position after the token.
	//
	// This is pretty low level.  It will do things like isolate a number, but 
	// won't actually give you the decimal value for that number
	//
	static bool nextPSLexeme(OctetCursor& src, PSLexeme& lex) noexcept {
		using CC = PSCharClass;

		// Skip whitespace
		skipWhile(src, PS_WHITESPACE);

		if (src.empty()) {
			lex.type = PSLexType::Eof;
			lex.span = src;
			return false;
		}

		const uint8_t* start = src.begin();
		uint8_t c = *start;

		// Handle comments (consume to end of line)
		if (CC::isCommentStart(c)) {
			return scanCommentLexeme(src, lex);
		}

		// Literal name: starts with '/'
		if (c == '/') {
			++src; // skip '/'
			const uint8_t* nameStart = src.begin();
			skipWhile(src, PS_NAME_CHAR);
			lex.type = PSLexType::LiteralName;
			lex.span = OctetCursor(nameStart, src.begin() - nameStart);
			return true;
		}

		// Procedure delimiters
		if (c == '{') {
			++src;
			lex.type = PSLexType::LBRACE;
			lex.span = OctetCursor(start, 1);
			return true;
		}
		if (c == '}') {
			++src;
			lex.type = PSLexType::RBRACE;
			lex.span = OctetCursor(start, 1);
			return true;
		}

		// Array delimiters
		if (c == '[') {
			++src;
			lex.type = PSLexType::LBRACKET;
			lex.span = OctetCursor(start, 1);
			return true;
		}
		if (c == ']') {
			++src;
			lex.type = PSLexType::RBRACKET;
			lex.span = OctetCursor(start, 1);
			return true;
		}

		// Strings
		if (c == '(') {
			return scanStringLexeme(src, lex);
		}

		// Dictionary start or hex string
		// Hexadecimal strings (starts with '<')
		// <45365664>
		// or Dictionary
		// << /key1 (value1) /key2 <value2> >>
		if (c == '<') {
			const uint8_t* p = src.begin();
			if (src.size() >= 2 && src.peek(1) == '<') {
				// DictBegin: <<
				lex.type = PSLexType::LLANGLE;
				lex.span = OctetCursor(p, 2);
				src.skip(2);
				return true;
			}
			else {
				// HexString: <...>
				++src; // skip '<'
				const uint8_t* strStart = src.begin();
				const uint8_t* q = strStart;
				const uint8_t* end = src.end();

				while (q < end && *q != '>') ++q;

				lex.type = (q < end) ? PSLexType::HexString : PSLexType::UnterminatedString;
				lex.span = OctetCursor(strStart, q - strStart);

				if (q < end) ++q; // skip '>'
				src.fStart = q;
				return true;
			}
		}

		// End of Hexstring, or Dictionary end (>>)
		if (c == '>') {
			const uint8_t* p = src.begin();
			if (src.size() >= 2 && src.peek(1) == '>') {
				// DictEnd: >>
				lex.type = PSLexType::RRANGLE;
				lex.span = OctetCursor(p, 2);
				src.skip(2);
				return true;
			}
			else {
				// Single '>' (possibly malformed)
				lex.type = PSLexType::Delimiter;
				lex.span = OctetCursor(p, 1);
				src.skip(1);
				return true;
			}
		}


		// Number (starts with digit, '.', '+', or '-')
		if (PSCharClass::isNumericBegin(c)) {
			return scanNumberLexeme(src, lex);
		}

		// Name token (default)
		if (CC::isNameChar(c)) {
			skipWhile(src, PS_NAME_CHAR);
			lex.type = PSLexType::Name;
			lex.span = OctetCursor(start, src.begin() - start);
			return true;
		}

		// Fallback: single-character delimiter
		++src.fStart;
		lex.type = PSLexType::Delimiter;
		lex.span = OctetCursor(start, 1);
		return true;
	}

} // namespace waavs

namespace waavs {
	struct PSLexemeGenerator {
		OctetCursor src;

		PSLexemeGenerator(OctetCursor input) : src(input) {}

		bool next(PSLexeme &lex) 
		{
			return nextPSLexeme(src, lex);
		}

	};
}
