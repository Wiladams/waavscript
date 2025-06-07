#pragma once

#include <functional>

#include "pscore.h"
#include "ps_lexer.h"
#include "typeconv.h"



namespace waavs
{
    enum class PSTokenType : uint8_t
    {
        PS_TOKEN_Invalid = 0,
        PS_TOKEN_EOI,                // End of input
        PS_TOKEN_Boolean,            // true / false
        PS_TOKEN_Number,             // Any numeric value
        PS_TOKEN_LiteralName,        // /foo
        PS_TOKEN_ExecutableName,     // foo
        PS_TOKEN_String,             // (abc)
        PS_TOKEN_HexString,          // <48656C6C6F> 
        PS_TOKEN_ProcBegin,          // {
        PS_TOKEN_ProcEnd,            // }
        PS_TOKEN_ArrayBegin,         // [
        PS_TOKEN_ArrayEnd,           // ]
        PS_TOKEN_DictBegin,          // << Dictionary begin
        PS_TOKEN_DictEnd,            // >> Dictionary end
        PS_TOKEN_Operator,           // Bound operator (optional at scan time)
        PS_TOKEN_Mark,               // e.g. from 'mark' operator
        PS_TOKEN_Null                // 'null'
    };


    struct PSToken 
    {
        PSTokenType type = PSTokenType::PS_TOKEN_Invalid;
        OctetCursor span; // Original input
        double numberValue = 0.0; // only valid for Number
        bool boolValue = false;   // only valid for Boolean

        inline void reset(PSTokenType t, const OctetCursor &oc) 
        {
            type = t;
            span = oc;
            numberValue = 0.0;
            boolValue = false;
		}

        inline void reset(PSTokenType t, const OctetCursor& oc, bool bValue)
        {
            type = t;
            span = oc;
            numberValue = 0.0;
            boolValue = bValue;
        }

        inline void reset(PSTokenType t, const OctetCursor& oc, double numValue)
        {
            type = t;
            span = oc;
            numberValue = numValue;
            boolValue = false;
		}
    };

    static inline bool nextPSToken(PSLexemeGenerator& lexgen, PSToken& out) 
    {
        PSLexeme lex;
        while (lexgen.next(lex)) {
            switch (lex.type) {
            case PSLexType::Whitespace:
            case PSLexType::Comment:
                continue; // Skip

            case PSLexType::LiteralName:
				out.reset(PSTokenType::PS_TOKEN_LiteralName, lex.span);
                return true;

            case PSLexType::Name: {
                // Recognize built-in constants
                if (lex.span == "true") {
					out.reset(PSTokenType::PS_TOKEN_Boolean, lex.span, true);
                    return true;
                }
                else if (lex.span == "false") {
                    out.reset(PSTokenType::PS_TOKEN_Boolean, lex.span, false);
                    return true;
                }
                else if (lex.span == "null") {
                    out.reset(PSTokenType::PS_TOKEN_Null, lex.span);
                    return true;
                }
                else {
					out.reset(PSTokenType::PS_TOKEN_ExecutableName, lex.span);
                    return true;
                }
            }

            case PSLexType::Number: {
                double value = 0.0;
                if (readDecimal(lex.span, value)) {
					out.reset(PSTokenType::PS_TOKEN_Number, lex.span, value);
                    return true;
                }
                else {
					out.reset(PSTokenType::PS_TOKEN_Invalid, lex.span);
                    return true;
                }
            }

            case PSLexType::String:
				out.reset(PSTokenType::PS_TOKEN_String, lex.span);
                return true;

			case PSLexType::HexString:
				out.reset(PSTokenType::PS_TOKEN_HexString, lex.span);
				return true;

            case PSLexType::ProcBegin:
				out.reset(PSTokenType::PS_TOKEN_ProcBegin, lex.span);

                return true;

            case PSLexType::ProcEnd:
				out.reset(PSTokenType::PS_TOKEN_ProcEnd, lex.span);
                return true;

            case PSLexType::ArrayBegin:
				out.reset(PSTokenType::PS_TOKEN_ArrayBegin, lex.span);
                return true;

            case PSLexType::ArrayEnd:
				out.reset(PSTokenType::PS_TOKEN_ArrayEnd, lex.span);
                return true;

            case PSLexType::DictBegin:
                out.reset(PSTokenType::PS_TOKEN_DictBegin, lex.span);
                return true;

            case PSLexType::DictEnd:
                out.reset(PSTokenType::PS_TOKEN_DictEnd, lex.span);
                return true;

            default:
				out.reset(PSTokenType::PS_TOKEN_Invalid, lex.span);
                return true;
            }
        }

		out.reset(PSTokenType::PS_TOKEN_EOI, OctetCursor()); // End of input

        return false;
    }

    struct PSTokenGenerator
    {
        PSLexemeGenerator lexgen;
        explicit PSTokenGenerator(const OctetCursor &oc) : lexgen(oc) {}

        bool next(PSToken& tok) 
        {
            return nextPSToken(lexgen, tok);
        }


	};
}

