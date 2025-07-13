#pragma once

#include <functional>

#include "pscore.h"
#include "ps_lexer.h"
#include "typeconv.h"

//
// The scanner's job is to convert a stream of lexemes into a stream of PSObjects
// Chapter 2 of Blue Book
//

namespace waavs
{


    static bool spanToString(OctetCursor oc, std::vector<uint8_t>& result)
    {
        const char* src = (const char *)oc.data();
        size_t len = oc.size();

        result.reserve(len); // worst case

        for (size_t i = 0; i < len; ++i) {
            char ch = src[i];
            if (ch == '\\' && i + 1 < len) {
                char next = src[++i];

                switch (next) {
                case 'n': result.push_back('\n'); break;
                case 'r': result.push_back('\r'); break;
                case 't': result.push_back('\t'); break;
                case 'b': result.push_back('\b'); break;
                case 'f': result.push_back('\f'); break;
                case '\\': result.push_back('\\'); break;
                case '(': result.push_back('('); break;
                case ')': result.push_back(')'); break;
                default:
                    if (next >= '0' && next <= '7') {
                        // Octal escape: up to 3 digits
                        int val = next - '0';
                        int count = 1;

                        while (count < 3 && i + 1 < len && src[i + 1] >= '0' && src[i + 1] <= '7') {
                            val = (val << 3) + (src[++i] - '0');
                            ++count;
                        }
                        result.push_back(static_cast<uint8_t>(val));
                    }
                    else {
                        // Unknown escape, just keep literal
                        result.push_back(next);
                    }
                    break;
                }
            }
            else {
                result.push_back(static_cast<uint8_t>(ch));
            }
        }

        return true;
    }


    static bool spanToHexString(OctetCursor src, std::vector<uint8_t>& out) noexcept
    {
        out.clear();

        while (!src.empty()) {
            skipWhile(src, PS_WHITESPACE);
            if (src.empty()) break;

            uint8_t hiChar = *src;
            ++src;
            skipWhile(src, PS_WHITESPACE);

            uint8_t loChar = src.empty() ? '0' : *src;
            ++src;

            uint8_t hi, lo;
            if (!decodeHex(hiChar, hi) || !decodeHex(loChar, lo))
                return false;

            out.push_back((hi << 4) | lo);
        }

        return true;
    }


    static inline bool objectFromLex(PSLexeme lex, PSObject& obj)
    {
        switch (lex.type) {
            // In the case of whitespace, we'll return a null object
        case PSLexType::Whitespace:
        case PSLexType::Comment:
        case PSLexType::DSCComment:
            obj.reset();
            return true;

		case PSLexType::RBRACE: // } - end of procedure
            printf("Seeing RBRACE in objectFromLex\n");
            obj.reset();
            return true;

        case PSLexType::LiteralName:
            return obj.resetFromName(lex.span);

        case PSLexType::Name: {
            // Recognize built-in constants
            if (lex.span == "true") {
                return obj.resetFromBool(true);
            }
            else if (lex.span == "false") {
                return obj.resetFromBool(false);
            }
            else if (lex.span == "null") {
                return obj.reset();
            }
            else {
                // Otherwise, treat it as an executable name
                if (!obj.resetFromName(lex.span)) return false;
                obj.setExecutable(true);
                return true;
            }
        }

        case PSLexType::SystemName:
        {
            obj.resetFromName(lex.span);
            obj.setExecutable(true); // System names are always executable
            obj.setSystemOp(true);
            return true;
        }


        case PSLexType::Number:     // 123.456
        {
            double value = 0.0;
            if (readDecimal(lex.span, value)) {
				// if it's an integer, we can reset it to an integer
                if (value == static_cast<int64_t>(value)) {
                    return obj.resetFromInt(static_cast<int64_t>(value));
				}
                return obj.resetFromReal(value);
            }
            else {
                return false; // Invalid number format
            }
        }

        case PSLexType::String: {    // (abc)
            // BUGBUG - here we can do string decoding, or perhaps we leave
            // that to the PSString class?
            std::vector<uint8_t> decoded;
            if (!spanToString(lex.span, decoded)) {
                return false; // Invalid string format
            }

            return obj.resetFromString(PSString::fromVector(decoded));
        }

        case PSLexType::HexString: { // <48656C6C6F>
            std::vector<uint8_t> decoded;
            if (!spanToHexString(lex.span, decoded))
                return false;
            PSString str = PSString::fromVector(decoded);
            return obj.resetFromString(str);
        }

        // The default case is to return anything not already identified
        // as an executable name
        default:
            if (!obj.resetFromName(lex.span)) return false;
            obj.setExecutable(true);
            return true;

            // These have operators named after them to be dealt with
            // op_arraybegin, op_arrayend, op_dictbegin, op_dictend
            //case PSLexType::LBRACKET:   // [
            //case PSLexType::RBRACKET:   // ]
            //case PSLexType::LLANGLE:    // <<
            //case PSLexType::RRANGLE:    // >>
        }


        return false;
    }

    static inline bool nextPSObject(PSLexemeGenerator& lexgen, PSObject& obj);


    // Scan a procedure, which is a sequence of tokens that ends with a ProcEnd token.
    static bool scanProcedure(PSLexemeGenerator& lexgen, PSObject& out)
    {
        auto arr = PSArray::create();

        while (true) {
            PSObject element;

			if (!nextPSObject(lexgen, element)) 
                return false;   // unterminated procedure

            // We get a null object either at procEnd, or end of input
            if (element.isNull()) {
                // If we hit a null object, we break
                break;
			}

            arr->append(element);
        }

        out.resetFromArray(arr);
        out.setExecutable(true); // mark the procedure as executable

        return true;
    }


    // Take the stream of lexemes, and return the next PSObject from there
    //
    static inline bool nextPSObject(PSLexemeGenerator& lexgen, PSObject &obj) 
    {
        PSLexeme lex;
        while (lexgen.next(lex)) {

            switch (lex.type) {
                // Just consume whitespace
                case PSLexType::Whitespace:
                case PSLexType::Comment:
                case PSLexType::DSCComment:
                    continue; // Skip

                case PSLexType::Eof:
				    return obj.reset(); // Reset the object to null on EOF

				case PSLexType::LBRACE: // {
                    return scanProcedure(lexgen, obj);

                case PSLexType::RBRACE: // }
                    return obj.reset();

                // The default case is to return anything not already identified
                // as an executable name
                default:
				    return objectFromLex(lex, obj);
            }
        }

        return false;
    }

    struct PSObjectGenerator
    {
        PSLexemeGenerator lexgen;
        explicit PSObjectGenerator(const OctetCursor &oc) : lexgen(oc) {}

        bool next(PSObject& obj) 
        {
            return nextPSObject(lexgen, obj);
        }


	};


}

