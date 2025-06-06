#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    inline bool op_and(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject b, a;
        s.pop(b); s.pop(a);

        if (a.isBool() && b.isBool()) {
            s.push(PSObject::fromBool(a.asBool() && b.asBool()));
            return true;
        }
        if (a.isInt() && b.isInt()) {
            s.push(PSObject::fromInt(a.asInt() & b.asInt()));
            return true;
        }
        return false;
    }

    inline bool op_or(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject b, a;
        s.pop(b); s.pop(a);

        if (a.isBool() && b.isBool()) {
            s.push(PSObject::fromBool(a.asBool() || b.asBool()));
            return true;
        }
        if (a.isInt() && b.isInt()) {
            s.push(PSObject::fromInt(a.asInt() | b.asInt()));
            return true;
        }
        return false;
    }

    inline bool op_xor(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject b, a;
        s.pop(b); s.pop(a);

        if (a.isBool() && b.isBool()) {
            s.push(PSObject::fromBool(a.asBool() != b.asBool()));
            return true;
        }
        if (a.isInt() && b.isInt()) {
            s.push(PSObject::fromInt(a.asInt() ^ b.asInt()));
            return true;
        }
        return false;
    }

    inline bool op_not(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject a;
        s.pop(a);

        if (a.isBool()) {
            s.push(PSObject::fromBool(!a.asBool()));
            return true;
        }
        if (a.isInt()) {
            s.push(PSObject::fromInt(~a.asInt()));
            return true;
        }
        return false;
    }

    inline const PSOperatorFuncMap& getLogicOps()
    {
        static const PSOperatorFuncMap table = {
            { "and", op_and },
            { "or",  op_or },
            { "xor", op_xor },
            { "not", op_not }
        };
        return table;
    }

} // namespace waavs
