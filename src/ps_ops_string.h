#pragma once

#include "pscore.h"
#include "psvm.h"


namespace waavs {
    inline bool op_cvs(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) 
            return vm.error("stackunderflow");

        PSObject strObj;
        PSObject valObj;

        s.pop(strObj);
        s.pop(valObj);

        if (!strObj.isString()) 
            return vm.error("op_cvs: typecheck");

        auto &buf = strObj.asMutableString();
        char* dst = reinterpret_cast<char*>(buf.data());
        size_t maxLen = buf.capacity()+1;

        int len = 0;

        switch (valObj.type) {
        case PSObjectType::Int:
            len = std::snprintf(dst, maxLen, "%d", valObj.asInt());
            break;
        case PSObjectType::Real:
            len = std::snprintf(dst, maxLen, "%.6g", valObj.asReal());
            break;
        case PSObjectType::Bool:
            len = std::snprintf(dst, maxLen, "%s", valObj.asBool() ? "true" : "false");
            break;
        case PSObjectType::Name:
            len = std::snprintf(dst, maxLen, "/%s", valObj.asName().c_str());
            break;
        default:
            len = std::snprintf(dst, maxLen, "<object>");
            break;
        }

        // Ensure length is valid and clamped to capacity
        if (len < 0) len = 0;
        if (static_cast<size_t>(len) > maxLen) 
            len = static_cast<int>(maxLen);
        buf.setLength(len);

        return s.push(strObj);

    }

    inline bool op_cvn(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return vm.error("stackunderflow");

        PSObject strObj;
        s.pop(strObj);

        if (!strObj.isString())
            return vm.error("typecheck");

        auto psStr = strObj.asString();
        const uint8_t* data = psStr.data();
        size_t len = psStr.length();
        OctetCursor oc(data, len);

        s.push(PSObject::fromName(oc));
        return true;
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


    static const PSOperatorFuncMap& getStringOps()
    {
        static const PSOperatorFuncMap table = {
            { "cvs",    op_cvs }
            ,{ "cvn",    op_cvn }
            ,{ "string", op_string }
            ,{ "search", op_search }
        };
        return table;
    }
}