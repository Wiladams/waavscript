#pragma once
#include <charconv>
#include <system_error>
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <limits>
#include <vector>
#include <utility>


#include "pscore.h"
#include "ps_lex_tokenizer.h"
#include "typeconv.h"

namespace waavs {
    using PSNameResolver = bool (*)(void* context, const PSName& name, PSObject& out);

    enum class PSScanStatus { Object, Eof, ProcedureEnd, SyntaxError, LimitCheck, Undefined };

    struct PSObjectGenState {
        PSLexGenState fLexState;
        void* fResolverContext = nullptr;
        PSNameResolver fResolver = nullptr;
        PSScanStatus fLastStatus = PSScanStatus::Eof;

        PSObjectGenState(const PSFileHandle file, void* context, PSNameResolver resolver)
            : fLexState(file), fResolverContext(context), fResolver(resolver) {}
    };

    inline bool resolveNameImmediately(PSObjectGenState& state, const PSName& name, PSObject& out) {
        return state.fResolver && state.fResolver(state.fResolverContext, name, out);
    }

    static inline bool spanToString(OctetCursor oc, std::vector<uint8_t>& result) {
        const uint8_t* src = oc.data();
        const size_t len = oc.size();
        result.clear();
        result.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            uint8_t ch = src[i];
            if (ch == '\\') {
                if (++i == len) break;
                const uint8_t next = src[i];
                switch (next) {
                case 'n': result.push_back('\n'); break;
                case 'r': result.push_back('\r'); break;
                case 't': result.push_back('\t'); break;
                case 'b': result.push_back('\b'); break;
                case 'f': result.push_back('\f'); break;
                case '\n': break;
                case '\r': if (i + 1 < len && src[i + 1] == '\n') ++i; break;
                default:
                    if (next >= '0' && next <= '7') {
                        unsigned value = next - '0';
                        for (unsigned j = 0; j < 2 && i + 1 < len && src[i + 1] >= '0' && src[i + 1] <= '7'; ++j)
                            value = (value << 3) | (src[++i] - '0');
                        result.push_back(static_cast<uint8_t>(value));
                    }
                    else result.push_back(next);
                    break;
                }
            }
            else if (ch == '\r') {
                if (i + 1 < len && src[i + 1] == '\n') ++i;
                result.push_back('\n');
            }
            else result.push_back(ch);
        }
        return true;
    }

    static inline int psHexDigit(uint8_t c) noexcept {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
    }

    static inline bool spanToHexString(OctetCursor src, std::vector<uint8_t>& out) {
        out.clear();
        int high = -1;
        for (const uint8_t* p = src.begin(); p < src.end(); ++p) {
            if (PSCharClass::isWhitespace(*p)) continue;
            int d = psHexDigit(*p);
            if (d < 0) return false;
            if (high < 0) high = d;
            else { out.push_back(static_cast<uint8_t>((high << 4) | d)); high = -1; }
        }
        if (high >= 0) out.push_back(static_cast<uint8_t>(high << 4));
        return true;
    }

    static inline bool spanToASCII85String(OctetCursor src, std::vector<uint8_t>& out) {
        out.clear();
        uint64_t value = 0;
        unsigned count = 0;
        for (const uint8_t* p = src.begin(); p < src.end(); ++p) {
            const uint8_t c = *p;
            if (PSCharClass::isWhitespace(c)) continue;
            if (c == 'z') {
                if (count != 0) return false;
                out.insert(out.end(), 4, 0);
                continue;
            }
            if (c < '!' || c > 'u') return false;
            value = value * 85 + static_cast<unsigned>(c - '!');
            if (++count == 5) {
                if (value > UINT32_MAX) return false;
                for (int shift = 24; shift >= 0; shift -= 8) out.push_back(static_cast<uint8_t>(value >> shift));
                value = 0;
                count = 0;
            }
        }
        if (count == 1) return false;
        if (count > 1) {
            const unsigned original = count;
            while (count++ < 5) value = value * 85 + 84;
            if (value > UINT32_MAX) return false;
            for (unsigned i = 0; i < original - 1; ++i) out.push_back(static_cast<uint8_t>(value >> (24 - 8 * i)));
        }
        return true;
    }

    struct PSParsedNumber {
        enum class Kind { NotNumber, Integer, Real, LimitCheck } kind = Kind::NotNumber;
        int32_t integer = 0;
        double real = 0;
    };

    static inline PSParsedNumber parsePSNumber(OctetCursor token) noexcept
    {
        PSParsedNumber out;

        const uint8_t* begin = token.begin();
        const uint8_t* end = token.end();

        if (begin == end)
            return out;

        const char* first = reinterpret_cast<const char*>(begin);
        const char* last = reinterpret_cast<const char*>(end);

        // ------------------------------------------------------
        // Radix integers: base#digits
        // ------------------------------------------------------

        if (*begin >= '0' && *begin <= '9') {
            const uint8_t* p = begin;
            unsigned base = 0;

            while (p < end && *p >= '0' && *p <= '9') {
                if (base <= 36)
                    base = base * 10 + (*p - '0');
                ++p;
            }

            if (p < end && *p == '#') {
                if (base < 2 || base > 36 || p + 1 == end)
                    return out;

                const char* digits = reinterpret_cast<const char*>(p + 1);
                uint32_t value = 0;

                auto result = std::from_chars(digits, last, value, static_cast<int>(base));

                if (result.ec == std::errc::result_out_of_range) {
                    out.kind = PSParsedNumber::Kind::LimitCheck;
                    return out;
                }

                if (result.ec != std::errc{} || result.ptr != last)
                    return out;

                // Preserve existing 32-bit two's-complement semantics.
                const int64_t signedValue = value <= INT32_MAX
                    ? static_cast<int64_t>(value)
                    : static_cast<int64_t>(value) - 4294967296LL;

                out.integer = static_cast<int32_t>(signedValue);
                out.kind = PSParsedNumber::Kind::Integer;
                return out;
            }
        }

        // ------------------------------------------------------
        // Decimal integers and reals
        // ------------------------------------------------------

        const uint8_t* p = begin;

        if (*p == '+' || *p == '-')
            ++p;

        if (p == end)
            return out;

        const uint8_t* integerStart = p;

        while (p < end && *p >= '0' && *p <= '9')
            ++p;

        const bool hasIntegerDigits = p != integerStart;
        bool hasDot = false;
        bool hasExponent = false;

        if (p < end && *p == '.') {
            hasDot = true;
            ++p;

            const uint8_t* fractionStart = p;

            while (p < end && *p >= '0' && *p <= '9')
                ++p;

            if (!hasIntegerDigits && p == fractionStart)
                return out;
        }
        else if (!hasIntegerDigits) {
            return out;
        }

        if (p < end && (*p == 'e' || *p == 'E')) {
            hasExponent = true;
            ++p;

            if (p < end && (*p == '+' || *p == '-'))
                ++p;

            const uint8_t* exponentStart = p;

            while (p < end && *p >= '0' && *p <= '9')
                ++p;

            if (p == exponentStart)
                return out;
        }

        // The entire token must match the numeric grammar.
        if (p != end)
            return out;

        // ------------------------------------------------------
        // Fast path: decimal integer
        // ------------------------------------------------------

        if (!hasDot && !hasExponent) {
            int32_t integer = 0;

            // std::from_chars does not accept a leading '+'.
            const char* intFirst = first;
            if (*intFirst == '+')
                ++intFirst;

            auto result = std::from_chars(intFirst, last, integer, 10);

            if (result.ec == std::errc{} && result.ptr == last) {
                out.kind = PSParsedNumber::Kind::Integer;
                out.integer = integer;
                return out;
            }

            // A decimal integer outside the supported integer range
            // may still be a representable PostScript real.
            if (result.ec != std::errc::result_out_of_range)
                return out;
        }

        // ------------------------------------------------------
        // Real conversion
        // ------------------------------------------------------

        // std::from_chars does not accept a leading '+' for floating point.
        const char* realFirst = first;

        if (*realFirst == '+')
            ++realFirst;

        double real = 0.0;

        auto result = std::from_chars(realFirst, last, real, std::chars_format::general);

        if (result.ec == std::errc::result_out_of_range) {
            out.kind = PSParsedNumber::Kind::LimitCheck;
            return out;
        }

        if (result.ec != std::errc{} || result.ptr != last)
            return out;

        out.kind = PSParsedNumber::Kind::Real;
        out.real = real;
        return out;
    }

    static inline PSScanStatus objectFromLexStatus(const PSLexeme& lex, PSObject& obj) {
        switch (lex.type) {
        case PSLexType::Whitespace:
        case PSLexType::Comment:
        case PSLexType::DSCComment: return PSScanStatus::SyntaxError; // caller skips them
        case PSLexType::LiteralName:
            obj.resetFromName(lex.span);
            return PSScanStatus::Object;
        case PSLexType::Name:
        case PSLexType::Number: {
            if (lex.span.empty()) return PSScanStatus::SyntaxError;
            if (!PSCharClass::isNumericBegin(*lex.span)) {
                obj.resetFromName(lex.span);
                obj.setExecutable(true);
                return PSScanStatus::Object;
            }
            const PSParsedNumber num = parsePSNumber(lex.span);
            if (num.kind == PSParsedNumber::Kind::Integer) obj.resetFromInt(num.integer);
            else if (num.kind == PSParsedNumber::Kind::Real) obj.resetFromReal(num.real);
            else if (num.kind == PSParsedNumber::Kind::LimitCheck) return PSScanStatus::LimitCheck;
            else { obj.resetFromName(lex.span); obj.setExecutable(true); }
            return PSScanStatus::Object;
        }
        case PSLexType::ImmediateName:
            obj.resetFromName(lex.span);
            obj.setSystemOp(true); // transient syntax marker, resolved in nextPSObjectStatus
            return PSScanStatus::Object;
        case PSLexType::String:
        case PSLexType::HexString:
        case PSLexType::ASCII85String: {
            std::vector<uint8_t> decoded;
            const bool valid = lex.type == PSLexType::String ? spanToString(lex.span, decoded) :
                lex.type == PSLexType::HexString ? spanToHexString(lex.span, decoded) : spanToASCII85String(lex.span, decoded);
            if (!valid) return PSScanStatus::SyntaxError;
            obj.resetFromString(PSString::fromVector(decoded));
            return PSScanStatus::Object;
        }
        case PSLexType::LBRACKET:
        case PSLexType::RBRACKET:
        case PSLexType::LLANGLE:
        case PSLexType::RRANGLE:
            obj.resetFromName(lex.span);
            obj.setExecutable(true);
            return PSScanStatus::Object;
        case PSLexType::RBRACE: return PSScanStatus::ProcedureEnd;
        case PSLexType::Eof: return PSScanStatus::Eof;
        default: return PSScanStatus::SyntaxError;
        }
    }

    static inline bool objectFromLex(PSLexeme lex, PSObject& obj) {
        return objectFromLexStatus(lex, obj) == PSScanStatus::Object;
    }

    static PSScanStatus nextPSObjectStatus(PSObjectGenState& state, PSObject& obj, bool inProcedure = false);

    static PSScanStatus scanProcedureStatus(PSObjectGenState& state, PSObject& out) {
        auto arr = PSArray::create();
        for (;;) {
            PSObject element;
            PSScanStatus status = nextPSObjectStatus(state, element, true);
            if (status == PSScanStatus::ProcedureEnd) break;
            if (status == PSScanStatus::Eof) return PSScanStatus::SyntaxError;
            if (status != PSScanStatus::Object) return status;
            if (!arr->append(element)) return PSScanStatus::LimitCheck;
        }
        out.resetFromArray(arr);
        out.setExecutable(true);
        return PSScanStatus::Object;
    }

    static PSScanStatus nextPSObjectStatus(PSObjectGenState& state, PSObject& obj, bool inProcedure) {
        PSLexeme lex;
        while (nextPSLexeme(state.fLexState, lex)) {
            if (lex.type == PSLexType::Comment || lex.type == PSLexType::DSCComment || lex.type == PSLexType::Whitespace) continue;
            if (lex.type == PSLexType::LBRACE) return scanProcedureStatus(state, obj);
            if (lex.type == PSLexType::RBRACE) return inProcedure ? PSScanStatus::ProcedureEnd : PSScanStatus::SyntaxError;
            PSScanStatus status = objectFromLexStatus(lex, obj);
            if (status != PSScanStatus::Object) return status;
            if (obj.isSystemOp()) {
                PSObject resolved;
                if (!resolveNameImmediately(state, obj.asName(), resolved)) return PSScanStatus::Undefined;
                obj = std::move(resolved);
            }
            return PSScanStatus::Object;
        }
        return lex.type == PSLexType::Eof ? PSScanStatus::Eof : PSScanStatus::SyntaxError;
    }

    static inline bool scanProcedure(PSObjectGenState& state, PSObject& out) {
        state.fLastStatus = scanProcedureStatus(state, out);
        return state.fLastStatus == PSScanStatus::Object;
    }

    static inline bool nextPSObject(PSObjectGenState& state, PSObject& obj) {
        state.fLastStatus = nextPSObjectStatus(state, obj);
        return state.fLastStatus == PSScanStatus::Object;
    }
}
