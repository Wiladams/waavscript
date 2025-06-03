#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    static const PSOperatorFuncMap dictionaryOps = {
        { "def", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 2) return false;

            PSObject value;
            PSObject key;

            s.pop(value);
            s.pop(key);

            if (!key.isName() || key.isExecutable()) {
                return vm.error("typecheck: def expects a literal name");
            }

            vm.dictionaryStack.def(key.asName(), value);
            return true;
        }},

        { "dict", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.empty()) return false;

            PSObject sizeObj;

			s.pop(sizeObj);

            if (!sizeObj.is(PSObjectType::Int)) return false;

            auto d = PSDictionary::create(sizeObj.asInt());
            s.push(PSObject::fromDictionary(d));
            return true;
        }},

        { "begin", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.empty()) return false;

            PSObject dictObj;
            
            s.pop(dictObj);

            if (!dictObj.is(PSObjectType::Dictionary)) return vm.error("type mismatch");

            vm.dictionaryStack.push(dictObj.asDictionary());
            return true;
        }},

        { "end", [](PSVirtualMachine& vm) -> bool {
            vm.dictionaryStack.pop();
            return true;
        }},

        { "maxlength", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.empty()) return false;

            PSObject dictObj;

            s.pop(dictObj);
            
            if (dictObj.type != PSObjectType::Dictionary)
                return false;

            s.push(PSObject::fromInt(999)); // placeholder
            return true;
        }},

        { "load", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.empty()) return false;

            PSObject name;
			
            s.pop(name);

            if (name.type != PSObjectType::Name) return false;

            PSObject value;
            if (!vm.dictionaryStack.load(name.asName(), value)) {
                return false; // undefined name
            }

            s.push(value);
            return true;
        }},

        { "where", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.empty()) return false;

            PSObject nameObj;

            s.pop(nameObj);

            if (nameObj.type != PSObjectType::Name) return false;

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
        }},

        { "currentdict", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            auto top = vm.dictionaryStack.currentdict();
            if (!top) return false;
            s.push(PSObject::fromDictionary(top));
            return true;
        }},

        { "countdictstack", [](PSVirtualMachine& vm) -> bool {
            int count = static_cast<int>(vm.dictionaryStack.stack.size());
            vm.opStack().push(PSObject::fromInt(count));
            return true;
        }},

        { "known", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 2) return false;

            PSObject key;
            PSObject dictObj;

            s.pop(key);
            s.pop(dictObj);

            if (key.type != PSObjectType::Name ||
                dictObj.type != PSObjectType::Dictionary)
                return false;

            auto dict = dictObj.asDictionary();
            if (!dict) return false;

            bool exists = dict->contains(key.asName());
            s.push(PSObject::fromBool(exists));
            return true;
        }}
    };

} // namespace waavs
