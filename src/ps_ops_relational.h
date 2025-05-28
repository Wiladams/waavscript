#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    template <typename Func>
    inline bool binaryCompareOp(PSVirtualMachine& vm, Func func) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject b;
        PSObject a;

        s.pop(b);
        s.pop(a);

        if (!a.isNumber() || !b.isNumber()) return false;

        double av = a.asReal();
        double bv = b.asReal();
        s.push(PSObject::fromBool(func(av, bv)));
        return true;
    }

    static const PSOperatorFuncMap relationalOps = {
        { "gt", [](PSVirtualMachine& vm) { return binaryCompareOp(vm, [](double a, double b) { return a > b; }); } },
        { "lt", [](PSVirtualMachine& vm) { return binaryCompareOp(vm, [](double a, double b) { return a < b; }); } },
        { "ge", [](PSVirtualMachine& vm) { return binaryCompareOp(vm, [](double a, double b) { return a >= b; }); } },
        { "le", [](PSVirtualMachine& vm) { return binaryCompareOp(vm, [](double a, double b) { return a <= b; }); } },
    };

} // namespace waavs
