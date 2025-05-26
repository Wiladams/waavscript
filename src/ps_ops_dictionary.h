#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    static const PSOperatorFuncMap dictionaryOps = {
        { "def", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.size() < 2) return false;

            PSObject value = s.back(); s.pop_back();
            PSObject key = s.back(); s.pop_back();

            if (key.type != PSObjectType::Name) return false;

            vm.dictionaryStack.def(key.data.name, value);
            return true;
        }},

        { "dict", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.empty()) return false;

            PSObject sizeObj = s.back(); s.pop_back();
            if (sizeObj.type != PSObjectType::Int) return false;

            auto* d = new PSDictionary(); // size ignored
            s.push_back(PSObject::fromDictionary(d));
            return true;
        }},

        { "begin", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.empty()) return false;

            PSObject dictObj = s.back(); s.pop_back();
            if (dictObj.type != PSObjectType::Dictionary)
                return false;

            vm.dictionaryStack.push(std::shared_ptr<PSDictionary>(dictObj.data.dict));
            return true;
        }},

        { "end", [](PSVirtualMachine& vm) -> bool {
            vm.dictionaryStack.pop();
            return true;
        }},

        { "maxlength", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.empty()) return false;

            PSObject dictObj = s.back(); s.pop_back();
            if (dictObj.type != PSObjectType::Dictionary)
                return false;

            s.push_back(PSObject::fromInt(999)); // placeholder
            return true;
        }},

        { "load", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.empty()) return false;

            PSObject name = s.back(); s.pop_back();
            if (name.type != PSObjectType::Name) return false;

            PSObject value;
            if (!vm.dictionaryStack.load(name.data.name, value)) {
                return false; // undefined name
            }

            s.push_back(value);
            return true;
        }},

        { "where", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.empty()) return false;

            PSObject nameObj = s.back(); s.pop_back();
            if (nameObj.type != PSObjectType::Name) return false;

            const char* name = nameObj.data.name;

            for (const auto& dict : vm.dictionaryStack.stack) {
                if (dict->contains(name)) {
                    s.push_back(PSObject::fromDictionary(dict.get()));
                    s.push_back(PSObject::fromBool(true));
                    return true;
                }
            }

            s.push_back(PSObject::fromBool(false));
            return true;
        }},

        { "currentdict", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            auto top = vm.dictionaryStack.currentdict();
            if (!top) return false;
            s.push_back(PSObject::fromDictionary(top.get()));
            return true;
        }},

        { "countdictstack", [](PSVirtualMachine& vm) -> bool {
            int count = static_cast<int>(vm.dictionaryStack.stack.size());
            vm.operandStack.push_back(PSObject::fromInt(count));
            return true;
        }},

        { "known", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.size() < 2) return false;

            PSObject key = s.back(); s.pop_back();
            PSObject dictObj = s.back(); s.pop_back();

            if (key.type != PSObjectType::Name ||
                dictObj.type != PSObjectType::Dictionary)
                return false;

            PSDictionary* dict = dictObj.data.dict;
            if (!dict) return false;

            bool exists = dict->contains(key.data.name);
            s.push_back(PSObject::fromBool(exists));
            return true;
        }}
    };

} // namespace waavs
