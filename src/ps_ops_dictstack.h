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

    static bool op_currentdict(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto top = vm.dictionaryStack.currentdict();
        if (!top) return false;

        s.push(PSObject::fromDictionary(top));
        return true;
    }

    // countdictstack
    // Count the number of dictionaries in the dictionary stack, including the userdict and systemdict.
    static bool op_countdictstack(PSVirtualMachine& vm) {
        int count = static_cast<int>(vm.dictionaryStack.stack.size());
        vm.opStack().push(PSObject::fromInt(count));
        return true;
    }

    // Create an array from all the items in the dictionary stack
    // and push that array onto the operand stack
    inline bool op_dictstack(PSVirtualMachine& vm) {
        const auto& stack = vm.dictionaryStack.stack;
        auto arr = PSArray::create();
        
        for (auto& dict : stack) {
            arr->append(PSObject::fromDictionary(dict));
        }
        
        vm.opStack().push(PSObject::fromArray(arr));

        return true;
    }

    // cleardictstack
    // Clear all non permanent dictionaries, down to the systemdict.  Retain the userdict 
    // on top of the stack, but empty it, or just create a new empty dictionary as the topmost
    // dictionary on the stack.
    inline bool op_cleardictstack(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (vm.dictionaryStack.stack.empty()) return false;

        // Clear all dictionaries except the userdict and systemdict
        while (vm.dictionaryStack.stack.size() > 1) {
            vm.dictionaryStack.pop();
        }

        // Reset the userdict to an empty dictionary
        auto userDict = PSDictionary::create();
        vm.dictionaryStack.push(userDict);
        vm.setUserDict(userDict);

        // Push an empty dictionary onto the stack
        s.push(PSObject::fromDictionary(vm.getUserDict()));
        return true;
    }


    static bool op_where(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject nameObj;
        s.pop(nameObj);

        if (!nameObj.isName()) return false;

        auto name = nameObj.asName();
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


    // --- Operator Map ---

    inline const PSOperatorFuncMap& getDictionaryStackOps() {
        static const PSOperatorFuncMap table = {
            { "userdict",           op_userdict },
            { "systemdict",         op_systemdict },
            { "countdictstack",     op_countdictstack },
            { "cleardictstack",     op_cleardictstack},
            { "dictstack",          op_dictstack},

            { "begin",              op_begin },
            { "end",                op_end },
            { "load",               op_load },
            { "where",              op_where },
            { "currentdict",        op_currentdict },
            { "store",              op_store },

        };
        return table;
    }

}
