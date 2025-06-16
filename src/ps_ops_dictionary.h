#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    // Dictionary access
    inline bool op_systemdict(PSVirtualMachine& vm) {
        // The system dictionary is always the first dictionary in the stack.
        auto& s = vm.opStack();
        if (vm.dictionaryStack.stack.empty()) 
            return false;
        
        // Get the systemdict explicitly from the vm
        auto systemDict = vm.getSystemDict();
        if (!systemDict) return vm.error("op_systemdict: dictionary null");

        s.push(PSObject::fromDictionary(systemDict));
        
        return true;
    }

    inline bool op_userdict(PSVirtualMachine& vm) {
        auto& s = vm.opStack();

        // Get the userdict explicitly from the vm
        auto userDict = vm.getUserDict();
        if (!userDict) return vm.error("op_userdict: dictionary null");
        s.push(PSObject::fromDictionary(userDict));
        
        return true;
    }

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

        vm.dictionaryStack.def(key.asName(), value);
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

    static bool op_begin(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject dictObj;
        s.pop(dictObj);

        if (!dictObj.isDictionary())
            return vm.error("op_begin: typecheck");

        vm.dictionaryStack.push(dictObj.asDictionary());
        return true;
    }

    static bool op_end(PSVirtualMachine& vm) {
        vm.dictionaryStack.pop();
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

    static bool op_load(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject name;
        s.pop(name);

        if (!name.isName()) return false;

        PSObject value;
        if (!vm.dictionaryStack.load(name.asName(), value))
            return false;

        s.push(value);
        return true;
    }

    static bool op_where(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject nameObj;
        s.pop(nameObj);

        if (!nameObj.isName()) return false;

        const char* name = nameObj.asName();
        for (const auto& dict : vm.dictionaryStack.stack) {
            if (dict->contains(name)) {
                s.push(PSObject::fromDictionary(dict));
                s.push(PSObject::fromBool(true));
                return true;
            }
        }

        s.push(PSObject::fromBool(false));
        return true;
    }

    static bool op_currentdict(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto top = vm.dictionaryStack.currentdict();
        if (!top) return false;

        s.push(PSObject::fromDictionary(top));
        return true;
    }

    static bool op_countdictstack(PSVirtualMachine& vm) {
        int count = static_cast<int>(vm.dictionaryStack.stack.size());
        vm.opStack().push(PSObject::fromInt(count));
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

    static bool op_store(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject value;
        PSObject key;

        s.pop(value);
        s.pop(key);

        if (!key.isName())
            return vm.error("typecheck: store expects a name key");

        if (!vm.dictionaryStack.store(key.asName(), value))
            return vm.error("store: failed to store key");

        return true;
    }

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
            { "userdict",          op_userdict },
            { "systemdict",        op_systemdict },

            { "def",               op_def },
            { "dict",              op_dict },
            { "begin",             op_begin },
            { "end",               op_end },
            { "maxlength",         op_maxlength },
            { "load",              op_load },
            { "where",             op_where },
            { "currentdict",       op_currentdict },
            { "countdictstack",    op_countdictstack },
            { "known",             op_known },
            { "store",             op_store },
            { "<<",                op_dictbegin },
            { ">>",                op_dictend },
        };
        return table;
    }


} // namespace waavs
