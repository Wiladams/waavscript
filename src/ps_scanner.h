#pragma once

#include <functional>

#include "psvm.h"
#include "bspan.h"
#include "typeconv.h"

using namespace waavs;

namespace waavsps {
    // Valid characters for PostScript names (exclude delimiters and whitespace)
    static const charset nameCharSet = [] {
        charset cs;
        for (int c = 33; c <= 126; ++c) {
            switch (c) {
            case '(': case ')': case '<': case '>':
            case '[': case ']': case '{': case '}':
            case '/': case '%': case ' ':
                break;
            default:
                cs.add(static_cast<uint8_t>(c));
                break;
            }
        }
        return cs;
        }();


    enum struct PSTokenType {
        Invalid,            // Unrecognized or error
        EOI,                // End of input

        Boolean,            // true / false
        Number,             // 42, -1.5, 3.14, etc.

        LiteralName,        // /foo
        ExecutableName,     // foo

        String,             // (abc)
        HexString,          // <48656C6C6F>

        Procedure,          // { ... } block (executable array)
        LiteralArray,       // [ ... ] (if supported)

        Operator,            // Bound operator, function pointer

		Mark,               // A merker in the stack (not used yet)
		Null                // Null object (not used yet)

    };




    struct PSToken {
        PSTokenType type = PSTokenType::Invalid;
        int32_t line = -1;

        // A token's type should map to only one
        // of these values.
        bool bval = false;
        double fval = 0.0;
        ByteSpan span = {};
        void* ptr = nullptr;

        PSToken() = default;

        // Return the token as various type
        PSString* asString() const {
            return (type == PSTokenType::String || type == PSTokenType::HexString)
                ? reinterpret_cast<PSString*>(ptr)
                : nullptr;
        }


        static PSToken makeBoolean(bool b, int32_t line = -1) {
            PSToken tok;
            tok.type = PSTokenType::Boolean;
            tok.bval = b;
            tok.line = line;
            return tok;
        }

        static PSToken makeNumber(double v, int32_t line = -1) {
            PSToken tok;
            tok.type = PSTokenType::Number;
            tok.fval = v;
            tok.line = line;
            return tok;
        }

        static PSToken makeName(PSTokenType nameType, const ByteSpan& bs, int32_t line = -1) {
            PSToken tok;
            tok.type = nameType;
            tok.span = bs;
            tok.line = line;
            return tok;
        }

        static PSToken makeString(PSString* s, int32_t line = -1) {
            PSToken tok;
            tok.type = PSTokenType::String;
            tok.line = line;
            tok.ptr = s;
            return tok;
        }

        static PSToken makeHexString(PSString* s, int32_t line = -1) {
            PSToken tok;
            tok.type = PSTokenType::HexString;
            tok.ptr = s;
            tok.line = line;
            return tok;
        }

        static PSToken makeOperator(PSOperator* op, int32_t line = -1) {
            PSToken tok;
            tok.type = PSTokenType::Operator;
            tok.ptr = op;
            tok.line = line;
            return tok;
        }

        static PSToken makeProcedure(void* p, int32_t line = -1) {
            PSToken tok;
            tok.type = PSTokenType::Procedure;
            tok.ptr = p;
            tok.line = line;
            return tok;
        }

        static PSToken makeLiteralArray(void* p, int32_t line = -1) {
            PSToken tok;
            tok.type = PSTokenType::LiteralArray;
            tok.ptr = p;
            tok.line = line;
            return tok;
        }

        static PSToken makeEOI() {
            PSToken tok;
            tok.type = PSTokenType::EOI;
            return tok;
        }

        static PSToken makeInvalid() {
            return PSToken();  // default constructed
        }

        static PSToken makeMark(int32_t line = -1) {
            PSToken tok;
            tok.type = PSTokenType::Mark;
            tok.line = line;
            return tok;
        }

        static PSToken makeNull(int32_t line = -1) {
            PSToken tok;
            tok.type = PSTokenType::Null;
            tok.line = line;
            return tok;
        }


		// Turn a PSToken into a PSObject
        //
        inline PSObject toObject() const {
            using namespace waavsps;

            switch (type) {
            case PSTokenType::Boolean:
                return PSObject::fromBool(bval);
            case PSTokenType::Number:
                return PSObject::fromReal(fval);
            case PSTokenType::LiteralName:
            case PSTokenType::ExecutableName:
                return PSObject::fromName(span);
            case PSTokenType::String:
                return PSObject::fromString(static_cast<PSString*>(ptr));
            case PSTokenType::HexString:
                return PSObject::fromString(static_cast<PSString*>(ptr)); // treat as regular string
            case PSTokenType::LiteralArray:
                return PSObject::fromArray(static_cast<PSArray*>(ptr));
            case PSTokenType::Procedure:
                return PSObject::fromArray(static_cast<PSArray*>(ptr));  // with isExecutable = true
            case PSTokenType::Operator:
                return PSObject::fromOperator(static_cast<PSOperator*>(ptr));

            case PSTokenType::Mark:
                return PSObject::mark();

            case PSTokenType::Null:
                return PSObject::null();


            case PSTokenType::Invalid:
            case PSTokenType::EOI:
            default:
                return PSObject::null();
            }
        }

    };




    // Token converters
    // readName
    // Read a name toke from the byte stream.
	// Return true on success, false on failure.
    bool readName(ByteSpan& input, bool isLiteral, PSToken &tok) 
    {
        const uint8_t* start = input.begin();
        input.skipWhile(nameCharSet);
        ByteSpan nameSpan(start, input.begin());

        if (nameSpan.empty()) {
            tok = PSToken::makeInvalid();

            return false;
        }

        tok = PSToken::makeName(isLiteral ? PSTokenType::LiteralName : PSTokenType::ExecutableName,nameSpan);

        return true;
    }

    // Read a decimal number from the input
    bool readNumber(ByteSpan& input, PSToken& tok) {
        double value = 0.0;

        ByteSpan original = input;  // Preserve for rollback on failure

        if (!readDecimal(input, value)) {
            input = original;
            return false;
        }

        tok = PSToken::makeNumber(value);
        return true;
    }


    // Read a string from the input
    bool readString(ByteSpan& input, PSToken& tok) {
        if (input.empty() || *input != '(') {
            return false;
        }

        input++;  // Skip initial '('

        int parenDepth = 1;
        std::vector<uint8_t> buffer;

        while (!input.empty()) {
            uint8_t ch = *input++;

            if (ch == '\\') {
                if (input.empty()) return false;

                uint8_t next = *input++;
                switch (next) {
                case 'n':  buffer.push_back('\n'); break;
                case 'r':  buffer.push_back('\r'); break;
                case 't':  buffer.push_back('\t'); break;
                case 'b':  buffer.push_back('\b'); break;
                case 'f':  buffer.push_back('\f'); break;
                case '\\': buffer.push_back('\\'); break;
                case '(':  buffer.push_back('(');  break;
                case ')':  buffer.push_back(')');  break;
                case '\n': case '\r': break; // Continuation
                default:   buffer.push_back(next); break;
                }
                continue;
            }

            if (ch == '(') {
                ++parenDepth;
            }
            else if (ch == ')') {
                --parenDepth;
                if (parenDepth == 0) {
                    // Create PSString object
                    PSString* ps = new PSString(buffer.size());
                    for (size_t i = 0; i < buffer.size(); ++i)
                        ps->put(i, buffer[i]);

                    tok = PSToken::makeString(ps);
                    return true;
                }
            }

            buffer.push_back(ch);
        }

        return false;  // Unterminated string
    }

    bool readHexString(ByteSpan& input, PSToken& tok) {
        if (input.empty() || *input != '<') {
            return false;
        }

        input++;  // Skip '<'

        std::vector<uint8_t> buffer;
        bool highNibble = true;
        uint8_t currentByte = 0;

        while (!input.empty()) {
            uint8_t ch = *input++;

            // Skip whitespace
            if (chrWspChars(ch)) {
                continue;
            }

            // End of hex string
            if (ch == '>') {
                if (!highNibble) {
                    // Odd number of nibbles: pad last one with 0
                    buffer.push_back(currentByte << 4);
                }

                // Create PSString from result
                PSString* ps = new PSString(buffer.size());
                for (size_t i = 0; i < buffer.size(); ++i)
                    ps->put(i, buffer[i]);

                tok = PSToken::makeHexString(ps);
                return true;
            }

            // Convert hex char to nibble
            uint8_t val = 0;
            if (ch >= '0' && ch <= '9')       val = ch - '0';
            else if (ch >= 'a' && ch <= 'f')  val = ch - 'a' + 10;
            else if (ch >= 'A' && ch <= 'F')  val = ch - 'A' + 10;
            else return false;  // Invalid character

            if (highNibble) {
                currentByte = val;
                highNibble = false;
            }
            else {
                currentByte = (currentByte << 4) | val;
                buffer.push_back(currentByte);
                highNibble = true;
            }
        }

        return false;  // Unterminated hex string
    }


    bool readProcedure(ByteSpan& input, PSVirtualMachine& vm, PSToken& tok, std::function<bool(ByteSpan&, PSToken&)> nextTokenFunc) {
        if (input.empty() || *input != '{') {
            return false;
        }

        input++;  // skip opening brace
        vm.beginProc();

        int braceDepth = 1;
        PSToken innerTok;

        while (!input.empty()) {
            // Look ahead for nested braces manually
            input.skipWhile(chrWspChars);
            if (input.empty()) break;

            uint8_t c = *input;
            if (c == '{') {
                ++braceDepth;
                input++;
                vm.beginProc();
                continue;
            }
            else if (c == '}') {
                --braceDepth;
                input++;

                if (braceDepth == 0) {
                    PSArray* proc = vm.endProc();
                    if (!proc) return false;
                    tok = PSToken::makeProcedure(proc);
                    return true;
                }

                PSArray* _ = vm.endProc();  // Nested block complete
                continue;
            }

            // Otherwise, read next token and push onto operand stack
            if (!nextTokenFunc(input, innerTok)) {
                return false;
            }

            vm.push(innerTok.toObject());  // push onto operand stack
        }

        return false;  // Unterminated procedure
    }




    //================================================
    // PSScanner
    // The work horse, turning a stream of bytes into
    // an executable program.
    //================================================

    struct PSScanner {
        ByteSpan& input;
        PSVirtualMachine* vm = nullptr;

        explicit PSScanner(ByteSpan& src, PSVirtualMachine* machine = nullptr)
            : input(src), vm(machine) {
        }

        
        // Skip leading whitespace and comments
        void skipWhitespace() {
            while (!input.empty()) {
                char c = *input;
                if (c == '%') {
                    // skip comment
                    while (!input.empty() && *input != '\n') {
                        input.remove_prefix(1);
                    }
                }
                else if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f') {
                    input.remove_prefix(1);
                }
                else {
                    break;
                }
            }
        }
        

        PSToken PSScanner::nextToken() 
        {
            skipWhitespace();
            if (input.empty()) 
                return PSToken::makeEOI();

            uint8_t ch = *input;

            // 1. Literal name (starts with '/')
            if (ch == '/') {
                input++;  // Skip '/'
                PSToken tok;
                if (readName(input, true, tok)) return tok;
                return PSToken::makeInvalid();
            }

            // 2. Procedure
            if (ch == '{') {
                PSToken tok;
                if (vm && readProcedure(input, *vm, tok,
                    [&](ByteSpan& in, PSToken& t) { t = this->nextToken(); return t.type != PSTokenType::Invalid; }))
                    return tok;

                return PSToken::makeInvalid();
            }

            // 3. String literal
            if (ch == '(') {
                PSToken tok;
                if (readString(input, tok)) return tok;
                return PSToken::makeInvalid();
            }

            // 4. Hex string
            if (ch == '<') {
                PSToken tok;
                if (readHexString(input, tok)) return tok;
                return PSToken::makeInvalid();
            }

            // 5. Single-character literal arrays (optional PostScript feature)
            if (ch == '[' || ch == ']') {
                input++;
                return PSToken::makeInvalid();  // No array list parsing yet
            }

            // 6. Number
            if (is_digit(ch) || ch == '+' || ch == '-' || ch == '.') {
                PSToken tok;
                if (readNumber(input, tok)) return tok;
                // Fall through to try reading name if it failed
            }

            // 7. Executable name
            PSToken tok;
            if (readName(input, false, tok)) {
                // Check if name is "true" or "false" ? remap to Boolean
                if (tok.span == ByteSpan("true")) {
                    return PSToken::makeBoolean(true);
                }
                if (tok.span == ByteSpan("false")) {
                    return PSToken::makeBoolean(false);
                }
                return tok;
            }

            return PSToken::makeInvalid();  // Default fallback
        }


        PSToken nextToken();
    };

}