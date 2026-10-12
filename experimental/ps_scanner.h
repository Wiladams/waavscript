#pragma once
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <limits>
#include <vector>
#include <utility>

#include "pscore.h"
#include "ps_lex_tokenizer.h"
// "typeconv.h"

namespace waavs {
    static inline bool psCouldBeNumber(OctetCursor span) noexcept
    {
        if (span.empty())
            return false;

        uint8_t c = *span;

        return (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.';
    }

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

    static inline PSParsedNumber parsePSNumber(OctetCursor token) noexcept {
        PSParsedNumber out;
        const uint8_t* p = token.begin();
        const uint8_t* end = token.end();
        if (p == end) return out;
        // Radix notation has no leading sign. Parse to a 32-bit bit pattern.
        const uint8_t* radix = p;
        unsigned base = 0;
        while (radix < end && PSCharClass::isDigit(*radix)) {
            if (base <= 100) base = base * 10 + (*radix - '0');
            ++radix;
        }
        if (radix > p && radix < end && *radix == '#') {
            if (base < 2 || base > 36 || radix + 1 == end) return out;
            uint64_t value = 0;
            for (const uint8_t* q = radix + 1; q < end; ++q) {
                const uint8_t c = *q;
                unsigned digit = c >= '0' && c <= '9' ? c - '0' :
                    c >= 'A' && c <= 'Z' ? c - 'A' + 10 :
                    c >= 'a' && c <= 'z' ? c - 'a' + 10 : 255;
                if (digit >= base) return out;
                value = value * base + digit;
                if (value > UINT32_MAX) { out.kind = PSParsedNumber::Kind::LimitCheck; return out; }
            }
            const uint32_t bits = static_cast<uint32_t>(value);
            out.integer = bits <= INT32_MAX ? static_cast<int32_t>(bits) : static_cast<int32_t>(static_cast<int64_t>(bits) - 4294967296LL);
            out.kind = PSParsedNumber::Kind::Integer;
            return out;
        }
        bool negative = false;
        if (*p == '+' || *p == '-') { negative = *p == '-'; ++p; }
        const uint8_t* integral = p;
        long double value = 0;
        while (p < end && PSCharClass::isDigit(*p)) { value = value * 10 + (*p++ - '0'); }
        const bool haveIntegerDigits = p != integral;
        bool dot = false;
        bool exponent = false;
        if (p < end && *p == '.') {
            dot = true;
            ++p;
            const uint8_t* fraction = p;
            long double place = 0.1L;
            while (p < end && PSCharClass::isDigit(*p)) {
                value += (*p++ - '0') * place;
                place *= 0.1L;
            }
            if (!haveIntegerDigits && p == fraction) return out;
        }
        else if (!haveIntegerDigits) return out;
        int expValue = 0;
        bool expNegative = false;
        if (p < end && (*p == 'e' || *p == 'E')) {
            exponent = true;
            ++p;
            if (p < end && (*p == '+' || *p == '-')) { expNegative = *p == '-'; ++p; }
            if (p == end || !PSCharClass::isDigit(*p)) return out;
            while (p < end && PSCharClass::isDigit(*p)) {
                if (expValue < 100000) expValue = expValue * 10 + (*p - '0');
                ++p;
            }
        }
        if (p != end) return out;
        if (negative) value = -value;
        if (exponent) value *= std::pow(10.0L, expNegative ? -static_cast<long double>(expValue) : static_cast<long double>(expValue));
        // The VM stores realtype as double; it can choose a stricter real range separately.
        if (!std::isfinite(value) || std::fabs(value) > static_cast<long double>(std::numeric_limits<double>::max())) {
            out.kind = PSParsedNumber::Kind::LimitCheck;
            return out;
        }
        if (!dot && !exponent && value >= INT32_MIN && value <= INT32_MAX) {
            out.kind = PSParsedNumber::Kind::Integer;
            out.integer = static_cast<int32_t>(value);
        }
        else {
            out.kind = PSParsedNumber::Kind::Real;
            out.real = static_cast<double>(value);
        }
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
        case PSLexType::Number:
        {
            if (psCouldBeNumber(lex.span)) {
                const PSParsedNumber num = parsePSNumber(lex.span);

                switch (num.kind) {
                case PSParsedNumber::Kind::Integer:
                    obj.resetFromInt(num.integer);
                    return PSScanStatus::Object;

                case PSParsedNumber::Kind::Real:
                    obj.resetFromReal(num.real);
                    return PSScanStatus::Object;

                case PSParsedNumber::Kind::LimitCheck:
                    return PSScanStatus::LimitCheck;

                case PSParsedNumber::Kind::NotNumber:
                    break;
                }
            }

            obj.resetFromName(lex.span);
            obj.setExecutable(true);
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
