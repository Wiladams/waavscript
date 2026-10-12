#pragma once

#include "pscore.h"
#include "psvm.h"
#include "ps_type_file.h"

namespace waavs 
{

    inline bool op_cvs(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_cvs: stackunderflow");

        PSObject strObj;
        PSObject valueObj;

        s.pop(strObj);
        s.pop(valueObj);

        if (!strObj.isString())
            return vm.error("op_cvs: typecheck");

        PSString dest = strObj.asString();
        std::string text;

        switch (valueObj.type)
        {
        case PSObjectType::Int:
            text = std::to_string(valueObj.asInt());
            break;

        case PSObjectType::Real:
        {
            char buffer[64];
            int len = std::snprintf(buffer, sizeof(buffer), "%.6g", valueObj.asReal());

            if (len < 0)
                return vm.error("op_cvs: rangecheck");

            text.assign(buffer, static_cast<size_t>(len));
            break;
        }

        case PSObjectType::Bool:
            text = valueObj.asBool() ? "true" : "false";
            break;

        case PSObjectType::String:
        {
            const PSString& src = valueObj.asString();
            text.assign(reinterpret_cast<const char*>(src.data()), src.length());
            break;
        }

        case PSObjectType::Name:
            text = valueObj.asName().c_str();
            break;

        case PSObjectType::Operator:
            text = valueObj.asOperator().name().c_str();
            break;

        default:
            text = "--nostringval--";
            break;
        }

        if (text.size() > dest.length())
            return vm.error("op_cvs: rangecheck");

        if (!text.empty())
            std::memcpy(dest.data(), text.data(), text.size());

        return s.pushString(dest.getInterval(0, static_cast<uint32_t>(text.size())));
    }



    inline bool op_cvn(PSVirtualMachine& vm) 
    {
        auto& s = vm.opStack();
        if (s.empty()) 
            return vm.error("op_cvn: stackunderflow");

        PSObject strObj;
        s.pop(strObj);

        if (!strObj.isString())
            return vm.error("op_cvn: typecheck");

        auto psStr = strObj.asString();
        const uint8_t* data = psStr.data();
        size_t len = psStr.length();
        OctetCursor oc(data, len);

        return s.push(PSObject::fromName(oc));
    }
    
    // string: (n -- string)
    // Creates a string of length n, filled with 0 bytes.
    inline bool op_string(PSVirtualMachine& vm) {
        auto& s = vm.opStack();

        // Check that there's at least one item
        if (s.empty()) return vm.error("op_string: stackunderflow");

        PSObject lenObj;
        s.pop(lenObj);

        if (!lenObj.isInt()) 
            return vm.error("op_string: typecheck");

        int len = lenObj.asInt();
        if (len < 0) 
            return vm.error("op_string: rangecheck");

        // Allocate string
        PSString psStr(static_cast<size_t>(len));


        // Push onto stack
        s.push(PSObject::fromString(psStr));

        return true;
    }

    inline bool op_search(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("stackunderflow");

        PSObject needleObj, haystackObj;
        s.pop(needleObj);
        s.pop(haystackObj);

        if (!needleObj.isString() || !haystackObj.isString())
            return vm.error("typecheck: expected strings");

        const PSString& needle = needleObj.asString();
        const PSString& haystack = haystackObj.asString();

        PSString pre, match, post;
        if (haystack.search(needle, pre, match, post)) {
            s.push(PSObject::fromString(post));
            s.push(PSObject::fromString(match));
            s.push(PSObject::fromString(pre));
            s.push(PSObject::fromBool(true));
        }
        else {
            s.push(PSObject::fromString(haystack));
            s.push(PSObject::fromBool(false));
        }

        return true;
    }

    inline bool op_anchorsearch(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_anchorsearch: stackunderflow");

        PSObject seekObj, sourceObj;
        s.pop(seekObj);
        s.pop(sourceObj);

        if (!sourceObj.isString() || !seekObj.isString())
            return vm.error("op_anchorsearch: typecheck");

        const PSString& source = sourceObj.asString();
        const PSString& seek = seekObj.asString();

        const uint32_t sourceLen = static_cast<uint32_t>(source.length());
        const uint32_t seekLen = static_cast<uint32_t>(seek.length());

        const bool matched = seekLen <= sourceLen &&
            (seekLen == 0 || std::memcmp(source.data(), seek.data(), seekLen) == 0);

        if (!matched) {
            s.push(sourceObj);
            return s.pushBool(false);
        }

        s.pushString(source.getInterval(seekLen, sourceLen - seekLen));
        s.pushString(source.getInterval(0, seekLen));
        return s.pushBool(true);
    }

    inline bool op_cvrs(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 3)
            return vm.error("op_cvrs: stackunderflow");

        PSObject strObj;
        PSObject radixObj;
        PSObject valueObj;

        s.pop(strObj);
        s.pop(radixObj);
        s.pop(valueObj);

        if (!strObj.isString() || !radixObj.isInt() || !valueObj.isNumber())
            return vm.error("op_cvrs: typecheck");

        const int radix = radixObj.asInt();

        if (radix < 2 || radix > 36)
            return vm.error("op_cvrs: rangecheck");

        PSString dest = strObj.asString();
        std::string text;

        if (radix == 10)
        {
            if (valueObj.type == PSObjectType::Int) {
                text = std::to_string(valueObj.asInt());
            }
            else {
                char buffer[64];
                int len = std::snprintf(buffer, sizeof(buffer), "%.6g", valueObj.asReal());

                if (len < 0)
                    return vm.error("op_cvrs: rangecheck");

                text.assign(buffer, static_cast<size_t>(len));
            }
        }
        else
        {
            int32_t intValue;

            if (valueObj.type == PSObjectType::Int)
                intValue = valueObj.asInt();
            else
                intValue = static_cast<int32_t>(valueObj.asReal());

            uint32_t value = static_cast<uint32_t>(intValue);

            char buffer[32];
            char* end = buffer + sizeof(buffer);
            char* ptr = end;

            static constexpr char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

            do {
                *--ptr = digits[value % static_cast<uint32_t>(radix)];
                value /= static_cast<uint32_t>(radix);
            } while (value != 0);

            text.assign(ptr, end);
        }

        if (text.size() > dest.length())
            return vm.error("op_cvrs: rangecheck");

        if (!text.empty())
            std::memcpy(dest.data(), text.data(), text.size());

        return s.pushString(dest.getInterval(0, static_cast<uint32_t>(text.size())));
    }


    inline bool op_token(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_token: stackunderflow");

        PSObject source;
        s.pop(source);

        const bool isString = source.isString();
        const bool isFile = source.isFile();

        if (!isString && !isFile)
            return vm.error("op_token: typecheck");

        if (!source.isAccessReadable())
            return vm.error("op_token: invalidaccess");

        PSFileHandle file;

        if (isString) {
            const PSString& str = source.asString();
            file = PSMemoryFile::create(OctetCursor(str.data(), str.length()));
        }
        else {
            file = source.asFile();

            if (!file || !file->hasCursor())
                return vm.error("op_token: ioerror");
        }

        // Immediate-name resolution uses the current dictionary stack.
        auto resolver = [](void* context, const PSName& name, PSObject& out) -> bool {
            auto* machine = static_cast<PSVirtualMachine*>(context);
            return machine->dictionaryStack.load(name, out);
            };

        PSObjectGenState genState(file, &vm, resolver);

        PSObject token;
        PSScanStatus status = nextPSObjectStatus(genState, token);

        if (status == PSScanStatus::Eof) {
            return s.pushBool(false);
        }

        switch (status) {
        case PSScanStatus::Object:
            break;

        case PSScanStatus::SyntaxError:
        case PSScanStatus::ProcedureEnd:
            return vm.error("op_token: syntaxerror");

        case PSScanStatus::LimitCheck:
            return vm.error("op_token: limitcheck");

        case PSScanStatus::Undefined:
            return vm.error("op_token: undefined");

        default:
            return vm.error("op_token: syntaxerror");
        }

        if (isString) {
            // The scanner has advanced the temporary file's cursor.
            // Return a string containing all unconsumed bytes.
            const OctetCursor& remaining = file->getCursor();

            std::vector<uint8_t> bytes;
            bytes.assign(remaining.begin(), remaining.end());

            PSString rest = PSString::fromVector(bytes);

            if (!s.pushString(rest))
                return false;
        }

        if (!s.push(token))
            return false;

        return s.pushBool(true);
    }


    static const PSOperatorFuncMap& getStringOps()
    {
        static const PSOperatorFuncMap table = {
            { "cvs",            op_cvs }
            ,{ "cvn",           op_cvn }
            ,{ "cvrs",          op_cvrs }
            ,{ "string",        op_string }
            ,{ "search",        op_search }
            ,{ "anchorsearch",  op_anchorsearch }

            ,{ "token",         op_token }
        };
        return table;
    }
}