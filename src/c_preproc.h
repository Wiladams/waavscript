#pragma once

#include "ps_lexer.h"

#pragma once

#include "ocspan.h"
#include "ps_lexer.h"
#include "psdictionary.h"

namespace waavs {

	enum class PrepTokenType {
		Invalid = 0,
		Identifier,
		Number,
		String,
		CharConst,
		Punctuator,
		Directive,
		Comment,
		Whitespace,
		Newline,
		EndOfFile
	};

	struct PrepToken {
		PrepTokenType type = PrepTokenType::Invalid;
		OctetCursor span;
	};

	static const PSDictionary * multiCharPunctTable()
	{
		static auto dict = PSDictionary::create(32);

		if (dict->empty()) {
			const char* tokens[] = {
				"...", ">>=", "<<=", "->", "++", "--", "==", "!=", "<=", ">=",
				"&&", "||", "<<", ">>", "+=", "-=", "*=", "/=", "%=", "&=", "^=", "|=", "##", "::", nullptr
			};
			for (const char** p = tokens; *p; ++p) {
				dict->put(PSName(*p), PSObject()); // Value can be unused
			}
		}

		return dict.get();
	}

	/*
	bool convertArg(const PSObject& obj, const PSName& type, void*& out)
	{
		if (type == "int")
		{
			if (!obj.isInt()) return false;
			out = reinterpret_cast<void*>(static_cast<intptr_t>(obj.asInt()));
			return true;
		}
		else if (type == "double")
		{
			if (!obj.isReal()) return false;
			out = new double(obj.asReal()); // allocate if needed for passing by pointer
			return true;
		}
		else if (type == "string")
		{
			if (!obj.isString()) return false;
			out = const_cast<char*>(obj.asString().c_str()); // or asCString()
			return true;
		}
		else if (type == "pointer")
		{
			if (!obj.isPointer()) return false;
			out = obj.asPointer();
			return true;
		}
		// Add more types as needed
		return false;
	}
	*/

	// should be able to pass in some parameters
	// like 'skip whitespace'
	// we'll default to skipping whitespace, other than reporting a newline
	static inline bool nextPreprocToken(OctetCursor& src, PrepToken& tok)
	{
		using CC = PSCharClass;

		// Skip and emit whitespace
		if (!src.empty() && CC::isWhitespace(*src.fStart)) {
			const uint8_t* start = src.fStart;
			while (!src.empty() && CC::isWhitespace(*src.fStart) && *src.fStart != '\n')
				++src.fStart;
			tok.type = PrepTokenType::Whitespace;
			tok.span = OctetCursor(start, src.fStart - start);
			//return true;
		}

		// Newline
		if (!src.empty() && *src.fStart == '\n') {
			tok.type = PrepTokenType::Newline;
			tok.span = OctetCursor(src.fStart, 1);
			++src.fStart;
			return true;
		}

		// End of input
		if (src.empty()) {
			tok.type = PrepTokenType::EndOfFile;
			tok.span = src;
			return false;
		}

		const uint8_t* start = src.fStart;
		uint8_t c = *start;

		// Preprocessor directive
		if (c == '#') {
			++src.fStart;
			skipWhile(src, PS_WHITESPACE);
			//OctetCursor identStart = src.fStart;
			skipWhile(src, PS_NAME_CHAR);
			tok.type = PrepTokenType::Directive;
			tok.span = OctetCursor(start, src.fStart - start);
			return true;
		}

		// Comment: //...
		if (c == '/' && src.peek(1) == '/') {
			src.fStart += 2;
			const uint8_t* begin = start;
			while (!src.empty() && *src.fStart != '\n') ++src.fStart;
			tok.type = PrepTokenType::Comment;
			tok.span = OctetCursor(begin, src.fStart - begin);
			return true;
		}

		// Comment: /* ... */
		if (c == '/' && src.peek(1) == '*') {
			src.fStart += 2;
			const uint8_t* begin = start;
			while (src.fStart + 1 < src.fEnd &&
				!(src.fStart[0] == '*' && src.fStart[1] == '/'))
			{
				++src.fStart;
			}
			if (src.fStart + 1 < src.fEnd)
				src.fStart += 2;
			tok.type = PrepTokenType::Comment;
			tok.span = OctetCursor(begin, src.fStart - begin);
			return true;
		}

		// String literal
		if (c == '"') {
			const uint8_t* p = ++src.fStart;
			while (!src.empty()) {
				if (*src.fStart == '\\') {
					++src.fStart;
					if (!src.empty()) ++src.fStart;
				}
				else if (*src.fStart == '"') {
					++src.fStart;
					break;
				}
				else {
					++src.fStart;
				}
			}
			tok.type = PrepTokenType::String;
			tok.span = OctetCursor(start, src.fStart - start);
			return true;
		}

		// Char literal
		if (c == '\'') {
			const uint8_t* p = ++src.fStart;
			while (!src.empty()) {
				if (*src.fStart == '\\') {
					++src.fStart;
					if (!src.empty()) ++src.fStart;
				}
				else if (*src.fStart == '\'') {
					++src.fStart;
					break;
				}
				else {
					++src.fStart;
				}
			}
			tok.type = PrepTokenType::CharConst;
			tok.span = OctetCursor(start, src.fStart - start);
			return true;
		}

		// Identifier
		if (std::isalpha(c) || c == '_') {
			skipWhile(src, PS_NAME_CHAR);
			tok.type = PrepTokenType::Identifier;
			tok.span = OctetCursor(start, src.fStart - start);
			return true;
		}

		// Number
		if (CC::isNumericBegin(c)) {
			const uint8_t* p = src.fStart++;
			while (!src.empty() && (CC::isNumeric(*src.fStart) || *src.fStart == '.'))
				++src.fStart;
			tok.type = PrepTokenType::Number;
			tok.span = OctetCursor(start, src.fStart - start);
			return true;
		}

		// Multi-char punctuators
		static const char* multiPunct[] = {
			"...", ">>=", "<<=", "->", "++", "--", "==", "!=", "<=", ">=",
			"&&", "||", "<<", ">>", "+=", "-=", "*=", "/=", "%=", "&=", "^=", "|=", "##", "::", nullptr
		};

		for (const char** p = multiPunct; *p; ++p) {
			size_t len = std::strlen(*p);
			if (src.size() >= len && std::memcmp(src.fStart, *p, len) == 0) {
				tok.type = PrepTokenType::Punctuator;
				tok.span = OctetCursor(src.fStart, len);
				src.skip(len);
				return true;
			}
		}

		// Single-char punctuator
		tok.type = PrepTokenType::Punctuator;
		tok.span = OctetCursor(src.fStart, 1);
		src.skip(1);
		return true;
	}


	static inline bool parseFunctionDeclaration(OctetCursor& src, PSDictionary& out)
	{
		using namespace waavs;
		PrepToken tok;

		// 1. Return type
		if (!nextPreprocToken(src, tok) || tok.type != PrepTokenType::Identifier)
			return false;

		PSName returnType((const char *)tok.span.data(), tok.span.size());

        // skip whitespace
        //skipWhile(src, PS_WHITESPACE);

		// 2. Function name
		if (!nextPreprocToken(src, tok) || tok.type != PrepTokenType::Identifier)
			return false;

		PSName funcName((const char *)tok.span.data(), tok.span.size());

		// 3. Open paren
		if (!nextPreprocToken(src, tok) || tok.type != PrepTokenType::Punctuator || tok.span != "(")
			return false;

		// 4. Parameter list
		auto paramArray = PSArray::create();

		while (true) {
			if (!nextPreprocToken(src, tok))
			{
				// incomplete, because we did not already see ')'
				return false;
			}

			if (tok.type == PrepTokenType::Punctuator)
			{
				if (tok.span == ")")
					break;  // end of param list

				// consume the ',', continue parsing
				if (tok.span == ",")
					continue;
			}

			if (tok.type != PrepTokenType::Identifier)
				return false;  // expected param type

			PSName paramType((const char *)tok.span.data(), tok.span.size());

			if (!nextPreprocToken(src, tok) || tok.type != PrepTokenType::Identifier)
				return false;  // expected param name

			PSName paramName((const char *)tok.span.data(), tok.span.size());

			// Add to param array
			paramArray->append(PSObject::fromName(paramName));
			paramArray->append(PSObject::fromName(paramType));

			//paramArray->append(PSObject::fromArray(argArray));
		}

		// 5. Semicolon
		if (!nextPreprocToken(src, tok) || tok.type != PrepTokenType::Punctuator || tok.span != ";")
			return false;

		// Success: fill out dictionary
		out.put(PSName("name"), PSObject::fromName(funcName));
		out.put(PSName("returnType"), PSObject::fromName(returnType));
		out.put(PSName("params"), PSObject::fromArray(paramArray));

		return true;
	}

} // namespace waavs

