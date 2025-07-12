#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {



    // --- Operator Implementations ---

    static bool op_def(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();
        
        if (ostk.size() < 2)
            return vm.error("op_def: stackunderflow");

        PSObject value;
        PSObject keyObj;

        ostk.pop(value);
        ostk.pop(keyObj);

        if (!keyObj.isLiteralName())
        {
            writeObjectDeep(value);
            return vm.error("op_def:typecheck: def expects a literal name");
        }

        vm.dictionaryStack.define(keyObj.asName(), value);

        return true;
    }

    static bool op_dict(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        
        if (s.empty())
            return vm.error("op_dict: stackunderflow");

        int32_t sz{ 0 };
        if (!s.popInt(sz))
            return vm.error("op_dict: typecheck; expected int");

        auto d = PSDictionary::create(sz);
        s.pushDictionary(d);

        return true;
    }



    static bool op_maxlength(PSVirtualMachine& vm) {
        auto& s = vm.opStack();

        if (s.empty()) 
            return vm.error("op_maxlength: stackunderflow");

        PSDictionaryHandle dictHandle;
        if (!s.popDictionary(dictHandle)) 
            return vm.error("op_maxlength: typecheck; expected dictionary handle");

        // PostScript allows arbitrary max size, but we return a placeholder.
        s.pushInt(999);
        return true;
    }

    static bool op_known(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) 
            return vm.error("op_known: stackunderflow");

        PSObject key;
        PSObject dictObj;

        s.pop(key);
        s.pop(dictObj);

        if (!key.isName() || !dictObj.isDictionary()) 
            return vm.error("op_known: typecheck");

        auto dict = dictObj.asDictionary();
        if (!dict) 
            return vm.error("op_known: typecheck; expected dictionary");

        bool exists = dict->contains(key.asName());
        s.pushBool(exists);

        return true;
    }


    // From '<<' to '>>', these operators are used for dictionary literals.
    static bool op_dictbegin(PSVirtualMachine& vm) 
    {
        auto& s = vm.opStack();
        return s.pushMark(PSMark("dictbegin"));
    }

    static bool op_dictend(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        int count{ 0 };
        s.countToMark(count);

        if (count < 0)
            return vm.error("op_dictend: unmatched >> with no mark");

        if ((count % 2) != 0)
            return vm.error("op_dictend: odd number of items in dictionary literal");

        auto dict = PSDictionary::create();

        for (int i = 0; i < count / 2; ++i) {
            PSObject val;
            PSObject key;

            s.pop(val);
            s.pop(key);

            if (!key.isLiteralName())
                return vm.error("op_dictend: key must be a literal name");

            dict->put(key.asName(), val);
        }

        PSObject top;
        s.pop(top); // pop the mark

        return s.pushDictionary(dict);
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
