#pragma once

#include <functional>

#include "ps_lexer.h"
#include "typeconv.h"
#include "pscore.h"


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






namespace waavs 
{

    inline bool transformTokenToPSObject(const PSToken& tok, PSObject &obj) 
    {
        switch (tok.type) {
        case PSTokenType::PS_TOKEN_Boolean:
            return obj.resetFromBool(tok.boolValue);

        case PSTokenType::PS_TOKEN_Number:
            // Use int if the number is whole
            if (tok.numberValue == static_cast<int32_t>(tok.numberValue))
                return obj.resetFromInt(static_cast<int32_t>(tok.numberValue));
            else
                return obj.resetFromReal(tok.numberValue);

        case PSTokenType::PS_TOKEN_LiteralName:
            return obj.resetFromName(tok.span);

        case PSTokenType::PS_TOKEN_ExecutableName:
            return obj.resetFromName(tok.span); // Executability will be resolved later

        case PSTokenType::PS_TOKEN_String: {
            auto* str = new PSString(tok.span.size());
            for (size_t i = 0; i < tok.span.size(); ++i)
                str->put(i, tok.span.data()[i]);
            return obj.resetFromString(str);
        }

        case PSTokenType::PS_TOKEN_HexString: {
            // Remove whitespace, decode hex pairs
            const uint8_t* p = tok.span.data();
            const uint8_t* end = p + tok.span.size();
            std::vector<uint8_t> bytes;
            while (p < end) {
                while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) ++p;
                if (p >= end) break;
                uint8_t hi = *p++;
                while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) ++p;
                uint8_t lo = (p < end) ? *p++ : '0'; // pad with 0 if odd

                auto decodeHex = [](uint8_t c) -> uint8_t {
                    if (c >= '0' && c <= '9') return c - '0';
                    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                    return 0;
                    };
                bytes.push_back((decodeHex(hi) << 4) | decodeHex(lo));
            }
            auto* str = new PSString(bytes.size());
            for (size_t i = 0; i < bytes.size(); ++i)
                str->put(i, bytes[i]);

            return obj.resetFromString(str);
        }

        case PSTokenType::PS_TOKEN_Null:
            return obj.reset();


        case PSTokenType::PS_TOKEN_Mark:
            return obj.resetFromMark();

        default:
            return obj.reset();
        }
    }

} // namespace waavs

namespace waavs {
    static inline bool parseObject(PSTokenGenerator& tokgen, PSObject& out);

    static inline bool parseArray(PSTokenGenerator& tokgen, PSObject& out, bool executable) {
        auto* arr = new PSArray();
        arr->setExecutable(executable);
        PSObject element;

        while (true) {
            PSToken tok;
            if (!tokgen.next(tok)) break;

            if ((executable && tok.type == PSTokenType::PS_TOKEN_ProcEnd) ||
                (!executable && tok.type == PSTokenType::PS_TOKEN_ArrayEnd)) {
                break;
            }

            if (tok.type == PSTokenType::PS_TOKEN_ProcBegin) {
                PSObject subProc;
                if (!parseArray(tokgen, subProc, true)) return false;
                arr->append(subProc);
            }
            else if (tok.type == PSTokenType::PS_TOKEN_ArrayBegin) {
                PSObject subArray;
                if (!parseArray(tokgen, subArray, false)) return false;
                arr->append(subArray);
            }
            else {
                if (!transformTokenToPSObject(tok, element)) return false;
                arr->append(element);
            }
        }

        return out.resetFromArray(arr);

    }

    static inline bool parseObject(PSTokenGenerator& tokgen, PSObject& out) {
        PSToken tok;
        if (!tokgen.next(tok)) return false;

        if (tok.type == PSTokenType::PS_TOKEN_ProcBegin)
            return parseArray(tokgen, out, true);
        if (tok.type == PSTokenType::PS_TOKEN_ArrayBegin)
            return parseArray(tokgen, out, false);

        return transformTokenToPSObject(tok, out);
    }


}

