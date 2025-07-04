#pragma once

#include "ocspan.h"
#include "psname.h"


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
		static constexpr bool isDigit(uint8_t c) noexcept { return c >= '0' && c <= '9'; }
		static constexpr bool isNumeric(uint8_t c) noexcept { return is(c, PS_NUMERIC); }
		static constexpr bool isNumericBegin(uint8_t c) noexcept { return (isdigit(c) || c == '.' ||c == '+' || c == '-'); }
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

	// Turn a supposed hex ascii character into a byte value.
	// Returns true if the character was a valid hex digit, and out is set to the value.
	// Returns false if the character was not a valid hex digit, and out is unchanged.
	static inline constexpr bool  decodeHex(uint8_t c, uint8_t& out) noexcept
	{
		if (c >= '0' && c <= '9') { out = (c - '0'); return true; }
		if (c >= 'a' && c <= 'f') { out = (c - 'a') + 10; return true; }
		if (c >= 'A' && c <= 'F') { out = (c - 'A') + 10; return true; }

		return false;
	}
}


namespace waavs 
{
	enum class PSLexType {
		Invalid = 0,
		Whitespace,
        Name,			// name without leading /, e.g. moveto
        LiteralName,	// /name with leading /, e.g. /moveto
        SystemName,		// //moveto
		Number,
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
        DSCComment,		// %%DSCKeyword value
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

    // scanCommentLexeme
    // Scans a comment lexeme from the input stream.
    // Comments start with '%' and can be either a single-line comment or a DSC (Document Structuring Convention) comment.

	inline bool scanCommentLexeme(OctetCursor& src, PSLexeme& tok) {
		const uint8_t* begin = src.fStart;
		const uint8_t* end = src.fEnd;
		if (begin >= end || *begin != '%')
			return false;

		const uint8_t* p = begin + 1;

		// Optional second %
		bool isDSC = (p < end && *p == '%');
		if (isDSC) ++p;

		//const uint8_t* commentStart = p;

		// Scan to end of line, allowing \n, \r, or \r\n
		while (p < end && *p != '\n' && *p != '\r')
			++p;

		const uint8_t* commentEnd = p;

		// Consume line ending
		if (p < end && *p == '\r') {
			++p;
			if (p < end && *p == '\n') ++p;  // handle \r\n
		}
		else if (p < end && *p == '\n') {
			++p;
		}

		src.fStart = p;

		tok.type = isDSC ? PSLexType::DSCComment : PSLexType::Comment;
		tok.span = OctetCursor(begin, commentEnd - begin);  // includes leading %, %% etc.
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

		if (p >= end)
			return false;

		// Optional sign
		if (*p == '+' || *p == '-')
			++p;

		if (p >= end)
			return false;

		// Try to detect radix format (e.g., 16#1A)
		const uint8_t* radixStart = p;
		while (p < end && isdigit(*p)) ++p;

		if (p < end && *p == '#') {
			++p; // skip '#'

			const uint8_t* valueStart = p;
			while (p < end && PSCharClass::isNameChar(*p)) ++p; // base-36 alphanumerics

			if (valueStart < p) {
				lex.type = PSLexType::Number;
				lex.span = OctetCursor(start, p - start);
				src.fStart = p;
				return true;
			}
			else {
				return false; // no valid digits after '#'
			}
		}

		// Reset to handle regular decimal format
		p = start;
		if (*p == '+' || *p == '-')
			++p;

		bool hasDot = false;
		bool hasExp = false;
		bool hasDigitsBeforeDot = false;
		bool hasDigitsAfterDot = false;

		while (p < end) {
			uint8_t c = *p;

			if (isdigit(c)) {
				if (!hasDot)
					hasDigitsBeforeDot = true;
				else
					hasDigitsAfterDot = true;
				++p;
			}
			else if (c == '.' && !hasDot && !hasExp) {
				hasDot = true;
				++p;
			}
			else if ((c == 'e' || c == 'E') && (hasDigitsBeforeDot || hasDigitsAfterDot) && !hasExp) {
				hasExp = true;
				++p;
				if (p < end && (*p == '+' || *p == '-'))
					++p;

				const uint8_t* expStart = p;
				while (p < end && isdigit(*p)) ++p;

				if (expStart == p)
					return false; // exponent with no digits
			}
			else {
				break;
			}
		}

		if ((hasDigitsBeforeDot || hasDigitsAfterDot) && p > start) {
			lex.type = PSLexType::Number;
			lex.span = OctetCursor(start, p - start);
			src.fStart = p;
			return true;
		}

		return false;
	}

	static bool scanLiteralNameLexeme(OctetCursor& src, PSLexeme& lex) noexcept
	{
		src.skip(1);

		const uint8_t* start = src.begin();
		const uint8_t* p = start;			// skip first '/'
		const uint8_t* end = src.end();


		if (p < end && *p == '/') {
			// This is a system operator name, like //moveto, //setgray, etc.
			++p; // skip second '/'
			lex.type = PSLexType::SystemName;
			src.skip(1);
		}
		else {
			// Literal name: /name}
			lex.type = PSLexType::LiteralName;
		}

		const uint8_t* nameStart = p;
		skipWhile(src, PS_NAME_CHAR);
		lex.span = OctetCursor(nameStart, src.begin() - nameStart);
		
		return true;
	}

	static bool scanNameLexeme(OctetCursor& src, PSLexeme& lex) noexcept
	{
		const uint8_t * start = src.begin();
		skipWhile(src, PS_NAME_CHAR);
		lex.type = PSLexType::Name;
		lex.span = OctetCursor(start, src.begin() - start);
		return true;
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
			return scanLiteralNameLexeme(src, lex);
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

		/*
		// Number (starts with digit, '.', '+', or '-')
		// need to distinguish between numbers
        // and extension operators, which begin with '.' typically (.max, .min, etc.)
		// or anything of these starts not immediately followed by a numeric
		if (PSCharClass::isNumericBegin(c)) {
			// if it's a '.', it could be a number
			// or an extension name
			uint8_t c1 = src.peek(1);
			if (src.size() > 1 && !CC::isNumeric(c1) && !CC::isWhitespace(c1) && !CC::isDelimiter(c1))
				return scanNameLexeme(src, lex);

			return scanNumberLexeme(src, lex);
		}
		*/

		if (c == '+' || c == '-') {
			uint8_t next = src.peek(1);

			if (CC::isDigit(next)) {
				// +5 or -3 etc.
				return scanNumberLexeme(src, lex);
			}

			if (next == '.') {
				uint8_t next2 = src.peek(2);
				if (CC::isDigit(next2)) {
					// +.5 or -.7
					return scanNumberLexeme(src, lex);
				}
			}

			// lone + or - or +name
			return scanNameLexeme(src, lex);
		}

		if (c == '.') {
			uint8_t next = src.peek(1);
			if (CC::isDigit(next)) {
				// .5
				return scanNumberLexeme(src, lex);
			}
			// .foo
			return scanNameLexeme(src, lex);
		}

		if (CC::isDigit(c)) {
			return scanNumberLexeme(src, lex);
		}


		// Name token (default)
		if (CC::isNameChar(c)) {
			scanNameLexeme(src, lex);
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
