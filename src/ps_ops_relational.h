#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    // ----- Reusable Compare Template -----

    template <typename Func>
    inline bool binaryCompareOp(PSVirtualMachine& vm, Func func) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject b, a;
        s.pop(b); s.pop(a);

        if (!a.isNumber() || !b.isNumber()) return false;

        double av = a.asReal();
        double bv = b.asReal();

        s.push(PSObject::fromBool(func(av, bv)));
        return true;
    }

    // ----- Operator Implementations -----

    inline bool op_gt(PSVirtualMachine& vm) { return binaryCompareOp(vm, [](double a, double b) { return a > b; }); }
    inline bool op_lt(PSVirtualMachine& vm) { return binaryCompareOp(vm, [](double a, double b) { return a < b; }); }
    inline bool op_ge(PSVirtualMachine& vm) { return binaryCompareOp(vm, [](double a, double b) { return a >= b; }); }
    inline bool op_le(PSVirtualMachine& vm) { return binaryCompareOp(vm, [](double a, double b) { return a <= b; }); }

    // ----- Operator Table -----

    inline const PSOperatorFuncMap& getRelationalOps() {
        static const PSOperatorFuncMap table = {
            { "gt", op_gt },
            { "lt", op_lt },
            { "ge", op_ge },
            { "le", op_le }
        };
        return table;
    }

} // namespace waavs
