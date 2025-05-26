#pragma once

//#include <cstdint>
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
		static constexpr uint8_t table[256] = {
			0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0,  1,  0,  0,
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
			0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };

		static constexpr uint8_t flags(uint8_t c) noexcept { return table[c]; }

		static constexpr bool is(uint8_t c, uint8_t category) noexcept { return table[c] & category; }

		static constexpr bool isWhitespace(uint8_t c) noexcept { return is(c, PS_WHITESPACE); }
		static constexpr bool isNameChar(uint8_t c) noexcept { return is(c, PS_NAME_CHAR); }
		static constexpr bool isNumeric(uint8_t c) noexcept { return is(c, PS_NUMERIC); }
		static constexpr bool isHexDigit(uint8_t c) noexcept { return is(c, PS_HEX_DIGIT); }
		static constexpr bool isDelimiter(uint8_t c) noexcept { return is(c, PS_DELIMITER); }
		static constexpr bool isCommentStart(uint8_t c) noexcept { return is(c, PS_COMMENT_START); }
		static constexpr bool isStringDelim(uint8_t c) noexcept { return is(c, PS_STRING_DELIM); }
		static constexpr bool isProcDelim(uint8_t c) noexcept { return is(c, PS_PROC_DELIM); }



	};

}

namespace waavs {

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
		ProcBegin,
		ProcEnd,
		ArrayBegin,
		ArrayEnd,
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

	inline bool nextPSLexeme(OctetCursor& src, PSLexeme& lex) noexcept {
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
			++src; // skip '%'
			const uint8_t* p = src.begin();
			const uint8_t* end = src.end();
			
			// skip to end of line
			while (p < end && *p != '\n' && *p != '\r') 
				++p;
			lex.type = PSLexType::Comment;
			lex.span = OctetCursor(start, p - start);
			src.fStart = p;
			return true;
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
			lex.type = PSLexType::ProcBegin;
			lex.span = OctetCursor(start, 1);
			return true;
		}
		if (c == '}') {
			++src;
			lex.type = PSLexType::ProcEnd;
			lex.span = OctetCursor(start, 1);
			return true;
		}

		// Array delimiters
		if (c == '[') {
			++src;
			lex.type = PSLexType::ArrayBegin;
			lex.span = OctetCursor(start, 1);
			return true;
		}
		if (c == ']') {
			++src;
			lex.type = PSLexType::ArrayEnd;
			lex.span = OctetCursor(start, 1);
			return true;
		}

		// Strings
		if (c == '(') {
			++src; // skip '('
			const uint8_t* strStart = src.begin();
			const uint8_t* p = strStart;
			const uint8_t* end = src.end();
			bool inEscape = false;
			while (p < end) {
				if (inEscape) { inEscape = false; ++p; continue; }
				if (*p == '\\') { inEscape = true; ++p; continue; }
				if (*p == ')') break;
				++p;
			}
			lex.type = (p < end) ? PSLexType::String : PSLexType::UnterminatedString;
			lex.span = OctetCursor(strStart, p - strStart);
			if (p < end) ++p; // skip ')'
			src.fStart = p;
			return true;
		}

		// Hexadecimal strings (starts with '<')
		// <45365664>
		if (c == '<') {
			++src; // skip '<'
			const uint8_t* strStart = src.begin();
			const uint8_t* p = strStart;
			const uint8_t* end = src.end();

			while (p < end && *p != '>') ++p;

			lex.type = (p < end) ? PSLexType::HexString : PSLexType::UnterminatedString;
			lex.span = OctetCursor(strStart, p - strStart);

			if (p < end) ++p; // skip '>'
			src.fStart = p;
			return true;
		}

		// Number (starts with digit, '.', '+', or '-')
		if (PSCharClass::isNumeric(c)) {
			const uint8_t* p = start;
			const uint8_t* end = src.end();
			if (*p == '+' || *p == '-') ++p;
			bool hasDigits = false;
			while (p < end && PSCharClass::is(*p, PS_NUMERIC)) {
				hasDigits = true;
				++p;
			}
			if (hasDigits) {
				lex = { PSLexType::Number, OctetCursor(start, p - start) };
				src.fStart = p;
				return true;
			}
		}

		// Name token (default)
		if (CC::isNameChar(c)) {
			const uint8_t* p = start;
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
