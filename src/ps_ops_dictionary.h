#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {



    // --- Operator Implementations ---

    static bool op_def(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject value;
        PSObject key;

        s.pop(value);
        s.pop(key);

        if (!key.isLiteralName())
            return vm.error("op_def:typecheck: def expects a literal name");

        vm.dictionaryStack.define(key.asName(), value);
        return true;
    }

    static bool op_dict(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject sizeObj;
        s.pop(sizeObj);

        if (!sizeObj.isInt()) return false;

        auto d = PSDictionary::create(sizeObj.asInt());
        s.push(PSObject::fromDictionary(d));
        return true;
    }



    static bool op_maxlength(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject dictObj;
        s.pop(dictObj);

        if (!dictObj.isDictionary()) return false;

        // PostScript allows arbitrary max size, but we return a placeholder.
        s.push(PSObject::fromInt(999));
        return true;
    }









    static bool op_known(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject key;
        PSObject dictObj;

        s.pop(key);
        s.pop(dictObj);

        if (!key.isName() || !dictObj.isDictionary()) return false;

        auto dict = dictObj.asDictionary();
        if (!dict) return false;

        bool exists = dict->contains(key.asName());
        s.push(PSObject::fromBool(exists));
        return true;
    }



    // From '<<' to '>>', these operators are used for dictionary literals.
    static bool op_dictbegin(PSVirtualMachine& vm) {
        return vm.opStack().push(PSObject::fromMark(PSMark("dictbegin")));
    }

    static bool op_dictend(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        int count{ 0 };
        s.countToMark(count);

        if (count < 0)
            return vm.error("dictend: unmatched >> with no mark");

        if ((count % 2) != 0)
            return vm.error("dictend: odd number of items in dictionary literal");

        auto dict = PSDictionary::create();

        for (int i = 0; i < count / 2; ++i) {
            PSObject val = s.pop();
            PSObject key = s.pop();

            if (!key.isLiteralName())
                return vm.error("dictend: key must be a literal name");

            dict->put(key.asName(), val);
        }

        s.pop(); // pop the mark

        return s.push(PSObject::fromDictionary(dict));
    }

    // --- Operator Map ---

    inline const PSOperatorFuncMap& getDictionaryOps() {
        static const PSOperatorFuncMap table = {
            { "def",               op_def },
            { "dict",              op_dict },
            { "maxlength",         op_maxlength },
            { "known",             op_known },
            { "<<",                op_dictbegin },
            { ">>",                op_dictend },
        };
        return table;
    }


} // namespace waavs
