#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    // ----- Stack Operator Implementations -----

    inline bool op_dup(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject top;
        if (!s.top(top)) return false;
        s.push(top);
        return true;
    }

    inline bool op_pop(PSVirtualMachine& vm) {
        PSObject obj;
        return vm.opStack().pop(obj);
    }

    inline bool op_exch(PSVirtualMachine& vm) {
        return vm.opStack().exch();
    }

    inline bool op_index(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        PSObject countObj;

        if (!s.pop(countObj)) return false;
        if (!countObj.isInt()) return false;

        int n = countObj.asInt();
        PSObject nthObj;
        if (!s.nth(n, nthObj)) return false;

        return s.push(nthObj);
    }

    inline bool op_roll(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        PSObject jObj, nObj;

        if (!s.pop(jObj)) return false;
        if (!s.pop(nObj)) return false;
        if (!jObj.isInt() || !nObj.isInt()) return false;

        int count = nObj.asInt();
        int shift = jObj.asInt();

        return s.roll(count, shift);
    }

    inline bool op_clear(PSVirtualMachine& vm) {
        return vm.opStack().clear();
    }

    inline bool op_count(PSVirtualMachine& vm) {
        int count = static_cast<int>(vm.opStack().size());
        return vm.opStack().push(PSObject::fromInt(count));
    }

    inline bool op_mark(PSVirtualMachine& vm) {
        return vm.opStack().mark();
    }

    inline bool op_cleartomark(PSVirtualMachine& vm) {
        return vm.opStack().clearToMark();
    }

    inline bool op_counttomark(PSVirtualMachine& vm) {
        int count = 0;
        if (!vm.opStack().countToMark(count)) return false;
        return vm.opStack().push(PSObject::fromInt(count));
    }


    inline bool op_rightbracket(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        int count{ 0 };
        s.countToMark(count);
        if (count < 0)
            return vm.error("rightbracket: no matching mark");

        auto arr = PSArray::create(count);
        for (int i = count - 1; i >= 0; --i) {
            arr->put(i, s.pop());
        }

        s.pop(); // pop the mark

        return s.push(PSObject::fromArray(arr));
    }


    // ----- Operator Table -----

    inline const PSOperatorFuncMap& getStackOps() {
        static const PSOperatorFuncMap table = {
            { "dup",          op_dup },
            { "pop",          op_pop },
            { "exch",         op_exch },
            { "index",        op_index },
            { "roll",         op_roll },
            { "clear",        op_clear },
            { "count",        op_count },
            { "mark",         op_mark },
            { "[",            op_mark},         // alias for mark
            { "]",            op_rightbracket}, // alias for rightbracket
            { "cleartomark",  op_cleartomark },
            { "counttomark",  op_counttomark }
        };
        return table;
    }

} // namespace waavs
