#pragma once

#include "pscore.h"

// ===========================================================
// Dictionary related operators
// ===========================================================
namespace waavs 
{
    // def: /key value -> (store in current dictionary)
    inline bool op_def(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.size() < 2) return false;

        PSObject value = s.back(); s.pop_back();
        PSObject key = s.back(); s.pop_back();

        if (key.type != PSObjectType::Name) return false;

        vm.dictionaryStack.def(key.data.name, value);
        return true;
    }

    inline bool op_dict(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.empty()) return false;

        PSObject sizeObj = s.back(); s.pop_back();
        if (sizeObj.type != PSObjectType::Int) return false;

        auto* d = new PSDictionary(); // ignore size
        s.push_back(PSObject::fromDictionary(d));
        return true;
    }

    inline bool op_begin(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.empty()) return false;

        PSObject dictObj = s.back(); s.pop_back();
        if (dictObj.type != PSObjectType::Dictionary)
            return false;

        vm.dictionaryStack.push(std::shared_ptr<PSDictionary>(dictObj.data.dict));
        return true;
    }

    inline bool op_end(PSVirtualMachine& vm) {
        vm.dictionaryStack.pop();
        return true;
    }

    inline bool op_maxlength(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.empty()) return false;

        PSObject dictObj = s.back(); s.pop_back();
        if (dictObj.type != PSObjectType::Dictionary)
            return false;

        // PostScript reports maxlength even though not enforced
        s.push_back(PSObject::fromInt(999)); // or some arbitrary value
        return true;
    }





    inline bool op_load(PSVirtualMachine& vm) {
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
    }

    inline bool op_where(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;

        if (s.empty())
            return false;

        // Get the name from the stack
        PSObject nameObj = s.back();
        s.pop_back();

        if (nameObj.type != PSObjectType::Name)
            return false;

        const ByteSpan name = nameObj.name;

        // Search dictionary stack from top to bottom
        for (const auto& dict : vm.dictionaryStack.stack) {
            if (dict->contains(name)) {
                // Push dictionary and true
                s.push_back(PSObject::fromDictionary(dict.get()));
                s.push_back(PSObject::fromBool(true));
                return true;
            }
        }

        // Not found
        s.push_back(PSObject::fromBool(false));
        return true;
    }


    // currentdict: -> dict (current dictionary)
    // Pushes the dictionary at the top of the dictionary stack onto the operand stack.
    //
    inline bool op_currentdict(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;

        auto top = vm.dictionaryStack.currentdict();
        if (!top)
            return false;

        // You'll need to define this if not already:
        // static PSObject fromDictionary(std::shared_ptr<PSDictionary> dict);

        // For now, just return true as a placeholder
        s.push_back(PSObject::fromBool(true)); // replace with dictionary object later
        return true;
    }

    inline bool op_countdictstack(PSVirtualMachine& vm) {
        int count = static_cast<int>(vm.dictionaryStack.stack.size());
        vm.operandStack.push_back(PSObject::fromInt(count));
        return true;
    }

    

    // op_known: dict key -> (true if key exists in dict)
    inline bool op_known(PSVirtualMachine& vm) {
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
    }
}