#pragma once

#include "pscore.h"
#include "psvm.h"
#include "nametable.h"

namespace waavs {
    inline bool op_cvs(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return vm.error("stackunderflow");

        PSObject strObj;
        PSObject valObj;

        s.pop(strObj);
        s.pop(valObj);

        if (!strObj.isString()) return vm.error("typecheck");

        auto buf = strObj.asString();
        char* dst = reinterpret_cast<char*>(buf->data());
        size_t maxLen = buf->capacity();

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
            len = std::snprintf(dst, maxLen, "/%s", valObj.asName());
            break;
        default:
            len = std::snprintf(dst, maxLen, "<object>");
            break;
        }

        // Ensure length is valid and clamped to capacity
        if (len < 0) len = 0;
        if (static_cast<size_t>(len) > maxLen) len = static_cast<int>(maxLen);
        buf->setLength(len);

        return s.push(strObj);

    }

    inline bool op_cvn(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return vm.error("stackunderflow");

        PSObject strObj;
        s.pop(strObj);

        if (!strObj.isString() || !strObj.asString())
            return vm.error("typecheck");

        auto psStr = strObj.asString();
        const uint8_t* data = psStr->data();
        size_t len = psStr->length();
        OctetCursor oc(data, len);

        const char* interned = PSNameTable::INTERN(oc);
        if (!interned) return vm.error("invalidaccess");

        s.push(PSObject::fromName(interned));
        return true;
    }
    
    // string: (n -- string)
// Creates a string of length n, filled with 0 bytes.
    inline bool op_string(PSVirtualMachine& vm) {
        auto& s = vm.opStack();

        // Check that there's at least one item
        if (s.empty()) return vm.error("stackunderflow");

        PSObject lenObj;
        s.pop(lenObj);

        if (!lenObj.isInt()) return vm.error("typecheck");

        int len = lenObj.asInt();
        if (len < 0) return vm.error("rangecheck");

        // Allocate string
        auto psStr = PSString::createFromSize(static_cast<size_t>(len));


        // Push onto stack
        s.push(PSObject::fromString(psStr));
        return true;
    }


    static const PSOperatorFuncMap stringOps = {

        { "cvs", op_cvs },
        { "cvn", op_cvn},
		{ "string", op_string  }

    };
}