// ps_lex_tokenizer.h
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>
#include "pscore.h"
#include "ocspan.h"
#include "ps_type_name.h"
#include "ps_type_file.h"
#include "ps_charclass.h"
#include "ps_lexeme.h"

namespace waavs {
    static inline const uint8_t* skipWhile(OctetCursor& src, uint8_t categoryMask) noexcept {
        const uint8_t* p = src.begin();
        const uint8_t* end = src.end();
        while (p < end && PSCharClass::is(*p, categoryMask)) ++p;
        src.fStart = p;
        return p;
    }

    static inline const uint8_t* skipUntil(OctetCursor& src, uint8_t categoryMask) noexcept {
        const uint8_t* p = src.begin();
        const uint8_t* end = src.end();
        while (p < end && !PSCharClass::is(*p, categoryMask)) ++p;
        src.fStart = p;
        return p;
    }

    inline bool skipUntilKeyword(OctetCursor& src, const OctetCursor& keyword) noexcept {
        const size_t len = keyword.size();
        if (!len || src.size() < len) return false;
        const uint8_t* p = src.begin();
        const uint8_t* end = src.end();
        while (static_cast<size_t>(end - p) >= len) {
            const uint8_t* found = static_cast<const uint8_t*>(std::memchr(p, *keyword.begin(), static_cast<size_t>(end - p) - len + 1));
            if (!found) return false;
            if (std::memcmp(found, keyword.begin(), len) == 0) { src.fStart = found; return true; }
            p = found + 1;
        }
        return false;
    }

    struct PSLexGenState {
        PSFileHandle fFile;
        explicit PSLexGenState(PSFileHandle file) : fFile(std::move(file)) {}
    };

    // A CRLF pair is one PostScript newline, not two whitespace characters.
    static inline void consumePSWhitespace(OctetCursor& src) noexcept {
        if (src.empty() || !PSCharClass::isWhitespace(*src)) return;
        const uint8_t c = *src;
        ++src;
        if (c == '\r' && !src.empty() && *src == '\n') ++src;
    }

    static inline bool scanCommentLexeme(OctetCursor& src, PSLexeme& lex) noexcept {
        if (src.empty() || *src != '%') return false;
        const uint8_t* start = src.begin();
        const uint8_t* p = start + 1;
        const uint8_t* end = src.end();
        const bool dsc = p < end && *p == '%';
        while (p < end && *p != '\r' && *p != '\n' && *p != '\f') ++p;
        lex.type = dsc ? PSLexType::DSCComment : PSLexType::Comment;
        lex.span = OctetCursor(start, static_cast<size_t>(p - start));
        src.fStart = p;
        consumePSWhitespace(src);
        return true;
    }

    static inline bool scanStringLexeme(OctetCursor& src, PSLexeme& lex) noexcept {
        const uint8_t* start = src.begin();
        const uint8_t* p = start + 1;
        const uint8_t* end = src.end();
        size_t depth = 1;
        while (p < end) {
            const uint8_t c = *p++;
            if (c == '\\') {
                if (p >= end) break;
                const uint8_t escaped = *p++;
                if (escaped >= '0' && escaped <= '7') {
                    for (unsigned i = 0; i < 2 && p < end && *p >= '0' && *p <= '7'; ++i) ++p;
                }
                else if (escaped == '\r' && p < end && *p == '\n') ++p;
            }
            else if (c == '(') ++depth;
            else if (c == ')' && --depth == 0) {
                lex.type = PSLexType::String;
                lex.span = OctetCursor(start + 1, static_cast<size_t>(p - start - 2));
                src.fStart = p;
                return true;
            }
        }
        lex.type = PSLexType::UnterminatedString;
        lex.span = OctetCursor(start + 1, static_cast<size_t>(end - start - 1));
        src.fStart = end;
        return true;
    }

    static inline bool scanLiteralNameLexeme(OctetCursor& src, PSLexeme& lex) noexcept {
        ++src; // first '/'
        if (!src.empty() && *src == '/') { ++src; lex.type = PSLexType::ImmediateName; }
        else lex.type = PSLexType::LiteralName;
        const uint8_t* start = src.begin();
        const uint8_t* q = start;
        const uint8_t* end = src.end();
        while (q < end && PSCharClass::isNameChar(*q) && !(*q >= 0x80 && *q <= 0x9F)) ++q;
        src.fStart = q;
        lex.span = OctetCursor(start, static_cast<size_t>(src.begin() - start));
        consumePSWhitespace(src);
        return true;
    }

    // Always scan the entire regular-character run. Numeric classification is done by the scanner.
    static inline bool scanNameLexeme(OctetCursor& src, PSLexeme& lex) noexcept {
        const uint8_t* start = src.begin();
        const uint8_t* q = start;
        const uint8_t* end = src.end();
        while (q < end && PSCharClass::isNameChar(*q) && !(*q >= 0x80 && *q <= 0x9F)) ++q;
        src.fStart = q;
        lex.type = PSLexType::Name;
        lex.span = OctetCursor(start, static_cast<size_t>(src.begin() - start));
        consumePSWhitespace(src);
        return true;
    }

    static inline bool nextPSLexeme(PSLexGenState& state, PSLexeme& lex) noexcept {
        lex = PSLexeme{};
        if (!state.fFile || !state.fFile->hasCursor()) return false;
        OctetCursor& src = state.fFile->getCursor();
        while (!src.empty() && PSCharClass::isWhitespace(*src)) consumePSWhitespace(src);
        if (src.empty()) { lex.type = PSLexType::Eof; lex.span = src; return false; }
        const uint8_t* p = src.begin();
        const uint8_t c = *p;
        if (c == '%') return scanCommentLexeme(src, lex);
        if (c == '/') return scanLiteralNameLexeme(src, lex);
        if (c == '(') return scanStringLexeme(src, lex);
        if (c == '<' && src.size() >= 2 && src.peek(1) == '<') {
            lex.type = PSLexType::LLANGLE; lex.span = OctetCursor(p, 2); src.advance(2); return true;
        }
        if (c == '>' && src.size() >= 2 && src.peek(1) == '>') {
            lex.type = PSLexType::RRANGLE; lex.span = OctetCursor(p, 2); src.advance(2); return true;
        }
        if (c == '<') {
            const bool ascii85 = src.size() >= 2 && src.peek(1) == '~';
            src.advance(ascii85 ? 2 : 1);
            const uint8_t* start = src.begin();
            const uint8_t* q = start;
            const uint8_t* end = src.end();
            if (ascii85) {
                while (q < end && !(end - q >= 2 && q[0] == '~' && q[1] == '>')) ++q;
                lex.type = (end - q >= 2) ? PSLexType::ASCII85String : PSLexType::UnterminatedString;
                lex.span = OctetCursor(start, static_cast<size_t>(q - start));
                src.fStart = q + (end - q >= 2 ? 2 : 0);
            }
            else {
                while (q < end && *q != '>') ++q;
                lex.type = (q < end) ? PSLexType::HexString : PSLexType::UnterminatedString;
                lex.span = OctetCursor(start, static_cast<size_t>(q - start));
                src.fStart = q + (q < end ? 1 : 0);
            }
            return true;
        }
        switch (c) {
        case '{': lex.type = PSLexType::LBRACE; break;
        case '}': lex.type = PSLexType::RBRACE; break;
        case '[': lex.type = PSLexType::LBRACKET; break;
        case ']': lex.type = PSLexType::RBRACKET; break;
        case '>': case ')': lex.type = PSLexType::Delimiter; break;
        default:
            // Binary token type bytes need their own decoder (PLRM 3.14).
            // Reject rather than silently treating these as ordinary names.
            if (c >= 0x80 && c <= 0x9F) { lex.type = PSLexType::Delimiter; break; }
            return scanNameLexeme(src, lex);
        }
        lex.span = OctetCursor(p, 1);
        ++src;
        return true;
    }
}
